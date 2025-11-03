#include "BinarySerializer/binarySerializer.h"
#include "BinarySerializer/tableView.h"
#include "utility/colorFormat.h"
#include <assert.h>
#include <stdio.h>

#if defined(BS_ENABLE_MI_MALLOC)
#include <mimalloc-override.h>
#endif

static int SortStatDataFunc(const void *__restrict lhs,
                            const void *__restrict rhs) {
  const StatData *sdlhs = lhs;
  const StatData *sdrhs = rhs;
  return sdlhs->cost > sdrhs->cost;
}

void LoadDumpHelper(StatData **data, size_t *size, const char *path) {
  Status loadFirstSt = LoadDump(path, data, size);
  if (loadFirstSt == InvalidPointerOrSize || loadFirstSt == Error) {
    fprintf(stderr, BS_RED("Cannot load dump from: [path:%s][error:%d]\n"),
            path, loadFirstSt);
  } else if (loadFirstSt == EmptyFile) {
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
  TableView view;
  TablewViewStatus tvStatus = initTableView(&view, NULL, NULL, 0);
  if (tvStatus != TVSSuccess) {
    fprintf(stderr, BS_RED("Cannot initTableView\n"));
    return -1;
  }

  BINARYSERIALIZER_UNUSED(PrintDump(resultData, resultSize, 10, &view));
  clearTableView(&view);

  BINARYSERIALIZER_UNUSED(StoreDump(argv[3], resultData, resultSize));

  free(resultData);
  free(first);
  free(second);
  fprintf(stdout, BS_GREEN("Success serialize data\n"));

  return 0;
}