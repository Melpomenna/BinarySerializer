/**
 * @file MergeHashTable.h
 * @brief Хеш-таблица с поддержкой слияния неуникальных данных
 * @author Melpomenna
 * @version 1.0
 * @date 03.11.2025
 *
 * Этот файл предоставляет реализацию хеш-таблицы, которая автоматически
 * объединяет данные с одинаковыми ключами, используя пользовательскую
 * функцию слияния. Подходит для агрегации статистических данных.
 */

#ifndef MERGEHASHTABLE_H
#define MERGEHASHTABLE_H

/**
 * @def BINARYSERIALIZER_DEFUALT_BUCKETS_COUNT
 * @brief Размер хеш-таблицы по умолчанию при инициализации
 *
 * Определяет начальное количество бакетов в хеш-таблице.
 * Значение выбрано как компромисс между потреблением памяти
 * и производительностью для типичных сценариев использования.
 *
 * @note При большом количестве данных рекомендуется увеличить это значение
 * @see initHashTable
 */
#define BINARYSERIALIZER_DEFUALT_BUCKETS_COUNT 512

#include "BinarySerializer/config.h"
#include "BinarySerializer/statData.h"
#include <stddef.h>

/**
 * @typedef hash_t
 * @brief Тип для хранения хеш-значения
 *
 * Представляет собой 64-битное беззнаковое целое число,
 * обеспечивающее широкий диапазон хеш-значений и минимизацию коллизий.
 */
typedef unsigned long long hash_t;

/**
 * @struct Bucket
 * @brief Внутренняя структура для хранения массива элементов
 *
 * Непрозрачный тип, детали реализации скрыты в .c файле.
 * Представляет собой бакет хеш-таблицы, содержащий массив элементов из типа
 * Node
 */
struct Bucket;

/**
 * @struct Node
 * @brief Внутренняя структура для представления узла в бакете
 *
 * Непрозрачный тип для элемента связного списка внутри бакета.
 * Используется функцией findInHashTable() для возврата найденного элемента.
 */
struct Node;

/**
 * @typedef HashFunction
 * @brief Функция вычисления хеш-значения для данных
 *
 * @param data Указатель на данные для хеширования
 * @return Хеш-значение типа hash_t
 *
 * @par Требования к реализации:
 * - Функция должна быть детерминированной (одинаковые данные → одинаковый хеш)
 * - Хорошее распределение значений по диапазону hash_t
 * - Быстрое вычисление
 *
 * @code{.c}
 * hash_t myHashFunc(const StatData *data) {
 *     return (hash_t)data->id * 2654435761ULL; // Умножение на простое число
 * }
 * @endcode
 *
 * @see initHashTable
 */
typedef hash_t (*HashFunction)(const StatData *data);

/**
 * @typedef MergeFunction
 * @brief Функция слияния двух элементов с одинаковым ключом
 *
 * @param lhs Указатель на левый операнд (результат сохраняется сюда)
 * @param rhs Указатель на правый операнд (только чтение)
 *
 * @par Семантика:
 * Функция должна объединить данные из rhs в lhs. После вызова:
 * - lhs содержит объединённые данные
 * - rhs остаётся неизменным
 *
 * @warning lhs и rhs не должны пересекаться в памяти (используется __restrict)
 *
 * @code{.c}
 * void mergeSumData(StatData *lhs, const StatData *rhs) {
 *     lhs->count += rhs->count;
 *     lhs->sum += rhs->sum;
 * }
 * @endcode
 *
 * @see insertToHashTable
 */
typedef void (*MergeFunction)(StatData *__restrict lhs,
                              const StatData *__restrict rhs);

/**
 * @typedef ForeachFunction
 * @brief Функция-обработчик для итерирования по элементам таблицы
 *
 * @param data Указатель на текущий элемент данных
 * @param args Пользовательские аргументы, переданные в
 * foreachElementInHashTable()
 *
 * @par Использование:
 * Вызывается для каждого элемента таблицы при обходе.
 * Может модифицировать data, но не должна удалять элементы из таблицы.
 *
 * @code{.c}
 * void printStat(StatData *data, void *args) {
 *     printf("ID: %d, Count: %zu\n", data->id, data->count);
 * }
 *
 * foreachElementInHashTable(&table, printStat, NULL);
 * @endcode
 *
 * @see foreachElementInHashTable
 */
typedef void (*ForeachFunction)(StatData *__restrict data,
                                void *__restrict args);

