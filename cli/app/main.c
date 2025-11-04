#include "BinarySerializer/binarySerializer.h"
#include "BinarySerializer/tableView.h"
#include "utility/colorFormat.h"

#if defined(BS_ENABLE_MI_MALLOC)
#include <mimalloc-override.h>
#else
#include <stdlib.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef enum Columns {
  NUMBER = -1,
  ID = 0,
  COUNT,
  COST,
  PRIMARY,
  MODE
} Columns;

static void PrintStatDataFormatter(int id, const void *__restrict data,
                                   char *__restrict buffer, size_t bufferSize) {
  const StatData *p = data;
  size_t count = 0;
  switch (id) {
  case ID: {
    count = snprintf(buffer, bufferSize, "%lx", p->id);
    break;
  }
  case COUNT: {
    count = snprintf(buffer, bufferSize, "%d", p->count);
    break;
  }
  case COST: {
    count = snprintf(buffer, bufferSize, "%.3e", p->cost);
    break;
  }
  case PRIMARY: {
    count = snprintf(buffer, bufferSize, "%c", p->primary == 1 ? 'y' : 'n');
    break;
  }
  case MODE: {
    unsigned int mode = p->mode;
    char tmp[sizeof(mode)];
    memset(tmp, 0, sizeof(tmp));
    for (int i = 2; i >= 0; i--) {
      tmp[2 - i] = (mode & (1 << i)) ? '1' : '0';
    }
    count = snprintf(buffer, bufferSize, "%s", tmp);
    break;
  }
  default: {
    LOG_ERR("Unknow id for formatter:%d\n", id);
    assert(0);
  }
  }
  buffer[count] = ' ';
}

static int SortStatDataFunc(const void *__restrict lhs,
                            const void *__restrict rhs) {
  const StatData *sdlhs = lhs;
  const StatData *sdrhs = rhs;
  return sdlhs->cost > sdrhs->cost;
}

static void LoadDumpHelper(StatData **data, size_t *size, const char *path) {
  Status loadFirstSt = LoadDump(path, data, size);
  if (loadFirstSt == INVALID_POINTER_OR_SIZE || loadFirstSt == ERROR) {
    fprintf(stderr, BS_RED("Cannot load dump from: [path:%s][ERROR:%d]\n"),
            path, loadFirstSt);
  } else if (loadFirstSt == EMPTY_FILE) {
    fprintf(stdout, "Empty file [path:%s]\n", path);
  }
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(
        stderr,
        BS_RED("Program must have 3 argmunts with application (total 4) in "
               "format: joinBs firstStoredPath "
               "secondStorePath resultPath.  All paths must be exited! [args "
               "count:%d]\n"),
        argc);
    return -1;
  }

  StatData *first = NULL;
  StatData *second = NULL;
  size_t firstSize = 0;
  size_t secondSize = 0;

  LoadDumpHelper(&first, &firstSize, argv[1]);
  LoadDumpHelper(&second, &secondSize, argv[2]);
  StatData *resultData = NULL;
  size_t resultSize = 0;
  BINARYSERIALIZER_UNUSED(
      JoinDump(first, firstSize, second, secondSize, &resultData, &resultSize));

  BINARYSERIALIZER_UNUSED(SortDump(resultData, resultSize, &SortStatDataFunc));

  LOG("Result data size: [size:%zu]\n", resultSize);

  const char idField[] = "id";
  const char countField[] = "count";
  const char costField[] = "cost";
  const char primaryField[] = "primary";
  const char modeField[] = "mode";

  const Field fields[] = {
      {NULL, 0, NUMBER, 15},
      {idField, sizeof(idField), ID, 15},
      {countField, sizeof(countField), COUNT, 15},
      {costField, sizeof(costField), COST, 15},
      {primaryField, sizeof(primaryField), PRIMARY, 9},
      {modeField, sizeof(modeField), MODE, 6},
  };

  TableView view;
  TablewViewStatus tvStatus = InitTableView(
      &view, &PrintStatDataFormatter, fields, sizeof(fields) / sizeof(Field));
  if (tvStatus != TVS_SUCCESS) {
    fprintf(stderr, BS_RED("Cannot initTableView\n"));
    return -1;
  }

  BINARYSERIALIZER_UNUSED(PrintDump(resultData, resultSize, 10, &view));
  ClearTableView(&view);

  BINARYSERIALIZER_UNUSED(StoreDump(argv[3], resultData, resultSize));

  free(resultData);
  free(first);
  free(second);

  return 0;
}