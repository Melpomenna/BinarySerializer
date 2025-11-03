#include "BinarySerializer/tableView.h"

#include "BinarySerializer/config.h"
#include <stdio.h>

TablewViewStatus initTableView(TableView *__restrict view,
                               FormatterFunc formatter,
                               const Field *__restrict fields,
                               size_t fieldsCount) {
  BINARYSERIALIZER_UNUSED(view);        // TODO
  BINARYSERIALIZER_UNUSED(formatter);   // TODO
  BINARYSERIALIZER_UNUSED(fields);      // TODO
  BINARYSERIALIZER_UNUSED(fieldsCount); // TODO
  return TVSSuccess;
}
void clearTableView(TableView *view) {
  BINARYSERIALIZER_UNUSED(view); // TODO
}

TablewViewStatus printTable(TableView *view, size_t linesCount) {
  BINARYSERIALIZER_UNUSED(view);       // TODO
  BINARYSERIALIZER_UNUSED(linesCount); // TODO
  return TVSSuccess;
}