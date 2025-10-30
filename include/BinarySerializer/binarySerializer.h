#ifndef BINARYSERIALIZER_BINARYSERIALIZER__H
#define BINARYSERIALIZER_BINARYSERIALIZER__H

#include "BinarySerializer/config.h"
#include "BinarySerializer/statData.h"

#include <stdlib.h>

typedef enum Status { Success, BadFile, InvalidPointerOrSize, Error } Status;

typedef int (*SortFunction)(const void *__restrict lhs,
                            const void *__restrict rhs);

#if defined(__cplusplus)
extern "C" {
#endif

BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API Status
StoreDump(const char *filePath, const StatData *data, size_t size);
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API Status
LoadDump(const char *filePath, StatData **data, size_t *size);
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API Status
JoinDump(const StatData *__restrict firstData, size_t firstSize,
         const StatData *__restrict secondData, size_t secondSize,
         StatData **__restrict resultData, size_t *resultSize);
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API Status
SortDump(StatData *data, size_t size, SortFunction sortFunc);

#if defined(__cplusplus)
}
#endif

#endif // BINARYSERIALIZER_BINARYSERIALIZER__H