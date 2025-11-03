#include "BinarySerializer/mergeHashTable.h"

#include "BinarySerializer/config.h"

#ifndef NDEBUG
#include <stdio.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#if defined(BS_ENABLE_MI_MALLOC)
#include <mimalloc-override.h>
#endif

/**
 * @struct Node
 * @brief Узел хеш-таблицы, содержащий данные и метаинформацию
 *
 * @details
 * Каждый узел хранит:
 * - Указатель на родительский bucket
 * - Данные StatData
 * - хеш для данного элемента StatData, полученный путем вызова функции hash из
 * MegreHashTable
 * - Индекс внутри bucket (для быстрого удаления)
 *
 * @see Bucket, StatData
 */
typedef struct Node {
  struct Bucket *bucket; /**< Указатель на родительский bucket */
  StatData *data; /**< Данные узла (выделены динамически) */
  hash_t hash; /**< Предвычисленное хеш-значение для оптимизации */
  size_t index; /**< Индекс узла внутри bucket->nodes */
} Node;

/**
 * @struct Bucket
 * @brief Контейнер для узлов в хеш-таблице
 *
 * @details
 * Bucket реализует динамический массив узлов для разрешения коллизий
 * методом цепочек (separate chaining).
 *
 * @par Стратегия роста:
 * - Начальная емкость: 1 элемент
 * - Рост: удвоение при заполнении (capacity *= 2)
 * - Сжатие: не выполняется автоматически
 *
 * @par Сложность операций:
 * - Вставка (без коллизий): O(1) амортизированная
 * - Поиск: O(n), где n = nodesCount
 * - Удаление: O(n)
 *
 * @warning После clearBucket() необходимо повторно инициализировать bucket
 *
 * @see Node, insertIntoBucket(), clearBucket()
 */
typedef struct Bucket {
  Node *nodes;       /**< Динамический массив узлов */
  size_t nodesCount; /**< Количество занятых элементов */
  size_t capacity; /**< Текущая емкость массива */
} Bucket;

/**
 * @struct HashNodeToArrayHelper
 * @brief Вспомогательная структура для копирования данных из хеш-таблицы в
 * массив
 *
 * @details
 * Используется в качестве контекста при обходе хеш-таблицы.
 *
 * @see HashTableToArrayHelperFunc()
 */
typedef struct HashNodeToArrayHelper {
  StatData *data; /**< Указатель на целевой массив */
  size_t shift; /**< Текущее смещение (счетчик записанных элементов) */
} HashNodeToArrayHelper;

/**
 * @brief Callback-функция для копирования данных узла в массив
 *
 * @details
 * Копирует данные StatData в массив и увеличивает счетчик смещения.
 * Используется с функциями обхода хеш-таблицы.
 *
 * @param[in] data Указатель на данные узла (источник)
 * @param[in,out] args Указатель на HashNodeToArrayHelper (приводится к типу)
 *
 * @pre data != NULL
 * @pre args != NULL
 * @pre args указывает на корректную структуру HashNodeToArrayHelper
 * @pre helper->data имеет достаточный размер для копирования
 *
 * @see HashNodeToArrayHelper
 */
static void HashTableToArrayHelperFunc(StatData *__restrict data,
                                       void *__restrict args) {
  HashNodeToArrayHelper *helper = (HashNodeToArrayHelper *)args;
  memcpy(helper->data + helper->shift, data, sizeof(StatData));
  helper->shift++;
}

/**
 * @brief Вычисляет индекс bucket для заданного хеш-значения
 *
 * @details
 * Использует битовое смешивание (mixing) для улучшения распределения:
 * - XOR с правым сдвигом на 16 бит уменьшает зависимость от старших битов
 * - Побитовое И с маской обеспечивает результат в диапазоне [0, bucketsCount)
 *
 * @param[in] table Указатель на хеш-таблицу
 * @param[in] hash Хеш-значение ключа
 *
 * @return Индекс bucket в диапазоне [0, table->bucketsCount)
 *
 * @pre table != NULL
 * @pre table->bucketsCount является степенью двойки
 *
 * @par Алгоритм:
 * @code
 * index = (hash ^ (hash >> 16)) & (bucketsCount - 1)
 * @endcode
 *
 * @par Сложность: O(1)
 *
 * @see MergeHashTable
 */
