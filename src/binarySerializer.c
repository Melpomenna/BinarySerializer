#include "BinarySerializer/binarySerializer.h"
#include "BinarySerializer/mergeHashTable.h"

#if defined(BS_ENABLE_MI_MALLOC)
#include <mimalloc-override.h>
#endif

#include "tableViewImpl.h"

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
static const size_t butchesSizeInBytes = butchSize * sizeof(StatData);

static void closeFd(const char *filePath, int fd) {
  int result = close(fd);
  assert(result == 0);
  if (result != 0) {
    BINARYSERIALIZER_UNUSED(filePath);
    LOG_ERR("Cannot close fd by [path:%s] [fd:%d]\n", filePath, fd);
  }
}

static void tmsync(void *addr, size_t len, int flags) {
  int result = msync(addr, len, flags);
  assert(result == 0);
  if (result != 0) {
    LOG_ERR("Failure to sync mapped file for [addr:%p]\n", addr);
  }
}

static void tmunmap(void *addr, size_t len) {
  int result = munmap(addr, len);
  assert(result == 0);
  if (result != 0) {
    LOG_ERR("Failure to munmap for [addr:%p]\n", addr);
  }
}

Status StoreDump(const char *filePath, const StatData *data, size_t size) {
  LOG("[StoreDump begin]_____________________\n");
  if (BINARYSERIALIZER_UNLIKELY(!filePath || !data || size == 0)) {
    LOG_ERR("Bad filePath or data or size=0\n");
    LOG("[StoreDump end]_____________________\n");
    return InvalidPointerOrSize;
  }
  int fd = open(filePath, O_RDWR);
  if (BINARYSERIALIZER_UNLIKELY(fd < 0)) {
    LOG_ERR("Cannot open file with [path:%s]\n", filePath);
    LOG("[StoreDump end]_____________________\n");
    return BadFile;
  }

  size_t fileSize = sizeof(StatData) * size;
  if (BINARYSERIALIZER_UNLIKELY(ftruncate(fd, fileSize) == -1)) {
    closeFd(filePath, fd);
    LOG_ERR("Cannot truncate file [filePath:%s] to size [size:%zu]", filePath,
            fileSize);
    LOG("[StoreDump end]_____________________\n");
    return BadFile;
  }
  LOG("[filePath:%s] [fd:%d] [dataSize:%zu] [fileSize:%zu]\n", filePath, fd,
      size, fileSize);
  void *addr =
      mmap(NULL, sizeof(StatData) * size, PROT_WRITE, MAP_SHARED, fd, 0);
  if (BINARYSERIALIZER_UNLIKELY(addr == MAP_FAILED)) {
    closeFd(filePath, fd);
    LOG_ERR("Cannot mmap file [filePath:%s] with size [size:%zu][line:%d]\n",
            filePath, fileSize, __LINE__);
    LOG("[StoreDump end]_____________________\n");
    return InvalidPointerOrSize;
  }
  memcpy(addr, data, sizeof(StatData) * size);
  tmsync(addr, sizeof(StatData) * size, MS_ASYNC);
  tmunmap(addr, sizeof(StatData) * size);
  closeFd(filePath, fd);
  LOG("[StoreDump end]_____________________\n");
  return Success;
}

