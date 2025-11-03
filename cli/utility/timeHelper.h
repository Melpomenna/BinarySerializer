#ifndef TIMEHELPER_H
#define TIMEHELPER_H

#include "BinarySerializer/config.h"

typedef void* time_handle_t;

BINARYSERIALIZER_NODISCARD time_handle_t beginTime();

BINARYSERIALIZER_NODISCARD double endTimeMS(time_handle_t);

void clearTime(time_handle_t handle);

#endif // TIMEHELPER_H