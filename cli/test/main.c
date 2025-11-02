#include "BinarySerializer/binarySerializer.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <sys/wait.h>
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
      fprintf(stderr, "Not equal at [index:%lu]\n", i);
      return 0;
    }
  }
  return 1;
}

int main() {
  const char firstPath[] = "case_1_in_a.dat";
  const char secondPath[] = "case_1_in_b.dat";
  const char resultPath[] = "case_1_out.dat";

  FILE *fd1 = fopen(firstPath, "wb+");
  FILE *fd2 = fopen(secondPath, "wb+");
  FILE *fd3 = fopen(resultPath, "ab+");

  fclose(fd1);
  fclose(fd2);
  fclose(fd3);

  if (StoreDump(firstPath, case_1_in_a,
                sizeof(case_1_in_a) / sizeof(StatData)) != Success) {
    remove(firstPath);
    remove(secondPath);
    remove(resultPath);
    fprintf(stderr, "Cannot store dump in: [path:%s]\n", firstPath);
    return -1;
  }

  if (StoreDump(secondPath, case_1_in_b,
                sizeof(case_1_in_b) / sizeof(StatData)) != Success) {
    remove(firstPath);
    remove(secondPath);
    remove(resultPath);
    fprintf(stderr, "Cannot store dump in: [path:%s]\n", secondPath);
    return -1;
  }

  pid_t pid = fork();

  if (pid == 0) {
    execl("serializeData", firstPath, secondPath, resultPath, NULL);
    remove(firstPath);
    remove(secondPath);
    remove(resultPath);
    fprintf(stderr, "Simething went wrong");
    exit(1);
  }

  int status = 0;
  waitpid(pid, &status, 0);

  StatData *resultData = NULL;
  size_t resultSize = 0;

  if (LoadDump(resultPath, &resultData, &resultSize) != Success) {
    fprintf(stderr, "Cannot load dump from [path:%s]\n", resultPath);
    remove(firstPath);
    remove(secondPath);
    remove(resultPath);
    return -1;
  }

  assert(sizeof(case_1_out) / sizeof(StatData) == resultSize);
  assert(resultData);

  if (checkResult(resultData, resultSize) == 0) {
    assert(0);
    remove(firstPath);
    remove(secondPath);
    remove(resultPath);
    return -1;
  }

  remove(firstPath);
  remove(secondPath);
  remove(resultPath);
  fprintf(stdout, "Success pass tests\n");

  return 0;
}