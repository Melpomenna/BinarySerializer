#include "BinarySerializer/binarySerializer.h"

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

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr,
            "Program must have 3 argmunts in format: joinBs firstStoredPath "
            "secondStorePath resultPath.  All paths must be exited! [args "
            "count:%d]\n",
            argc);
    return -1;
  }

  StatData *first = NULL;
  StatData *second = NULL;
  size_t firstSize = 0;
  size_t secondSize = 0;
  Status loadFirstSt = LoadDump(argv[0], &first, &firstSize);
  if (loadFirstSt != Success) {
    assert(loadFirstSt == Success);
    fprintf(stderr, "Cannot load dump from: [path:%s][error:%d]\n", argv[0],
            loadFirstSt);
  }

  Status loadSecondSt = LoadDump(argv[1], &second, &secondSize);
  if (loadSecondSt != Success) {
    assert(loadSecondSt == Success);
    fprintf(stderr, "Cannot load dump from: [path:%s][error:%d]\n", argv[1],
            loadSecondSt);
  }

  StatData *result = NULL;
  size_t resultSize = 0;
  if (JoinDump(first, firstSize, second, secondSize, &result, &resultSize) !=
      Success) {
    assert(0);
    free(first);
    free(second);
    return -1;
  }

  if (SortDump(result, resultSize, &SortStatDataFunc) != Success) {
    assert(0);
    return -1;
  }

  if (PrintDump(result, resultSize, 10, NULL) != Success) {
    assert(0);
    return -1;
  }

  if (StoreDump(argv[2], result, resultSize) != Success) {
    fprintf(stderr, "Cannot store dump in: [path:%s]", argv[2]);
  }

  free(result);
  free(first);
  free(second);
  fprintf(stdout, "Success serialize data\n");

  return 0;
}