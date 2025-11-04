/**
 * @file config.h
 * @brief Конфигурационные макросы и утилиты библиотеки BinarySerializer
 * @author Melpomenna
 * @version 1.0
 * @date 03.11.2025
 */

#ifndef BINARYSERIALIZER_CONFIG_H
#define BINARYSERIALIZER_CONFIG_H

/**
 * @defgroup Config Конфигурация
 * @brief Макросы для настройки видимости символов, логирования и оптимизаций
 * @{
 */

/**
 * @def BINARYSERIALIZER_UNUSED
 * @brief Макрос служит для того, что некоторое значение может не
 * использоваться, нужна как заглушка
 *
 * @code{.c}
 * void foo(int x) {
 *   BINARYSERIALIZER_UNUSED(x);
 * }
 * @endcode
 */
#define BINARYSERIALIZER_UNUSED(__x) (void)__x

/**
 * @def BINARYSERIALIZER_API
 * @brief Макрос для экспорта символов из динамической библиотеки
 *
 * Делает функцию видимой за пределами библиотеки
 * Используется для публичного API.
 *
 * @note Работает с атрибутом GCC/Clang `visibility("default")`
 *
 * @code{.c}
 * BINARYSERIALIZER_API void publicFunction(void);
 * @endcode
 */
#define BINARYSERIALIZER_API __attribute__((visibility("default")))

/**
 * @def BINARYSERIALIZER_NODISCARD
 * @brief Макрос для предупреждения о неиспользуемом возвращаемом значении
 *
 * Генерирует предупреждение компилятора, если результат функции игнорируется.
 * Используется для функций, где игнорирование результата может привести к
 * ошибкам (например, функции выделения памяти или проверки ошибок).
 *
 * @warning Игнорирование возвращаемого значения может привести к проблемам с
 * дальнейшей логикой или ресурсами
 *
 * @code{.c}
 * BINARYSERIALIZER_NODISCARD void* allocate(size_t size);
 *
 * // Компилятор выдаст предупреждение:
 * allocate(100); // Warning: ignoring return value
 * @endcode
 */
#define BINARYSERIALIZER_NODISCARD __attribute__((warn_unused_result))

/**
 * @def BINARYSERIALIZER_LIKELY
 * @brief Подсказка компилятору о наиболее вероятном значении условия
 *
 * Оптимизирует генерацию кода для случаев, когда условие почти всегда истинно.
 * Помогает процессору правильно предсказать переход и уменьшить задержки.
 *
 * @param __condition Проверяемое условие
 *
 * @note Используется для "горячих" путей выполнения (happy path)
 *
 * @code{.c}
 * if (BINARYSERIALIZER_LIKELY(ptr != NULL)) {
 *     // Обычный случай - быстрый путь
 *     return process(ptr);
 * }
 * // Редкий случай - медленный путь
 * return ERROR_NULL_PTR;
 * @endcode
 */
#define BINARYSERIALIZER_LIKELY(__condition)                                   \
  __builtin_expect(!!(__condition), 1)

/**
 * @def BINARYSERIALIZER_UNLIKELY
 * @brief Подсказка компилятору о маловероятном значении условия
 *
 * Оптимизирует генерацию кода для случаев, когда условие почти всегда ложно.
 * Помогает компилятору расположить редко выполняемый код отдельно.
 *
 * @param __condition Проверяемое условие
 *
 * @note Используется для обработки ошибок и исключительных ситуаций
 *
 * @code{.c}
 * if (BINARYSERIALIZER_UNLIKELY(file == NULL)) {
 *     // Редкий случай - ошибка открытия файла
 *     return ERROR_FILE_NOT_FOUND;
 * }
 * // Обычный путь выполнения
 * return readFile(file);
 * @endcode
 */
#define BINARYSERIALIZER_UNLIKELY(__condition)                                 \
  __builtin_expect(!!(__condition), 0)

#if defined(NDEBUG)
#define LOG_ERR(...)
#define LOG(...)
#else

#if defined(BS_ENABLE_LOG)
/**
 * @def LOG_ERR
 * @brief Макрос для вывода сообщений об ошибках в stderr
 *
 * В режиме отладки (NDEBUG не определён) выводит форматированное сообщение
 * в стандартный поток ошибок. В релизной сборке ничего не делает.
 *
 * @param ... Форматная строка и аргументы (как в printf)
 *
 * @note Автоматически отключается при компиляции с -DNDEBUG
 *
 * @code{.c}
 * if (ptr == NULL) {
 *     LOG_ERR("ERROR: Failed to allocate %zu bytes\n", size);
 * }
 * @endcode
 *
 * @see LOG
 */
#define LOG_ERR(...) fprintf(stderr, __VA_ARGS__)

/**
 * @def LOG
 * @brief Макрос для вывода отладочных сообщений в stdout
 *
 * В режиме отладки выводит форматированное сообщение в стандартный вывод.
 * В релизной сборке ничего не делает.
 *
 * @param ... Форматная строка и аргументы (как в printf)
 *
 * @note Автоматически отключается при компиляции с -DNDEBUG
 *
 * @code{.c}
 * LOG("Debug: Processing %d records\n", count);
 * @endcode
 *
 * @see LOG_ERR
 */
#define LOG(...) fprintf(stdout, __VA_ARGS__)

/** @} */ // end of Config group
#else
#define LOG_ERR(...)
#define LOG(...)
#endif
#endif

#endif // BINARYSERIALIZER_CONFIG_H