#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include "BinarySerializer/config.h"
#include <stddef.h>

typedef enum TablewViewStatus { TVSSuccess, TVSError } TablewViewStatus;

typedef void (*FormatterFunc)(int id, const void *data, char *buffer,
                              size_t bufferSize);

typedef struct Field {
  const char *header;
  size_t headerSize;
  int id;
  int fieldSize;
} Field;

typedef struct TableView {
  const Field *fields;
  char *buffer;
  const void *data;
  FormatterFunc formatter;
  size_t columns;
  size_t dataSize;
  size_t memSize;
  size_t bufferSize;
} TableView;

BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API TablewViewStatus
initTableView(TableView *__restrict view, FormatterFunc formatter,
              const Field *__restrict fields, size_t fieldsCount);
BINARYSERIALIZER_API void clearTableView(TableView *view);

#endif // TABLEVIEW_H