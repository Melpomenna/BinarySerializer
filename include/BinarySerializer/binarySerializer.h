/**
 * @file BinarySerializer.h
 * @brief Библиотека для сериализации, загрузки и обработки статистических
 * данных
 * @author Melpomenna
 * @version 1.0
 * @date 03.11.2025
 *
 * Предоставляет функции для работы с бинарными дампами структур StatData:
 * - Сохранение и загрузка из файлов
 * - Объединение дампов
 * - Сортировка данных
 * - Вывод содержимого
 */

#ifndef BINARYSERIALIZER_BINARYSERIALIZER__H
#define BINARYSERIALIZER_BINARYSERIALIZER__H

#include "BinarySerializer/config.h"
#include "BinarySerializer/statData.h"
#include "BinarySerializer/tableView.h"

#include <stdlib.h>

/**
 * @enum Status
 * @brief Коды возврата функций библиотеки
 */
typedef enum Status {
  Success, /**< Операция выполнена успешно */
  BadFile, /**< Ошибка работы с файлом (не найден, нет прав доступа) */
  EmptyFile, /**< Для LoadDump был подан пустой файл */
  InvalidPointerOrSize, /**< Невалидный указатель или размер данных */
  Error /**< Общая ошибка выполнения */
} Status;

/**
 * @typedef SortFunction
 * @brief Функция сравнения для сортировки элементов StatData
 *
 * @param lhs Указатель на левый элемент сравнения (const StatData*)
 * @param rhs Указатель на правый элемент сравнения (const StatData*)
 * @return Отрицательное значение, если lhs < rhs;
 *         Ноль, если lhs == rhs;
 *         Положительное значение, если lhs > rhs
 *
 * @note Сигнатура совместима с qsort() из stdlib.h
 *
 * @par Пример использования:
 * @code
 * int CompareByAge(const void *lhs, const void *rhs) {
 *     const StatData *a = (const StatData*)lhs;
 *     const StatData *b = (const StatData*)rhs;
 *     return a->age < b->age;
 * }
 * @endcode
 */
typedef int (*SortFunction)(const void *__restrict lhs,
                            const void *__restrict rhs);

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Сохраняет массив структур StatData в бинарный файл
 *
 * Записывает данные в файл в бинарном формате. Если файл существует,
 * он будет перезаписан. Формат файла:
 * - Заголовок (опционально, зависит от реализации)
 * - Массив структур StatData (size элементов)
 *
 * @param[in] filePath Путь к файлу для сохранения (не должен быть NULL)
 * @param[in] data Указатель на массив структур StatData (не должен быть NULL)
 * @param[in] size Количество элементов в массиве (должно быть > 0)
 *
 * @return Success при успешном сохранении
 * @return BadFile если не удалось открыть файл
 * @return InvalidPointerOrSize если data == NULL, filePath == NULL или size ==
 * 0
 * @return Error при ошибке записи данных
 *
 * @warning Функция не проверяет корректность содержимого структур StatData и не
 * создает файл
 *
 * @par Пример использования:
 * @code
 * StatData data[100];
 * // ... заполнение данных ...
 * Status result = StoreDump("output.bin", data, 100);
 * if (result != Success) {
 *     fprintf(stderr, "Ошибка сохранения: %d\n", result);
 * }
 * @endcode
 *
 * @see LoadDump
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API Status
StoreDump(const char *filePath, const StatData *data, size_t size);