Status LoadDump(const char *filePath, StatData **data, size_t *size) {
  LOG("[LoadDump begin]_____________________\n");
  if (BINARYSERIALIZER_UNLIKELY(!filePath || !data || !size)) {
    LOG_ERR("Bad filePath or data or size\n");
    LOG("[LoadDump end]_____________________\n");
    return InvalidPointerOrSize;
  }
  LOG("[path:%s]\n", filePath);
  int fd = open(filePath, O_RDWR);
  if (BINARYSERIALIZER_UNLIKELY(fd < 0)) {
    LOG_ERR("LoadDump: cannot open file [path:%s]\n", filePath);
    LOG("[LoadDump end]_____________________\n");
    return BadFile;
  }

  size_t fileSize = 0;
  struct stat statBuf;
  int result = fstat(fd, &statBuf);
  if (BINARYSERIALIZER_UNLIKELY(result < 0)) {
    assert(result == 0);
    closeFd(filePath, fd);
    LOG_ERR("LoadDump: bad result on fstat [result:%d]\n", result);
    LOG("[LoadDump end]_____________________\n");
    return Error;
  }
  fileSize = statBuf.st_size;

  if (fileSize == 0 || fileSize < sizeof(StatData)) {
    closeFd(filePath, fd);
    LOG_ERR("Cannot load dump from empty or zero elements file [paths:%s]\n",
            filePath);
    LOG("[LoadDump end]_____________________\n");
    return EmptyFile;
  }

  LOG("File opened with [size:%zu][StatData size:%zu]\n", fileSize,
      sizeof(StatData));

  fileSize = (fileSize / sizeof(StatData)) * sizeof(StatData);

  LOG("File size in bytes after reducing size by sizeof(StatData) [size:%zu]\n",
      fileSize);

  size_t butchesCount = fileSize / butchesSizeInBytes;
  LOG("LoadDump: butches count [count:%zu]\n", butchesCount);
  size_t totalSize = fileSize;
  StatData *resultData = NULL;
  size_t i = 0;
  void *baseAddr = NULL;
  for (; i < butchesCount && totalSize >= butchesSizeInBytes;
       ++i, totalSize -= butchesSizeInBytes) {
    StatData *rdata = realloc(resultData, butchesSizeInBytes * (i + 1));
    if (BINARYSERIALIZER_UNLIKELY(!rdata)) {
      free(resultData);
      closeFd(filePath, fd);
      LOG_ERR("Cannot allocate [bytes:%zu]\n", butchesSizeInBytes * (i + 1));
      LOG("[LoadDump end]_____________________\n");
      return Error;
    }
    resultData = rdata;
    void *addr = mmap(baseAddr, butchesSizeInBytes * (i + 1), PROT_READ,
                      MAP_SHARED, fd, 0);
    if (BINARYSERIALIZER_UNLIKELY(addr == MAP_FAILED)) {
      free(resultData);
      closeFd(filePath, fd);
      LOG_ERR("Cannot mmap file [filePath:%s] with size [size:%zu][line:%d]\n",
              filePath, butchesSizeInBytes, __LINE__);
      LOG("[LoadDump end]_____________________\n");
      return Error;
    }
    baseAddr = addr;
    memcpy(resultData + i * butchSize,
           (char *)baseAddr + i * butchesSizeInBytes, butchesSizeInBytes);
  }
  LOG("Total size after butching: [size:%zu]\n", totalSize);

  if (totalSize != 0) {
    StatData *rdata = realloc(resultData, fileSize);
    if (BINARYSERIALIZER_UNLIKELY(!rdata)) {
      free(resultData);
      closeFd(filePath, fd);
      LOG_ERR("Cannot allocate [bytes:%zu]\n", fileSize);
      LOG("[LoadDump end]_____________________\n");
      return Error;
    }
    resultData = rdata;
    void *addr = mmap(NULL, fileSize, PROT_READ, MAP_SHARED, fd, 0);
    if (BINARYSERIALIZER_UNLIKELY(addr == MAP_FAILED)) {
      free(resultData);
      closeFd(filePath, fd);
      LOG_ERR("Cannot mmap file [filePath:%s] with size [size:%zu][line:%d]\n",
              filePath, butchesSizeInBytes, __LINE__);
      LOG("[LoadDump end]_____________________\n");
      return Error;
    }
    baseAddr = addr;
    memcpy(resultData + i * butchSize,
           (char *)baseAddr + i * butchesSizeInBytes, totalSize);
  }

  tmsync(baseAddr, fileSize, MS_ASYNC);
  tmunmap(baseAddr, fileSize);

  closeFd(filePath, fd);
  *data = resultData;
  *size = fileSize / sizeof(StatData);
  LOG("[LoadDump end]_____________________\n");
  return Success;
}

Status JoinDump(const StatData *__restrict firstData, size_t firstSize,
                const StatData *__restrict secondData, size_t secondSize,
                StatData **__restrict resultData, size_t *resultSize) {
  LOG("[JoinDump begin]_____________________\n");
  if (BINARYSERIALIZER_UNLIKELY(((!firstData || firstSize == 0) &&
                                 (!secondData || secondSize == 0)) ||
                                !resultData || !resultSize)) {
    LOG_ERR("All data or result data is null or empty\n");
    LOG("[LoadDump end]_____________________\n");
    return InvalidPointerOrSize;
  }

  MergeHashTable table;
  if (BINARYSERIALIZER_UNLIKELY(!initHashTable(&table, NULL, NULL, NULL))) {
    LOG_ERR("Cannot init MergeHashTable\n");
    LOG("[LoadDump end]_____________________\n");
    return Error;
  }
  size_t maxSize = fmax(firstSize, secondSize);
  for (size_t i = 0; i < maxSize; ++i) {
    if (i < firstSize) {
      if (BINARYSERIALIZER_LIKELY(firstData)) {
        int result = insertToHashTable(&table, firstData + i);
        assert(result == 1);
        if (result != 1) {
          clearHashTable(&table);
          LOG_ERR("Cannot insert value into hash table\n");
          LOG("[LoadDump end]_____________________\n");
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
          LOG_ERR("Cannot insert value into hash table\n");
          LOG("[LoadDump end]_____________________\n");
          return Error;
        }
      }
    }
  }

  Status result =
      hashTableToArray(&table, resultData, resultSize) ? Success : Error;
  clearHashTable(&table);
  if (result != Success) {
    LOG_ERR("hashTableToArray failed\n");
  }
  LOG("[LoadDump end]_____________________\n");
  return result;
}

Status SortDump(StatData *data, size_t size, SortFunction sortFunc) {
  if (BINARYSERIALIZER_UNLIKELY(!data || size == 0 || !sortFunc))
    return InvalidPointerOrSize;
  qsort(data, size, sizeof(StatData), sortFunc);
  return Success;
}

Status PrintDump(const StatData *data, size_t size, size_t linesCount,
                 TableView *view) {
  LOG("[PrintDump begin]_____________________\n");
  if (BINARYSERIALIZER_UNLIKELY(!data || size == 0 || !view)) {
    LOG_ERR("Invalid pointer or size\n");
    LOG("[PrintDump end]_____________________\n");
    return InvalidPointerOrSize;
  }
  view->data = data;
  view->dataSize = size;
  view->memSize = sizeof(StatData);

  TablewViewStatus status = printTable(view, linesCount);
  if (BINARYSERIALIZER_UNLIKELY(status == TVSError)) {
    LOG_ERR("Something then wrong with printTable\n");
  }
  LOG("[PrintDump end]_____________________\n");
  return status != TVSError ? Success : Error;
}