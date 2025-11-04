#include "app.h"
#include "BinarySerializer/binarySerializer.h"
#include "testCases.h"
#include "utility/colorFormat.h"
#include "utility/timeHelper.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BS_TEST_FAILURE(__index, __time)                                       \
  fprintf(stderr, BS_RED("Test [%zu] failure, time:%0.6f ms\n"), __index,      \
          __time)

#define BS_TEST_PASSED(__index, __time)                                        \
  fprintf(stdout, BS_GREEN("Test [%zu] SUCCESS, time:%0.6f ms\n"), __index,    \
          __time)

#define BS_EPS 1E-5

static int CheckEqualData(const StatData *__restrict first,
                          const StatData *__restrict second) {
  return first->id == second->id && first->count == second->count &&
         (fabsf(first->cost - second->cost) <= BS_EPS) &&
         first->primary == second->primary && first->mode == second->mode;
}

static void GetBufferedData(char *__restrict buff, size_t buffSize,
                            const StatData *__restrict data) {
  snprintf(buff, buffSize, "{id=%ld count=%d cost=%f primary=%u mode=%u}",
           data->id, data->count, data->cost, data->primary, data->mode);
}

static int CheckResult(const StatData *data, const StatData *result,
                       size_t size, size_t resultSize) {
  if (size != resultSize) {
    fprintf(stderr, "Different size [size:%zu] [needly:%zu]\n", size,
            resultSize);
    return 0;
  }
  for (size_t i = 0; i < size; ++i) {
    if (CheckEqualData(data + i, result + i) != 1) {
      char dumpedBuff[255];
      memset(dumpedBuff, 0, sizeof(dumpedBuff));
      char originalBuff[255];
      memset(originalBuff, 0, sizeof(originalBuff));
      GetBufferedData(dumpedBuff, sizeof(dumpedBuff), data + i);
      GetBufferedData(originalBuff, sizeof(originalBuff), result + i);
      fprintf(stderr,
              BS_RED("Not equal at [index:%zu] [dumped:%s] [original:%s]\n"), i,
              dumpedBuff, originalBuff);
      return 0;
    }
  }
  return 1;
}

static Status StoreDumpHelper(const char *path, const StatData *data,
                              size_t size) {
  Status result = StoreDump(path, data, size);
  if (result != SUCCESS) {
    fprintf(stderr, BS_RED("Cannot store dump in: [path:%s][status:%d]\n"),
            path, result);
  }
  return result;
}

static void StartTest(const char *storePath1, const char *storePath2,
                      const char *resultPath, const StatData *data1,
                      const StatData *data2, const StatData *result,
                      size_t dataSize1, size_t dataSize2, size_t resultSizeBase,
                      size_t *passedTestCount, size_t *failedCount,
                      double *totalTimeMS, size_t testIndex) {
  time_handle_t begin = beginTime();

  FILE *fd1 = fopen(storePath1, "wb+");
  FILE *fd2 = fopen(storePath2, "wb+");
  FILE *fd3 = fopen(resultPath, "ab+");

  if (BINARYSERIALIZER_UNLIKELY(fd1)) {
    fclose(fd1);
  }

  if (BINARYSERIALIZER_UNLIKELY(fd2)) {
    fclose(fd2);
  }
  if (BINARYSERIALIZER_UNLIKELY(fd3)) {
    fclose(fd3);
  }
  StoreDumpHelper(storePath1, data1, dataSize1);
  StoreDumpHelper(storePath2, data2, dataSize2);

  pid_t pid = fork();
  assert(pid >= 0);
  if (pid == -1) {
    remove(storePath1);
    remove(storePath2);
    remove(resultPath);
    fprintf(stderr, BS_RED("Something went wrong, cannot fork\n"));
    double endTime = endTimeMS(begin);
    BS_TEST_FAILURE(testIndex, endTime);
    *totalTimeMS += endTime;
    *failedCount += 1;
    clearTime(begin);
    return;
  }

  if (pid == 0) {
    execlp("serializeData", "serializeData", storePath1, storePath2, resultPath,
           NULL);
    exit(-1);
  }

  int status = 0;
  waitpid(pid, &status, 0);
  if (status != 0) {
    remove(storePath1);
    remove(storePath2);
    remove(resultPath);
    fprintf(stderr, BS_RED("Something went wrong [status:%d]!\n"), status);
    double endTime = endTimeMS(begin);
    BS_TEST_FAILURE(testIndex, endTime);
    *totalTimeMS += endTime;
    *failedCount += 1;
    clearTime(begin);
    return;
  }

  StatData *resultData = NULL;
  size_t resultSize = 0;
  Status loadResult = LoadDump(resultPath, &resultData, &resultSize);
  if (loadResult != SUCCESS &&
      ((loadResult == EMPTY_FILE || loadResult == BAD_FILE) &&
       resultData != result)) {
    fprintf(stderr, BS_RED("Cannot load dump from [path:%s]\n"), resultPath);
    remove(storePath1);
    remove(storePath2);
    remove(resultPath);
    double endTime = endTimeMS(begin);
    BS_TEST_FAILURE(testIndex, endTime);
    *totalTimeMS += endTime;
    *failedCount += 1;
    clearTime(begin);
    return;
  }

  if (CheckResult(resultData, result, resultSize, resultSizeBase) == 0) {
    fprintf(stderr, "Data from Dump not equal with original data!\n");
    remove(storePath1);
    remove(storePath2);
    remove(resultPath);
    double endTime = endTimeMS(begin);
    BS_TEST_FAILURE(testIndex, endTime);
    *totalTimeMS += endTime;
    *failedCount += 1;
    clearTime(begin);
    return;
  }

  remove(storePath1);
  remove(storePath2);
  remove(resultPath);
  double endTime = endTimeMS(begin);
  BS_TEST_PASSED(testIndex, endTime);
  *totalTimeMS += endTime;
  *passedTestCount += 1;
  clearTime(begin);
}

void StartRoutine() {
  size_t passedCount = 0, failedCount = 0;
  double elapsedTime = 0.0;
  for (size_t i = 0; i < GetTestsCount(); ++i) {
    const TestCase *tc = GetTestCase(i);
    fprintf(stdout, "Test case begin [index:%zu]...\n", i);
    StartTest(tc->firstStorePath, tc->secondStorePath, tc->resultPath,
              tc->firstIn, tc->secondIn, tc->resultData, tc->firstInSize,
              tc->secondInSize, tc->resultSize, &passedCount, &failedCount,
              &elapsedTime, i);
    fprintf(stdout, "Test case end [index:%zu]...\n", i);
  }
  fprintf(
      stdout,
      BS_BLUE("Statistics: [Passed:%zu][Failed:%zu][Total time:%0.6f ms]\n"),
      passedCount, failedCount, elapsedTime);
}