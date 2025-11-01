#include "hashTable.h"

#include "BinarySerializer/config.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#if defined(BS_ENABLE_MI_MALLOC)
#include <mimalloc-override.h>
#endif

/*
 * @brief
 */
typedef struct Node {
  struct Bucket *bucket;
  StatData *data;
  hash_t hash;
  size_t index;
} Node;

/*
 * @brief
 */
typedef struct Bucket {
  Node *nodes;
  size_t nodesCount;
  size_t capacity;
} Bucket;

typedef struct HashNodeToArrayHelper {
  StatData *data;
  size_t shift;
} HashNodeToArrayHelper;

/*
 * @brief
 */
static void HashTableToArrayHelperFunc(StatData *__restrict data,
                                       void *__restrict args) {
  HashNodeToArrayHelper *helper = (HashNodeToArrayHelper *)args;
  memcpy(helper->data + helper->shift, data, sizeof(StatData));
  helper->shift++;
}

/*
 * @brief
 * @retval
 */
static size_t BucketIndex(const MergeHashTable *table, hash_t hash) {
  return (hash ^ (hash >> 16)) & (table->bucketsCount - 1);
}

/*
 * @brief
 * @retval hash by StatDataID
 */
// https://ru.wikipedia.org/wiki/MurmurHash2
static hash_t DefaultMurmurHash2(const StatData *stData) {
  hash_t m = 0xc6a4a7935bd1e995ULL;
  const int r = 47;
  hash_t h = 0x8445d61a4e774912ULL ^ (8 * m);

  hash_t k = stData->id;
  k *= m;
  k ^= k >> r;
  k *= m;

  h ^= k;
  h *= m;

  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return h;
}

/*
 * @brief
 */
static void DefaultMerge(StatData *__restrict first,
                         const StatData *__restrict second) {
  // total sum of count
  first->count += second->count;

  // total sum of cost
  first->cost += second->cost;

  // if any primary have zero, than primary always zero
  first->primary &= second->primary;

  // max(mode) without if, for performance branch predictor
  unsigned tmpFirstMode = first->mode;
  unsigned tmpSecondMode = second->mode;
  int ftrue = (tmpFirstMode > tmpSecondMode);
  int ffalse = ftrue ^ 1;
  first->mode = ftrue * tmpFirstMode + ffalse * tmpSecondMode;
}

/*
 * @brief
 */
static int DefaultStatDataComparator(const StatData *__restrict lhs,
                                     const StatData *__restrict rhs) {
  return lhs->id == rhs->id;
}

/*
 * @brief
 */
static void clearBucket(Bucket *bucket) {
  for (size_t i = 0; i < bucket->nodesCount; ++i) {
    free(bucket->nodes[i].data);
  }
  free(bucket->nodes);
  bucket->nodesCount = 0;
  bucket->capacity = 1;
  bucket->nodes = NULL;
}

/*
 * @brief
 */
BINARYSERIALIZER_NODISCARD static int insertIntoBucket(MergeHashTable *table,
                                                       Bucket *bucket,
                                                       const StatData *data,
                                                       hash_t hash) {
  for (size_t i = 0; i < bucket->nodesCount; ++i) {
    if (bucket->nodes[i].hash == hash &&
        table->comparator(bucket->nodes[i].data, data) == 1) {
      table->merge(bucket->nodes[i].data, data);
      return 1;
    }
  }

  Node *p = bucket->nodes;
  StatData *newData = malloc(sizeof(StatData));
  if (BINARYSERIALIZER_UNLIKELY(!newData)) {
    return 0;
  }
  if (BINARYSERIALIZER_UNLIKELY(bucket->nodesCount == bucket->capacity ||
                                !bucket->nodes)) {
    bucket->capacity *= 2;
    p = realloc(bucket->nodes, sizeof(Node) * bucket->capacity);
    if (BINARYSERIALIZER_UNLIKELY(!p)) {
      free(newData);
      return 0;
    }
  }

  int nodesCount = bucket->nodesCount;
  bucket->nodes = p;
  p[nodesCount].hash = hash;
  memcpy(newData, data, sizeof(StatData));
  p[nodesCount].data = newData;
  p[nodesCount].bucket = bucket;
  p[nodesCount].index = nodesCount;
  bucket->nodesCount++;
  return 1;
}

