/*
 * @file MergeHashTable.h - file provides functions and struct to use hash table
 * with merge non unique data
 * @author Melpomenna
 * @date 30.10.2025
 */

#ifndef MERGEHASHTABLE_H
#define MERGEHASHTABLE_H

#define BINARYSERIALIZER_DEFUALT_BUCKETS_COUNT 8

#include "BinarySerializer/config.h"
#include "BinarySerializer/statData.h"
#include <stddef.h>

typedef unsigned long long hash_t;

struct Bucket;
struct Node;

typedef hash_t (*HashFunction)(const StatData *data);
typedef void (*MergeFunction)(StatData *__restrict lhs,
                              const StatData *__restrict rhs);
typedef void (*ForeachFunction)(StatData *__restrict data,
                                void *__restrict args);

/*
 * @brief Function check is equal StatData
 * @retval 1 - if equal
 * @retval 0 - if not equal
 */
typedef int (*StatDataCompareFunction)(const StatData *__restrict lhs,
                                       const StatData *__restrict rhs);

/*
 * @brief Struct
 */
typedef struct MergeHashTable {
  struct Bucket *buckets;
  HashFunction hash;
  MergeFunction merge;
  StatDataCompareFunction comparator;
  size_t bucketsCount;
  int isInited;
} MergeHashTable;

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * @brief
 * @retval
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API int
initHashTable(MergeHashTable *table, HashFunction hash, MergeFunction merge,
              StatDataCompareFunction comparator);

/*
 * @brief
 */
BINARYSERIALIZER_API void insertToHashTable(MergeHashTable *table,
                                            const StatData *data);

/*
 * @brief
 */
BINARYSERIALIZER_API void eraseFromHashTable(MergeHashTable *table,
                                             const StatData *data);

/*
 * @brief
 * @retval
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API struct Node *
findInHashTable(const MergeHashTable *table, const StatData *data);

/*
 * @brief
 */
BINARYSERIALIZER_API void clearHashTable(MergeHashTable *table);

/*
 * @brief
 * @retval
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API int
hashTableToArray(const MergeHashTable *table, StatData **data, size_t *size);

/*
 * @brief
 */
BINARYSERIALIZER_API void
foreachElementInHashTable(const MergeHashTable *__restrict table,
                          ForeachFunction action, void *__restrict args);

#if defined(__cplusplus)
}
#endif

#endif // MERGEHASHTABLE_H