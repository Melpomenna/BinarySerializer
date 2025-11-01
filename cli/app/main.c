#include "BinarySerializer/binarySerializer.h"

#include <assert.h>
#include <stdio.h>

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr,
            "Program must have 3 argmunts in format: joinBs firstStoredPath "
            "secondStorePath resultPath.  All paths must be exited! [args "
            "count:%d]",
            argc);
    return -1;
  }

  StatData *first = NULL;
  StatData *second = NULL;
  size_t firstSize = 0;
  size_t secondSize = 0;

  if (LoadDump(argv[0], &first, &firstSize) != Success) {
    assert(0);
    fprintf(stderr, "Cannot load dump from: [path:%s]", argv[0]);
  }

  if (LoadDump(argv[1], &second, &secondSize) != Success) {
    assert(0);
    fprintf(stderr, "Cannot load dump from: [path:%s]", argv[1]);
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

  if (SortDump(result, resultSize, NULL) != Success) {
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

  return 0;
}