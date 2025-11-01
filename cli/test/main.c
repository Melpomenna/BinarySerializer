#include "BinarySerializer/binarySerializer.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

const StatData case_1_in_a[2] = {
    {.id = 90889, .count = 13, .cost = 3.567, .primary = 0, .mode = 3},
    {.id = 90089, .count = 1, .cost = 88.90, .primary = 1, .mode = 0}};
const StatData case_1_in_b[2] = {
    {.id = 90089, .count = 13, .cost = 0.011, .primary = 0, .mode = 2},
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode = 2}};

const StatData case_1_out[3] = {
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode = 2},
    {.id = 90889, .count = 13, .cost = 3.567, .primary = 0, .mode = 3},
    {.id = 90089, .count = 14, .cost = 88.911, .primary = 0, .mode = 2}};

static int checkEqualData(const StatData *__restrict first,
                          const StatData *__restrict second) {
  return first->id == second->id && first->count == second->count &&
         (fabs(first->cost - second->cost) <= 1E-7) &&
         first->primary == second->primary && first->mode == second->mode;
}

static int checkResult(const StatData *data, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    if (checkEqualData(data + i, &case_1_out[i]) != 1) {
      fprintf(stderr, "Not equal at [index:%lu][%s]", i, "");
      return 0;
    }
  }
  return 1;
}

int main() {
  const char firstPath[] = "case_1_in_a.dat";
  const char secondPath[] = "case_1_in_a.dat";
  const char resultPath[] = "case_1_out.dat";

  FILE *fd1 = fopen(firstPath, "wb+");
  FILE *fd2 = fopen(secondPath, "wb+");
  FILE *fd3 = fopen(resultPath, "rb+");

  fclose(fd1);
  fclose(fd2);
  fclose(fd3);

  if (StoreDump(firstPath, case_1_in_a,
                sizeof(case_1_in_a) / sizeof(StatData)) != Success) {
    fprintf(stderr, "Cannot store dump in: [path:%s]", firstPath);
    return -1;
  }

  if (StoreDump(secondPath, case_1_in_b,
                sizeof(case_1_in_b) / sizeof(StatData)) != Success) {
    fprintf(stderr, "Cannot store dump in: [path:%s]", secondPath);
    return -1;
  }

  execl("serializeData", firstPath, secondPath, resultPath, NULL);

  StatData *resultData = NULL;
  size_t resultSize = 0;

  if (LoadDump(resultPath, &resultData, &resultSize) != Success) {
    return -1;
  }

  assert(sizeof(case_1_out) / sizeof(StatData) == resultSize);
  assert(resultData);

  if (!checkResult(resultData, resultSize)) {
    return -1;
  }

  return 0;
}