int initHashTable(MergeHashTable *table, HashFunction hash, MergeFunction merge,
                  StatDataCompareFunction comparator) {
  if (BINARYSERIALIZER_UNLIKELY(!table))
    return 0;
  table->hash = hash ? hash : &DefaultMurmurHash2;
  table->merge = merge ? merge : &DefaultMerge;
  table->comparator = comparator ? comparator : &DefaultStatDataComparator;
  table->buckets =
      malloc(sizeof(Bucket) * BINARYSERIALIZER_DEFUALT_BUCKETS_COUNT);
  if (BINARYSERIALIZER_LIKELY(table->buckets)) {
    table->bucketsCount = BINARYSERIALIZER_DEFUALT_BUCKETS_COUNT;
    for (size_t i = 0; i < BINARYSERIALIZER_DEFUALT_BUCKETS_COUNT; ++i) {
      table->buckets[i].nodes = NULL;
      table->buckets[i].nodesCount = 0;
      table->buckets[i].capacity = 1;
    }
  }
  return table->buckets != NULL;
}

int insertToHashTable(MergeHashTable *table, const StatData *data) {
  if (BINARYSERIALIZER_UNLIKELY(!table || !data || !table->hash ||
                                !table->merge))
    return 0;
  hash_t hash = table->hash(data);
  size_t index = BucketIndex(table, hash);
  assert(index < table->bucketsCount);
  return insertIntoBucket(table, table->buckets + index, data, hash);
}

void eraseFromHashTable(MergeHashTable *table, const StatData *data) {
  Node *node = findInHashTable(table, data);
  if (BINARYSERIALIZER_UNLIKELY(!node))
    return;

  if (node->bucket->nodesCount - 1 == 0) {
    clearBucket(node->bucket);
    return;
  }

  size_t index = node->bucket->nodesCount - 1;
  Node *lastNode = node->bucket->nodes + index;

  if (node->hash != lastNode->hash) {
    memcpy(node->data, lastNode->data, sizeof(StatData));
    free(lastNode->data);
    node->hash = lastNode->hash;
  } else {
    lastNode->index--;
  }

  node->bucket->nodesCount--;
}

Node *findInHashTable(const MergeHashTable *table, const StatData *data) {
  if (BINARYSERIALIZER_UNLIKELY(!table || !data || !table->hash))
    return NULL;

  hash_t hash = table->hash(data);
  size_t index = BucketIndex(table, hash);

  Bucket *bucket = table->buckets + index;
  for (size_t i = 0; i < bucket->nodesCount; ++i) {
    if (bucket->nodes[i].hash == hash)
      return bucket->nodes + i;
  }

  return NULL;
}

void clearHashTable(MergeHashTable *table) {
  if (BINARYSERIALIZER_UNLIKELY(!table))
    return;

  for (unsigned long i = 0; i < table->bucketsCount; ++i) {
    clearBucket(table->buckets + i);
  }

  free(table->buckets);
  table->buckets = NULL;
  table->bucketsCount = 0;
  table->hash = NULL;
  table->merge = NULL;
  table->comparator = NULL;
}

int hashTableToArray(const MergeHashTable *table, StatData **data,
                     size_t *size) {
  if (BINARYSERIALIZER_UNLIKELY(!table || !data || !size))
    return 0;

  size_t totalDataSize = 0;
  for (size_t i = 0; i < table->bucketsCount; ++i) {
    totalDataSize += table->buckets[i].nodesCount;
  }

  if (totalDataSize == 0)
    return 0;

  StatData *memBlock = malloc(sizeof(StatData) * totalDataSize);

  if (BINARYSERIALIZER_UNLIKELY(!memBlock))
    return 0;

  *size = totalDataSize;
  *data = memBlock;
  HashNodeToArrayHelper helper;
  helper.data = memBlock;
  helper.shift = 0;
  foreachElementInHashTable(table, &HashTableToArrayHelperFunc, &helper);
  return 1;
}

void foreachElementInHashTable(const MergeHashTable *table,
                               ForeachFunction action, void *args) {
  if (BINARYSERIALIZER_UNLIKELY(!table || !action))
    return;

  for (size_t i = 0; i < table->bucketsCount; ++i) {
    for (size_t j = 0; j < table->buckets[i].nodesCount; ++j) {
      action(table->buckets[i].nodes[i].data, args);
    }
  }
}