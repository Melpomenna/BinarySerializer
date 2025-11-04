/**
 * @file tableView.h
 * @brief API для создания и вывода табличного представления данных
 * @author Melpomenna
 * @version 1.0
 * @date 04.11.2025
 *
 * Этот модуль предоставляет функциональность для форматирования и отображения
 * данных в виде таблицы с настраиваемыми полями и пользовательским
 * форматированием.
 */

#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include "BinarySerializer/config.h"
#include <stddef.h>

/**
 * @enum TablewViewStatus
 * @brief Статусы выполнения операций с TableView
 */
typedef enum TablewViewStatus {
  TVS_SUCCESS, ///< Операция выполнена успешно
  TVS_ERROR ///< Произошла ошибка при выполнении операции
} TablewViewStatus;

/**
 * @typedef FormatterFunc
 * @brief Функция обратного вызова для форматирования данных поля
 *
 * @param id Идентификатор поля для форматирования
 * @param data Указатель на данные для форматирования
 * @param buffer Буфер для записи отформатированной строки
 * @param bufferSize Размер буфера в байтах
 *
 * @warning функция сама должна позаботиться, чтобы после форматирования в не не
 * оказалось \0
 *
 * Функция должна записать отформатированное строковое представление
 * данных в предоставленный буфер с учетом его размера.
 */
typedef void (*FormatterFunc)(int id, const void *data, char *buffer,
                              size_t bufferSize);

/**
 * @struct Field
 * @brief Описание поля таблицы
 *
 * Структура содержит метаданные для одного столбца таблицы.
 */
typedef struct Field {
  const char *header; ///< Заголовок столбца (строка)
  size_t headerSize;  ///< Длина заголовка в символах
  int id; ///< Уникальный идентификатор поля
  int fieldSize; ///< Ширина поля в символах при отображении
} Field;

/**
 * \struct TableView
 * \brief Представление таблицы данных
 *
 * Основная структура для работы с табличным представлением.
 * Содержит конфигурацию полей, данные и функцию форматирования.
 */
typedef struct TableView {
  Field *fields; ///< Массив описаний полей таблицы
  const void *data; ///< Указатель на данные для отображения
  FormatterFunc formatter; ///< Функция форматирования данных
  size_t fieldsCount; ///< Количество полей в таблице
  size_t dataSize;    ///< Количество элементов в data
  size_t memSize; ///< Размер одного элемента данных в байтах
} TableView;

/**
 * @brief Инициализирует структуру TableView
 *
 * Создает новое табличное представление с заданными полями и функцией
 * форматирования. Выделяет необходимую память для внутренних структур.
 *
 * @param[out] view Указатель на структуру TableView для инициализации
 * @param[in] formatter Функция обратного вызова для форматирования данных
 * @param[in] fields Массив описаний полей таблицы
 * @param[in] fieldsCount Количество полей в массиве
 *
 * @return TVS_SUCCESS при успешной инициализации, TVS_ERROR при ошибке
 *
 * @note После использования необходимо вызвать ClearTableView() для
 *       освобождения ресурсов
 *
 * @warning Параметр view, formatter, fields не должны быть NULL
 *
 * @code
 * Field fields[] = {
 *   {"ID", 2, 0, 10},
 *   {"Name", 4, 1, 20}
 * };
 * TableView view;
 * if (InitTableView(&view, myFormatter, fields, 2) == TVS_SUCCESS) {
 *   // Работа с таблицей
 *   ClearTableView(&view);
 * }
 * @endcode
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API TablewViewStatus
InitTableView(TableView *view, FormatterFunc formatter, const Field *fields,
              size_t fieldsCount);

/**
 * @brief Освобождает ресурсы, связанные с TableView
 *
 * Очищает внутреннюю память, выделенную для структуры TableView.
 * После вызова структура должна быть заново инициализирована перед
 * повторным использованием.
 *
 * @param[in,out] view Указатель на структуру TableView для очистки
 *
 * @note Безопасно вызывать с неинициализированной или уже очищенной структурой
 *
 * @code
 * TableView view;
 * InitTableView(&view, formatter, fields, count);
 * // ... использование ...
 * ClearTableView(&view);
 * @endcode
 */
BINARYSERIALIZER_API void ClearTableView(TableView *view);

/**
 * @brief Выводит таблицу с данными
 *
 * Форматирует и выводит указанное количество строк данных в виде таблицы.
 * Использует функцию форматирования, заданную при инициализации, для
 * преобразования данных в текстовое представление.
 *
 * @param[in] view Указатель на инициализированную структуру TableView
 * @param[in] linesCount Количество строк данных для вывода
 *
 * @return TVS_SUCCESS при успешном выводе, TVS_ERROR при ошибке
 *
 * @warning view должен быть корректно инициализирован через InitTableView()
 * @warning view->data должен указывать на валидные данные
 *
 * @code
 * TableView view;
 * // ... инициализация ...
 * view.data = myDataArray;
 * view.dataSize = dataSize;
 * view.memSize = sizeof(MyStruct);
 *
 * if (PrintTable(&view, 10) != TVS_SUCCESS) {
 *   fprintf(stderr, "Ошибка вывода таблицы\n");
 * }
 * @endcode
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API TablewViewStatus
PrintTable(TableView *view, size_t linesCount);

#endif // TABLEVIEW_H