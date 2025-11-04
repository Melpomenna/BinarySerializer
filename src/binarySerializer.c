#include "BinarySerializer/binarySerializer.h"
#include "BinarySerializer/mergeHashTable.h"

#if defined(BS_ENABLE_MI_MALLOC)
#include <mimalloc-override.h>
#else
#include <stdlib.h>
#endif

#include "BinarySerializer/tableView.h"

#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static const size_t butchSize = BINARYSERIALIZER_BUTCHE_SIZE;
static const size_t butchesSizeInBytes = butchSize * sizeof(StatData);

static void CloseFd(const char *filePath, int fd) {
  int result = close(fd);
  assert(result == 0);
  if (result != 0) {
    BINARYSERIALIZER_UNUSED(filePath);
    LOG_ERR("Cannot close fd by [path:%s] [fd:%d]\n", filePath, fd);
  }
}

static void Tmsync(void *addr, size_t len, int flags) {
  int result = msync(addr, len, flags);
  assert(result == 0);
  if (result != 0) {
    LOG_ERR("Failure to sync mapped file for [addr:%p]\n", addr);
  }
}

static void Tmunmap(void *addr, size_t len) {
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
    return INVALID_POINTER_OR_SIZE;
  }
  int fd = open(filePath, O_RDWR);
  if (BINARYSERIALIZER_UNLIKELY(fd < 0)) {
    LOG_ERR("Cannot open file with [path:%s]\n", filePath);
    LOG("[StoreDump end]_____________________\n");
    return BAD_FILE;
  }

  size_t fileSize = sizeof(StatData) * size;
  if (BINARYSERIALIZER_UNLIKELY(ftruncate(fd, fileSize) == -1)) {
    CloseFd(filePath, fd);
    LOG_ERR("Cannot truncate file [filePath:%s] to size [size:%zu]", filePath,
            fileSize);
    LOG("[StoreDump end]_____________________\n");
    return BAD_FILE;
  }
  LOG("[filePath:%s] [fd:%d] [dataSize:%zu] [fileSize:%zu]\n", filePath, fd,
      size, fileSize);
  void *addr =
      mmap(NULL, sizeof(StatData) * size, PROT_WRITE, MAP_SHARED, fd, 0);
  if (BINARYSERIALIZER_UNLIKELY(addr == MAP_FAILED)) {
    CloseFd(filePath, fd);
    LOG_ERR("Cannot mmap file [filePath:%s] with size [size:%zu][line:%d]\n",
            filePath, fileSize, __LINE__);
    LOG("[StoreDump end]_____________________\n");
    return INVALID_POINTER_OR_SIZE;
  }
  memcpy(addr, data, sizeof(StatData) * size);
  Tmsync(addr, sizeof(StatData) * size, MS_ASYNC);
  Tmunmap(addr, sizeof(StatData) * size);
  CloseFd(filePath, fd);
  LOG("[StoreDump end]_____________________\n");
  return SUCCESS;
}