/**
 * @typedef StatDataCompareFunction
 * @brief Функция сравнения двух элементов StatData на равенство
 *
 * @param lhs Указатель на левый операнд
 * @param rhs Указатель на правый операнд
 * @return 1 если элементы равны, 0 если различны
 *
 * @par Требования:
 * - Должна быть симметричной: compare(a, b) == compare(b, a)
 * - Транзитивность: если compare(a, b) и compare(b, c), то compare(a, c)
 *
 * @note Используется для определения, нужно ли объединять элементы
 *
 * @code{.c}
 * int compareById(const StatData *lhs, const StatData *rhs) {
 *     return lhs->id == rhs->id;
 * }
 * @endcode
 *
 * @see insertToHashTable, findInHashTable
 */
typedef int (*StatDataCompareFunction)(const StatData *__restrict lhs,
                                       const StatData *__restrict rhs);

/**
 * @struct MergeHashTable
 * @brief Основная структура хеш-таблицы с автоматическим слиянием
 *
 * Представляет хеш-таблицу, которая автоматически объединяет элементы
 * с на основе компаратора, используя пользовательскую функцию слияния.
 *
 * @par Пример использования:
 * @code{.c}
 * MergeHashTable table;
 * initHashTable(&table, myHash, myMerge, myCompare);
 *
 * StatData data1 = {.id = 1, .value = 100};
 * insertToHashTable(&table, &data1);
 *
 * clearHashTable(&table);
 * @endcode
 *
 * @warning Перед использованием необходимо вызвать initHashTable()
 * @warning После использования обязательно вызвать clearHashTable()
 */
typedef struct MergeHashTable {
  /**
   * @brief Массив бакетов для хранения элементов
   * @private
   */
  struct Bucket *buckets;

  /**
   * @brief Функция для вычисления хеш-значения
   * @private
   */
  HashFunction hash;

  /**
   * @brief Функция для слияния элементов с одинаковым ключом
   * @private
   */
  MergeFunction merge;

  /**
   * @brief Функция для сравнения элементов
   * @private
   */
  StatDataCompareFunction comparator;

  /**
   * @brief Количество бакетов в таблице
   * @private
   */
  size_t bucketsCount;
} MergeHashTable;

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Инициализация хеш-таблицы с заданными функциями
 *
 * Выделяет память для бакетов и настраивает хеш-таблицу для работы.
 * Количество бакетов задаётся константой
 * BINARYSERIALIZER_DEFUALT_BUCKETS_COUNT.
 *
 * @param[out] table Указатель на структуру таблицы для инициализации
 * @param[in] hash Функция вычисления хеш-значения, может быть NULL
 * @param[in] merge Функция слияния элементов, может быть NULL
 * @param[in] comparator Функция сравнения элементов, может быть NULL
 *
 * @return 1 при успешной инициализации, нулевое значение при ошибке
 *
 * @warning Перед повторной инициализацией нужно вызвать clearHashTable()
 *
 * @code{.c}
 * MergeHashTable table;
 * if (initHashTable(&table, hashFunc, mergeFunc, compareFunc) != 0) {
 *     fprintf(stderr, "Failed to initialize hash table\n");
 *     return -1;
 * }
 * @endcode
 *
 * @see clearHashTable, BINARYSERIALIZER_DEFUALT_BUCKETS_COUNT
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API int
initHashTable(MergeHashTable *table, HashFunction hash, MergeFunction merge,
              StatDataCompareFunction comparator);

/**
 * @brief Вставка элемента в хеш-таблицу с автоматическим слиянием
 *
 * Если элемент с таким же ключом уже существует (согласно comparator),
 * вызывается функция merge для объединения данных. Иначе создаётся
 * новый элемент в таблице.
 *
 * @param[in,out] table Указатель на хеш-таблицу
 * @param[in] data Указатель на данные для вставки
 *
 * @return 1 при успешной вставке/слиянии, нулевое значение при ошибке
 *
 * @note Функция создаёт копию данных, оригинал можно безопасно освободить
 * @warning table должна быть инициализирована через initHashTable()
 *
 * @par Сложность:
 * Средняя: O(1), худшая: O(n)
 *
 * @code{.c}
 * StatData data = {.id = 42, .value = 100};
 * if (insertToHashTable(&table, &data) == 0) {
 *     LOG_ERR("Insertion failed\n");
 * }
 * @endcode
 *
 * @see initHashTable, MergeFunction, StatDataCompareFunction
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API int
insertToHashTable(MergeHashTable *table, const StatData *data);

/**
 * @brief Удаление элемента из хеш-таблицы
 *
 * Находит и удаляет элемент, соответствующий заданным данным. Если элемент не
 * найден, функция ничего не делает.
 *
 * @param[in,out] table Указатель на хеш-таблицу
 * @param[in] data Указатель на данные для поиска и удаления
 *
 * @note Память удалённого элемента освобождается автоматически
 * @warning table должна быть инициализирована
 *
 * @par Сложность:
 * Средняя: O(1), худшая: O(n)
 *
 * @code{.c}
 * StatData toRemove = {.id = 42};
 * eraseFromHashTable(&table, &toRemove);
 * @endcode
 *
 * @see findInHashTable, clearHashTable
 */
