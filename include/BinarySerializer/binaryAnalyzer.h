#ifndef BINARYSERIALIZER_BINARYSERIALIZER__H
#define BINARYSERIALIZER_BINARYSERIALIZER__H

#include "BinarySerializer/config.h"
#include "BinarySerializer/statData.h"

typedef enum Status
{
    Success,
    BadFile,
    InvalidPointerOrSize
} Status;

#if defined(__cplusplus)
extern "C" {
#endif

Status StoreDump(const char*, const StatData* data, size_t size);
Status LoadDump(const char* file, StatData** data, size_t* size);
Status JoinDump(const StatData* firstData, size_t firstSize, const StatData* secondData, size_t secondSize);
Status SortDump(StatData* data, size_t size, void(*sortFunction)(const StatData*, const StatData*));

#if defined(__cplusplus)
}
#endif

#endif // BINARYSERIALIZER_BINARYSERIALIZER__H