Status LoadDump(const char *filePath, StatData **data, size_t *size) {
  LOG("[LoadDump begin]_____________________\n");
  if (BINARYSERIALIZER_UNLIKELY(!filePath || !data || !size)) {
    LOG_ERR("Bad filePath or data or size\n");
    LOG("[LoadDump end]_____________________\n");
    return INVALID_POINTER_OR_SIZE;
  }
  LOG("[path:%s]\n", filePath);
  int fd = open(filePath, O_RDWR);
  if (BINARYSERIALIZER_UNLIKELY(fd < 0)) {
    LOG_ERR("LoadDump: cannot open file [path:%s]\n", filePath);
    LOG("[LoadDump end]_____________________\n");
    return BAD_FILE;
  }

  size_t fileSize = 0;
  struct stat statBuf;
  int result = fstat(fd, &statBuf);
  if (BINARYSERIALIZER_UNLIKELY(result < 0)) {
    assert(result == 0);
    CloseFd(filePath, fd);
    LOG_ERR("LoadDump: bad result on fstat [result:%d]\n", result);
    LOG("[LoadDump end]_____________________\n");
    return ERROR;
  }
  fileSize = statBuf.st_size;

  if (fileSize == 0 || fileSize < sizeof(StatData)) {
    CloseFd(filePath, fd);
    LOG_ERR("Cannot load dump from empty or zero elements file [paths:%s]\n",
            filePath);
    LOG("[LoadDump end]_____________________\n");
    return EMPTY_FILE;
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
      CloseFd(filePath, fd);
      LOG_ERR("Cannot allocate [bytes:%zu]\n", butchesSizeInBytes * (i + 1));
      LOG("[LoadDump end]_____________________\n");
      return ERROR;
    }
    resultData = rdata;
    void *addr = mmap(baseAddr, butchesSizeInBytes * (i + 1), PROT_READ,
                      MAP_SHARED, fd, 0);
    if (BINARYSERIALIZER_UNLIKELY(addr == MAP_FAILED)) {
      free(resultData);
      CloseFd(filePath, fd);
      LOG_ERR("Cannot mmap file [filePath:%s] with size [size:%zu][line:%d]\n",
              filePath, butchesSizeInBytes, __LINE__);
      LOG("[LoadDump end]_____________________\n");
      return ERROR;
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
      CloseFd(filePath, fd);
      LOG_ERR("Cannot allocate [bytes:%zu]\n", fileSize);
      LOG("[LoadDump end]_____________________\n");
      return ERROR;
    }
    resultData = rdata;
    void *addr = mmap(NULL, fileSize, PROT_READ, MAP_SHARED, fd, 0);
    if (BINARYSERIALIZER_UNLIKELY(addr == MAP_FAILED)) {
      free(resultData);
      CloseFd(filePath, fd);
      LOG_ERR("Cannot mmap file [filePath:%s] with size [size:%zu][line:%d]\n",
              filePath, butchesSizeInBytes, __LINE__);
      LOG("[LoadDump end]_____________________\n");
      return ERROR;
    }
    baseAddr = addr;
    memcpy(resultData + i * butchSize,
           (char *)baseAddr + i * butchesSizeInBytes, totalSize);
  }

  Tmsync(baseAddr, fileSize, MS_ASYNC);
  Tmunmap(baseAddr, fileSize);

  CloseFd(filePath, fd);
  *data = resultData;
  *size = fileSize / sizeof(StatData);
  LOG("[LoadDump end]_____________________\n");
  return SUCCESS;
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
    return INVALID_POINTER_OR_SIZE;
  }

  MergeHashTable table;
  if (BINARYSERIALIZER_UNLIKELY(!InitHashTable(&table, NULL, NULL, NULL))) {
    LOG_ERR("Cannot init MergeHashTable\n");
    LOG("[LoadDump end]_____________________\n");
    return ERROR;
  }
  size_t maxSize = fmax((double)firstSize, (double)secondSize);
  for (size_t i = 0; i < maxSize; ++i) {
    if (i < firstSize) {
      if (BINARYSERIALIZER_LIKELY(firstData)) {
        int result = InsertToHashTable(&table, firstData + i);
        assert(result == 1);
        if (result != 1) {
          ClearHashTable(&table);
          LOG_ERR("Cannot insert value into hash table\n");
          LOG("[LoadDump end]_____________________\n");
          return ERROR;
        }
      }
    }

    if (i < secondSize) {
      if (BINARYSERIALIZER_LIKELY(secondData)) {
        int result = InsertToHashTable(&table, secondData + i);
        assert(result == 1);
        if (result != 1) {
          ClearHashTable(&table);
          LOG_ERR("Cannot insert value into hash table\n");
          LOG("[LoadDump end]_____________________\n");
          return ERROR;
        }
      }
    }
  }

  Status result =
      HashTableToArray(&table, resultData, resultSize) ? SUCCESS : ERROR;
  ClearHashTable(&table);
  if (result != SUCCESS) {
    LOG_ERR("HashTableToArray failed\n");
  }
  LOG("[LoadDump end]_____________________\n");
  return result;
}

Status SortDump(StatData *data, size_t size, SortFunction sortFunc) {
  LOG("[SortDump begin]_____________________\n");
  if (BINARYSERIALIZER_UNLIKELY(!data || size == 0 || !sortFunc)) {
    LOG_ERR("Data is null or empty or null sortFunc\n");
    LOG("[SortDump end]_____________________\n");
    return INVALID_POINTER_OR_SIZE;
  }
  qsort(data, size, sizeof(StatData), sortFunc);
  LOG("[SortDump end]_____________________\n");
  return SUCCESS;
}

Status PrintDump(const StatData *data, size_t size, size_t linesCount,
                 TableView *view) {
  LOG("[PrintDump begin]_____________________\n");
  if (BINARYSERIALIZER_UNLIKELY(!data || size == 0 || !view)) {
    LOG_ERR("Invalid pointer or size\n");
    LOG("[PrintDump end]_____________________\n");
    return INVALID_POINTER_OR_SIZE;
  }
  view->data = data;
  view->dataSize = size;
  view->memSize = sizeof(StatData);

  TablewViewStatus status = PrintTable(view, linesCount);
  if (BINARYSERIALIZER_UNLIKELY(status == TVS_ERROR)) {
    LOG_ERR("Something then wrong with printTable\n");
  }
  LOG("[PrintDump end]_____________________\n");
  return status != TVS_ERROR ? SUCCESS : ERROR;
}