BINARYSERIALIZER_API void eraseFromHashTable(MergeHashTable *table,
                                             const StatData *data);

/**
 * @brief Поиск элемента в хеш-таблице
 *
 * Выполняет поиск элемента по заданным данным, используя функции
 * hash
 *
 * @param[in] table Указатель на хеш-таблицу
 * @param[in] data Указатель на данные для поиска
 *
 * @return Указатель на узел с найденными данными или NULL, если не найдено
 *
 * @warning Возвращаемый указатель валиден до следующей модификации таблицы
 *
 * @par Сложность:
 * Средняя: O(1), худшая: O(n)
 *
 * @code{.c}
 * StatData searchKey = {.id = 42};
 * struct Node *found = findInHashTable(&table, &searchKey);
 * if (found != NULL) {
 *     // Элемент найден
 * }
 * @endcode
 *
 * @see insertToHashTable, eraseFromHashTable
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API struct Node *
findInHashTable(const MergeHashTable *table, const StatData *data);

/**
 * @brief Очистка хеш-таблицы и освобождение памяти
 *
 * Удаляет все элементы из таблицы и освобождает всю выделенную память,
 * включая массив бакетов.
 *
 * @param[in,out] table Указатель на хеш-таблицу для очистки
 *
 * @note После вызова необходима повторная инициализация через initHashTable()
 * @warning Все указатели на элементы таблицы становятся невалидными
 *
 * @code{.c}
 * clearHashTable(&table);
 * // Теперь таблицу можно инициализировать заново
 * initHashTable(&table, hashFunc, mergeFunc, compareFunc);
 * @endcode
 *
 * @see initHashTable
 */
BINARYSERIALIZER_API void clearHashTable(MergeHashTable *table);

/**
 * @brief Экспорт всех элементов таблицы в массив
 *
 * Создаёт динамический массив с копиями всех элементов хеш-таблицы.
 * Порядок элементов не гарантируется.
 *
 * @param[in] table Указатель на хеш-таблицу
 * @param[out] data Указатель на переменную для адреса массива (выделяется
 * функцией)
 * @param[out] size Указатель на переменную для количества элементов
 *
 * @return 1 при успехе, нулевое значение при ошибке выделения памяти
 *
 * @note Вызывающий код отвечает за освобождение памяти массива через free()
 * @warning При ошибке data и size не изменяются
 *
 * @par Пример:
 * @code{.c}
 * StatData *array = NULL;
 * size_t count = 0;
 *
 * if (hashTableToArray(&table, &array, &count) == 1) {
 *     for (size_t i = 0; i < count; i++) {
 *         printf("Element %zu: %d\n", i, array[i].id);
 *     }
 *     free(array);
 * }
 * @endcode
 *
 * @see foreachElementInHashTable
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API int
hashTableToArray(const MergeHashTable *table, StatData **data, size_t *size);

/**
 * @brief Применение функции ко всем элементам таблицы
 *
 * Перебирает все элементы хеш-таблицы и вызывает для каждого
 * пользовательскую функцию-обработчик. Порядок обхода не гарантируется.
 *
 * @param[in] table Указатель на хеш-таблицу
 * @param[in] action Функция-обработчик для применения к элементам
 * @param[in,out] args Пользовательские аргументы для передачи в action
 *
 * @warning action не должна удалять элементы из таблицы во время обхода
 *
 * @par Пример агрегации:
 * @code{.c}
 * void sumValues(StatData *data, void *args) {
 *     int *total = (int*)args;
 *     *total += data->value;
 * }
 *
 * int totalSum = 0;
 * foreachElementInHashTable(&table, sumValues, &totalSum);
 * printf("Total: %d\n", totalSum);
 * @endcode
 *
 * @see ForeachFunction, hashTableToArray
 */
BINARYSERIALIZER_API void
foreachElementInHashTable(const MergeHashTable *__restrict table,
                          ForeachFunction action, void *__restrict args);

#if defined(__cplusplus)
}
#endif

#endif // MERGEHASHTABLE_H