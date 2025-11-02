#include "BinarySerializer/binarySerializer.h"
#include "hashTable.h"

#if defined(BS_ENABLE_MI_MALLOC)
#include <mimalloc-override.h>
#endif

#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static const size_t butchSize = BINARYSERIALIZER_BUTCHE_SIZE;

Status StoreDump(const char *filePath, const StatData *data, size_t size) {
  if (BINARYSERIALIZER_UNLIKELY(!filePath || !data || size == 0))
    return InvalidPointerOrSize;

  int fd = open(filePath, O_RDWR);
  if (BINARYSERIALIZER_UNLIKELY(fd < 0))
    return BadFile;

  size_t fileSize = sizeof(StatData) * size;
  if (BINARYSERIALIZER_UNLIKELY(ftruncate(fd, fileSize) == -1)) {
    close(fd);
    return BadFile;
  }

  void *addr =
      mmap(NULL, sizeof(StatData) * size, PROT_WRITE, MAP_SHARED, fd, 0);
  if (BINARYSERIALIZER_UNLIKELY(addr == MAP_FAILED))
    return InvalidPointerOrSize;
  memcpy(addr, data, sizeof(StatData) * size);
  msync(addr, sizeof(StatData) * size, MS_ASYNC);
  munmap(addr, sizeof(StatData) * size);
  close(fd);

  return Success;
}

Status LoadDump(const char *filePath, StatData **data, size_t *size) {
  if (BINARYSERIALIZER_UNLIKELY(!filePath || !data || !size))
    return InvalidPointerOrSize;
  LOG("LoadDump: [path:%s]\n", filePath);
  int fd = open(filePath, O_RDWR);
  if (BINARYSERIALIZER_UNLIKELY(fd < 0)) {
    LOG_ERR("LoadDump: cannot open file [path:%s]", filePath);
    return BadFile;
  }

  size_t fileSize = 0;
  struct stat statBuf;
  int result = fstat(fd, &statBuf);
  if (BINARYSERIALIZER_UNLIKELY(result < 0)) {
    LOG_ERR("LoadDump: bad result on fstat: [result:%d]\n", result);
    assert(result > 0);
    close(fd);
    return Error;
  }
  fileSize = statBuf.st_size;

  if (fileSize == 0) {
    LOG_ERR("LoadDump: cannot load dump from empty file [paths:%s]\n",
            filePath);
    close(fd);
    return BadFile;
  }
  LOG("LoadDump: File opened with [size:%zu][StatData size:%zu]\n", fileSize,
      sizeof(StatData));
  while (fileSize % sizeof(StatData) != 0)
    fileSize--;
  LOG("LoadDump: reduced size [size:%zu]\n", fileSize);
  size_t butchesCount = (fileSize / (sizeof(StatData) * butchSize));
  LOG("LoadDump: butches count [count:%zu]\n", butchesCount);
  size_t i = 0;
  size_t totalSize = fileSize / sizeof(StatData);
  LOG("LoadDump: [totalSize:%zu]\n", totalSize);
  StatData *resultData = NULL;
  for (; i < butchesCount && totalSize >= butchSize;
       ++i, totalSize -= butchSize, resultData += butchSize) {
    StatData *rdata =
        realloc(resultData, sizeof(StatData) * butchSize * (i + 1));
    if (BINARYSERIALIZER_UNLIKELY(!rdata)) {
      free(resultData);
      close(fd);
      return Error;
    }
    resultData = rdata;
    void *addr = mmap(NULL, sizeof(StatData) * butchSize, PROT_READ,
                      MAP_PRIVATE, fd, sizeof(StatData) * i);
    if (BINARYSERIALIZER_UNLIKELY(!addr)) {
      free(resultData);
      close(fd);
      return Error;
    }
    memcpy(resultData, addr, sizeof(StatData) * butchSize);
    munmap(addr, sizeof(StatData) * i);
  }

  if (totalSize != 0) {
    StatData *rdata = realloc(resultData, fileSize);
    if (BINARYSERIALIZER_UNLIKELY(!rdata)) {
      close(fd);
      free(resultData);
      return Error;
    }
    resultData = rdata;
    void *addr = mmap(NULL, sizeof(StatData) * totalSize, PROT_READ, MAP_SHARED,
                      fd, sizeof(StatData) * i);
    if (BINARYSERIALIZER_UNLIKELY(!addr)) {
      close(fd);
      free(resultData);
      *data = NULL;
      return Error;
    }
    memcpy(resultData, addr, sizeof(StatData) * totalSize);
    msync(addr, sizeof(StatData) * totalSize, MS_ASYNC);
    munmap(addr, sizeof(StatData) * totalSize);
  }

  close(fd);
  *data = resultData;
  *size = fileSize / sizeof(StatData);
  return Success;
}

Status JoinDump(const StatData *__restrict firstData, size_t firstSize,
                const StatData *__restrict secondData, size_t secondSize,
                StatData **__restrict resultData, size_t *resultSize) {
  if (BINARYSERIALIZER_UNLIKELY(((!firstData || firstSize == 0) &&
                                 (!secondData || secondSize == 0)) ||
                                !resultData || !resultSize))
    return InvalidPointerOrSize;

  MergeHashTable table;
  if (BINARYSERIALIZER_UNLIKELY(!initHashTable(&table, NULL, NULL, NULL)))
    return Error;
  size_t maxSize = fmax(firstSize, secondSize);
  for (size_t i = 0; i < maxSize; ++i) {
    if (i < firstSize) {
      if (BINARYSERIALIZER_LIKELY(firstData)) {
        int result = insertToHashTable(&table, firstData + i);
        assert(result == 1);
        if (result != 1) {
          clearHashTable(&table);
          return Error;
        }
      }
    }

    if (i < secondSize) {
      if (BINARYSERIALIZER_LIKELY(secondData)) {
        int result = insertToHashTable(&table, secondData + i);
        assert(result == 1);
        if (result != 1) {
          clearHashTable(&table);
          return Error;
        }
      }
    }
  }

  Status result =
      hashTableToArray(&table, resultData, resultSize) ? Success : Error;
  clearHashTable(&table);

  return result;
}

Status SortDump(StatData *data, size_t size, SortFunction sortFunc) {
  if (BINARYSERIALIZER_UNLIKELY(!data || size == 0 || !sortFunc))
    return InvalidPointerOrSize;
  qsort(data, size, sizeof(StatData), sortFunc);
  return Success;
}

Status PrintDump(const StatData *data, size_t size, size_t linesCount,
                 FormatterFuncion formatter) {
  if (BINARYSERIALIZER_UNLIKELY(!data || size == 0))
    return InvalidPointerOrSize;
  (void)linesCount;
  (void)formatter;
  return Success;
}