static size_t BucketIndex(const MergeHashTable *table, hash_t hash) {
  return (hash ^ (hash >> 16)) & (table->bucketsCount - 1);
}

/**
 * @brief Вычисляет 64-битный хеш для StatData с использованием MurmurHash2
 *
 * @details
 * Реализация MurmurHash2-64A, оптимизированная для хеширования 64-битных целых.
 * Хеширует только поле id структуры StatData.
 *
 * @param[in] stData Указатель на структуру данных
 *
 * @return 64-битное хеш-значение
 *
 * @pre stData != NULL
 *
 * @par Константы:
 * - m = 0xc6a4a7935bd1e995 - множитель для перемешивания
 * - r = 47 - величина сдвига для дополнительного перемешивания
 * - seed = 0x8445d61a4e774912 - начальное значение хеша
 *
 * @warning **НЕ ЯВЛЯЕТСЯ криптографически стойкой функцией**
 *
 * @see https://ru.wikipedia.org/wiki/MurmurHash2
 * @see StatData
 *
 * @par Пример:
 * @code{.c}
 * StatData data = { .id = 12345, ... };
 * hash_t h = DefaultMurmurHash2(&data);
 * size_t bucket_idx = BucketIndex(table, h);
 * @endcode
 */
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

/**
 * @brief Объединяет данные двух узлов с одинаковым ключом
 *
 * @details
 * Реализация стратегии слияния по умолчанию:
 *
 * | Поле      | Операция                       | Описание |
 * |-----------|--------------------------------|-----------------------------------|
 * | count     | first += second                | Суммирование счетчиков | |
 * cost      | first += second                | Суммирование стоимостей | |
 * primary   | first &= second                | Логическое И (0 если хотя бы 0)
 * | | mode      | first = max(first, second)     | Максимум без ветвления |
 *
 * @param[in,out] first Целевая структура (будет изменена)
 * @param[in] second Исходная структура (только чтение)
 *
 * @pre first != NULL
 * @pre second != NULL
 * @pre first и second не должны перекрываться в памяти
 *
 * @par Оптимизация mode:
 * Использует branchless-вычисление максимума для избежания промахов branch
 * predictor:
 * @code{.c}
 * int is_first_greater = (first->mode > second->mode);  // 0 или 1
 * int is_second_greater = is_first_greater ^ 1;         // противоположное
 * mode = is_first_greater * first->mode + is_second_greater * second->mode;
 * @endcode
 *
 * @par Сложность: O(1)
 *
 * @warning При больших значениях count возможно переполнение (int)
 *
 * @see MergeHashTable::merge
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

/**
 * @brief Сравнивает два элемента StatData на равенство по ключу
 *
 * @details
 * Функция-компаратор по умолчанию, сравнивающая только поле id.
 * Используется для определения, являются ли два узла дубликатами.
 *
 * @param[in] lhs Левый операнд сравнения
 * @param[in] rhs Правый операнд сравнения
 *
 * @retval 1 Элементы равны (lhs->id == rhs->id)
 * @retval 0 Элементы не равны
 *
 * @pre lhs != NULL
 * @pre rhs != NULL
 *
 * @par Сложность: O(1)
 *
 * @note Для составных ключей требуется пользовательская функция сравнения
 *
 * @see MergeHashTable::comparator
 */
static int DefaultStatDataComparator(const StatData *__restrict lhs,
                                     const StatData *__restrict rhs) {
  return lhs->id == rhs->id;
}

