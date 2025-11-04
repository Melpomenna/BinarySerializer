#include "BinarySerializer/tableView.h"

#include "BinarySerializer/config.h"
#include <stdio.h>

#if defined(BS_ENABLE_MI_MALLOC)
#include <mimalloc-override.h>
#else
#include <stdlib.h>
#endif

#include <assert.h>
#include <string.h>

static void PrintHeader(TableView *view) {
  size_t count = 3;
  LOG("Fields count:%zu\n", view->fieldsCount);
  for (size_t i = 0; i < view->fieldsCount; ++i) {
    count += view->fields[i].fieldSize;
  }
  char *buff = malloc(count * sizeof(char));
  if (BINARYSERIALIZER_UNLIKELY(!buff)) {
    return;
  }
  memset(buff, '-', count);
  buff[count - 1] = 0;
  fprintf(stdout, "%s\n", buff);
  memset(buff, ' ', count);
  for (size_t i = 0, j = 0; i < view->fieldsCount; i++) {
    buff[j] = '|';
    if (view->fields[i].header && view->fields[i].header[0] != 0) {
      memcpy(buff + j + 1, view->fields[i].header,
             view->fields[i].headerSize - 1);
    }
    j += view->fields[i].fieldSize;
  }
  buff[count - 2] = '|';
  buff[count - 1] = 0;
  fprintf(stdout, "%s\n", buff);
  memset(buff, '-', count);
  buff[count - 1] = 0;
  fprintf(stdout, "%s\n", buff);
  free(buff);
}

TablewViewStatus InitTableView(TableView *view, FormatterFunc formatter,
                               const Field *fields, size_t fieldsCount) {
  if (BINARYSERIALIZER_UNLIKELY(!view || !formatter || !fields ||
                                fieldsCount == 0)) {
    return TVS_ERROR;
  }

  view->fieldsCount = 0;
  view->formatter = NULL;
  view->data = NULL;
  view->memSize = 0;
  view->fields = malloc(sizeof(Field) * fieldsCount);
  if (view->fields) {
    view->fieldsCount = fieldsCount;
    view->formatter = formatter;
    memcpy(view->fields, fields, sizeof(Field) * fieldsCount);
    return TVS_SUCCESS;
  }

  return TVS_ERROR;
}
void ClearTableView(TableView *view) {
  if (BINARYSERIALIZER_UNLIKELY(!view)) {
    return;
  }

  view->fieldsCount = 0;
  view->data = NULL;
  view->dataSize = 0;
  free(view->fields);
  view->fields = NULL;
  view->formatter = NULL;
  view->memSize = 0;
}

TablewViewStatus PrintTable(TableView *view, size_t linesCount) {
  if (BINARYSERIALIZER_UNLIKELY(!view || linesCount == 0)) {
    return TVS_ERROR;
  }

  if (linesCount > view->dataSize) {
    linesCount = view->dataSize;
  }

  size_t count = 3;
  for (size_t i = 0; i < view->fieldsCount; ++i) {
    count += view->fields[i].fieldSize;
  }

  char *separator = malloc(count);
  if (BINARYSERIALIZER_UNLIKELY(!separator)) {
    return TVS_ERROR;
  }
  char *buffer = malloc(count);
  if (BINARYSERIALIZER_UNLIKELY(!buffer)) {
    free(separator);
    return TVS_ERROR;
  }
  memset(separator, '-', count);
  separator[count - 1] = 0;

  PrintHeader(view);
  for (size_t dataIndex = 0; dataIndex < linesCount; ++dataIndex) {
    memset(buffer, ' ', count);
    buffer[count - 1] = 0;
    for (size_t i = 0, j = 0; i < view->fieldsCount; ++i) {
      buffer[j] = '|';
      if (view->fields[i].id == -1) {
        int bytes = snprintf(buffer + j + 1, count, "%zu", dataIndex + 1);
        (buffer + j + 1)[bytes] = ' ';
      } else {
        view->formatter(view->fields[i].id,
                        (const char *)view->data + dataIndex * view->memSize,
                        buffer + j + 1, count);
      }
      j += view->fields[i].fieldSize;
    }
    buffer[count - 2] = '|';
    fprintf(stdout, "%s\n", buffer);
    if (dataIndex + 1 != linesCount) {
      fprintf(stdout, "%s\n", separator);
    }
  }

  if (linesCount < view->dataSize) {
    memset(separator, '.', count);
  }

  separator[count - 1] = 0;
  fprintf(stdout, "%s\n", separator);

  free(buffer);
  free(separator);
  return TVS_SUCCESS;
}