/**
 * @brief Загружает массив структур StatData из бинарного файла
 *
 * Читает данные из файла и выделяет память для массива StatData.
 * Память выделяется динамически и должна быть освобождена вызывающей стороной.
 *
 * @param[in] filePath Путь к файлу для загрузки (не должен быть NULL)
 * @param[out] data Указатель на указатель, куда будет записан адрес загруженных
 * данных (не должен быть NULL; *data будет перезаписан)
 * @param[out] size Указатель, куда будет записано количество загруженных
 * элементов (не должен быть NULL)
 *
 * @return Success при успешной загрузке
 * @return BadFile если файл не найден или не может быть открыт
 * @return InvalidPointerOrSize если data == NULL, size == NULL или filePath ==
 * NULL
 * @return Error при ошибке чтения данных или выделения памяти
 *
 * @warning Вызывающая сторона ОБЯЗАНА освободить память
 * @note При ошибке *data и *size не изменяются
 *
 * @par Пример использования:
 * @code
 * StatData *data = NULL;
 * size_t size = 0;
 * Status result = LoadDump("input.bin", &data, &size);
 * if (result == Success) {
 *     // Работа с данными
 *     for (size_t i = 0; i < size; i++) {
 *         // process data[i]
 *     }
 *     free(data); // ОБЯЗАТЕЛЬНО!
 * }
 * @endcode
 *
 * @see StoreDump
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API Status
LoadDump(const char *filePath, StatData **data, size_t *size);

/**
 * @brief Объединяет два массива StatData в один
 *
 * Создает новый массив, содержащий все элементы из firstData и secondData
 * в указанном порядке. Дублирующиеся по полю id элементы соединяются по
следующему правилу:
 * При объединении записей с одинаковым id поля count и cost
складываться, поле primary будет иметь значение 0 если хотя бы в одном из
элементов оно 0. поле mode будет иметь максимальное значение из двух
представленных
 *
 * @param[in] firstData Первый массив для объединения (не должен быть NULL)
 * @param[in] firstSize Размер первого массива (должен быть > 0)
 * @param[in] secondData Второй массив для объединения (не должен быть NULL)
 * @param[in] secondSize Размер второго массива (должен быть > 0)
 * @param[out] resultData Указатель на указатель, куда будет записан результат
 *                        (не должен быть NULL)
 * @param[out] resultSize Указатель, куда будет записан размер результата
 *                        (resultSize = firstSize + secondSize)
 *
 * @return Success при успешном объединении
 * @return InvalidPointerOrSize если любой из указателей NULL или size == 0
 * @return Error при ошибке выделения памяти
 *
 * @warning Вызывающая сторона ОБЯЗАНА освободить память для результирующего
массива
 * @note Исходные массивы (firstData, secondData) не изменяются
 * @note При ошибке *resultData и *resultSize не изменяются
 *
 * @par Пример использования:
 * @code
 * StatData *first = LoadData("file1.bin");
 * StatData *second = LoadData("file2.bin");
 * StatData *merged = NULL;
 * size_t mergedSize = 0;
 *
 * Status result = JoinDump(first, 100, second, 50, &merged, &mergedSize);
 * if (result == Success) {
 *     // mergedSize == 150
 *     free(merged); // Обязательно!
 * }
 * free(first);
 * free(second);
 * @endcode
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API Status
JoinDump(const StatData *__restrict firstData, size_t firstSize,
         const StatData *__restrict secondData, size_t secondSize,
         StatData **__restrict resultData, size_t *resultSize);

/**
 * @brief Сортирует массив StatData с использованием пользовательской функции
 * сравнения
 *
 * Выполняет in-place сортировку массива. Использует алгоритм qsort() из stdlib.
 *
 * @param[in,out] data Указатель на массив для сортировки (не должен быть NULL)
 * @param[in] size Количество элементов в массиве (должно быть > 0)
 * @param[in] sortFunc Функция сравнения (не должна быть NULL)
 *
 * @return Success при успешной сортировке
 * @return InvalidPointerOrSize если data == NULL, sortFunc == NULL или size ==
 * 0
 *
 * @note Массив изменяется непосредственно (in-place сортировка)
 * @note Сложность: O(n log n) в среднем случае
 *
 * @par Пример использования:
 * @code
 * // Сортировка по возрасту (по возрастанию)
 * int CompareByAge(const void *lhs, const void *rhs) {
 *     return ((const StatData*)lhs)->age - ((const StatData*)rhs)->age;
 * }
 *
 * StatData data[100];
 * // ... заполнение данных ...
 * Status result = SortDump(data, 100, CompareByAge);
 * @endcode
 *
 * @see SortFunction
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API Status
SortDump(StatData *data, size_t size, SortFunction sortFunc);

/**
 * @brief Выводит содержимое массива StatData в стандартный вывод
 *
 * Печатает первые linesCount элементов массива в читаемом виде.
 * Если linesCount == 0 или linesCount > size ничего не делает
 *
 * @param[in] data Указатель на массив для вывода (не должен быть NULL)
 * @param[in] size Количество элементов в массиве (должно быть > 0)
 * @param[in] linesCount Максимальное количество строк для вывода
 *                       (если 0 - выводятся все элементы)
 * @param[in] tableView Указатель на структуру для вывода данных в формате
 * таблицы
 *
 * @return Success при успешном выводе
 * @return InvalidPointerOrSize если data == NULL или size == 0 или tableView ==
 * NULL
 * @return Error при ошибке вывода
 *
 * @note Вывод выполняется в stdout
 * @note Формат вывода зависит от реализации (определяется структурой StatData)
 *
 * @par Пример использования:
 * @code
 * StatData data[100];
 * // ... загрузка данных ...
 *
 * // Вывести первые 10 строк
 * PrintDump(data, 100, 10);
 *
 * // Ничего не произойдет
 * PrintDump(data, 100, 0);
 * @endcode
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API Status PrintDump(
    const StatData *data, size_t size, size_t linesCount, TableView *view);

#if defined(__cplusplus)
}
#endif

#endif // BINARYSERIALIZER_BINARYSERIALIZER__H