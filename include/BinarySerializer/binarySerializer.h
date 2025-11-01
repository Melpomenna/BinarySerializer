#ifndef BINARYSERIALIZER_BINARYSERIALIZER__H
#define BINARYSERIALIZER_BINARYSERIALIZER__H

#include "BinarySerializer/config.h"
#include "BinarySerializer/statData.h"

#include <stdlib.h>

typedef enum Status { Success, BadFile, InvalidPointerOrSize, Error } Status;

typedef int (*SortFunction)(const void *__restrict lhs,
                            const void *__restrict rhs);

typedef void (*FormatterFuncion)(const StatData *);

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * @brief
 * @retval
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API Status
StoreDump(const char *filePath, const StatData *data, size_t size);

/*
 * @brief
 * @retval
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API Status
LoadDump(const char *filePath, StatData **data, size_t *size);

/*
 * @brief
 * @retval
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API Status
JoinDump(const StatData *__restrict firstData, size_t firstSize,
         const StatData *__restrict secondData, size_t secondSize,
         StatData **__restrict resultData, size_t *resultSize);

/*
 * @brief
 * @retval
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API Status
SortDump(StatData *data, size_t size, SortFunction sortFunc);

/*
 * @brief
 * @retval
 */
BINARYSERIALIZER_NODISCARD BINARYSERIALIZER_API Status
PrintDump(const StatData *data, size_t size, size_t linesCount,
          FormatterFuncion formatter);

#if defined(__cplusplus)
}
#endif

#endif // BINARYSERIALIZER_BINARYSERIALIZER__H