/**
 * @brief Очищает bucket и освобождает все связанные ресурсы
 *
 * @details
 * Освобождает память:
 * 1. Для каждого узла: освобождает bucket->nodes[i].data
 * 2. Освобождает массив bucket->nodes
 * 3. Сбрасывает счетчики и указатели
 *
 * @param[in,out] bucket Указатель на bucket для очистки
 *
 * @pre bucket != NULL
 *
 * @post bucket->nodes == NULL
 * @post bucket->nodesCount == 0
 * @post bucket->capacity == 1
 *
 * @par Сложность: O(n), где n = bucket->nodesCount
 *
 * @warning После вызова bucket становится пустым, но остается валидным
 * @warning Все указатели на Node и StatData становятся недействительными
 *
 * @note Функция безопасна для вызова на уже очищенном bucket
 *
 * @see Bucket, Node
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

/**
 * @brief Вставляет элемент в bucket с объединением дубликатов
 *
 * @details
 * Алгоритм работы:
 * 1. **Поиск дубликата**: линейный поиск узла с тем же хешем и ключом
 * 2. **Объединение**: если найден дубликат - вызов table->merge()
 * 3. **Вставка**: если дубликат не найден:
 *    - Выделение памяти для StatData
 *    - Расширение массива nodes при необходимости (capacity *= 2)
 *    - Копирование данных и добавление нового узла
 *
 * @param[in,out] table Хеш-таблица (для доступа к merge и comparator)
 * @param[in,out] bucket Целевой bucket
 * @param[in] data Вставляемые данные
 * @param[in] hash Предвычисленное хеш-значение
 *
 * @retval 1 Успешная вставка или объединение
 * @retval 0 Ошибка выделения памяти
 *
 * @pre table != NULL
 * @pre bucket != NULL
 * @pre data != NULL
 * @pre table->merge != NULL
 * @pre table->comparator != NULL
 *
 * @par Сложность:
 * - Лучший случай (дубликат в начале): O(1)
 * - Худший случай (нет дубликата): O(n + realloc)
 * - Амортизированная: O(1) для вставки без коллизий
 *
 * @par Гарантии безопасности:
 * - При ошибке malloc/realloc состояние bucket не изменяется
 * - Отсутствие утечек памяти при любом результате
 *
 * @par Оптимизации:
 * - Использование BINARYSERIALIZER_UNLIKELY для редких случаев
 * - Предвычисление хеша для ускорения сравнения
 *
 * @warning Функция НЕ проверяет корректность индекса bucket в таблице
 *
 * @see Bucket, Node, MergeHashTable
 *
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
  LOG("[hashTableToArray begin]_____________________\n");
  if (BINARYSERIALIZER_UNLIKELY(!table || !data || !size)) {
    LOG_ERR("Null data\n");
    LOG("[hashTableToArray end]_____________________\n");
    return 0;
  }
  size_t totalDataSize = 0;
  for (size_t i = 0; i < table->bucketsCount; ++i) {
    totalDataSize += table->buckets[i].nodesCount;
  }

  if (totalDataSize == 0) {
    LOG_ERR("Empty totalSize for hashTable\n");
    LOG("[hashTableToArray end]_____________________\n");
    return 0;
  }
  StatData *memBlock = malloc(sizeof(StatData) * totalDataSize);

  if (BINARYSERIALIZER_UNLIKELY(!memBlock)) {
    LOG_ERR("Cannot allocate [bytes:%zu]\n", sizeof(StatData) * totalDataSize);
    LOG("[hashTableToArray end]_____________________\n");
    return 0;
  }
  *size = totalDataSize;
  *data = memBlock;
  HashNodeToArrayHelper helper;
  helper.data = memBlock;
  helper.shift = 0;
  foreachElementInHashTable(table, &HashTableToArrayHelperFunc, &helper);
  LOG("[hashTableToArray end]_____________________\n");
  return 1;
}

void foreachElementInHashTable(const MergeHashTable *table,
                               ForeachFunction action, void *args) {
  if (BINARYSERIALIZER_UNLIKELY(!table || !action))
    return;

  for (size_t i = 0; i < table->bucketsCount; ++i) {
    for (size_t j = 0; j < table->buckets[i].nodesCount; ++j) {
      action(table->buckets[i].nodes[j].data, args);
    }
  }
}