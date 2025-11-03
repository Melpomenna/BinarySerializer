#include "timeHelper.h"
#include <sys/time.h>

#include <stdlib.h>

typedef void *time_handle_t;

BINARYSERIALIZER_NODISCARD time_handle_t beginTime() {
  struct timeval *time = calloc(1, sizeof(struct timeval));

  if (BINARYSERIALIZER_LIKELY(time)) {
    gettimeofday(time, NULL);
  }

  return time;
}

BINARYSERIALIZER_NODISCARD double endTimeMS(time_handle_t startHandle) {
  if (BINARYSERIALIZER_UNLIKELY(!startHandle)) {
    return -1;
  }
  time_handle_t end = beginTime();
  if (BINARYSERIALIZER_UNLIKELY(!end)) {
    return -1;
  }
  struct timeval *beginTime = startHandle, *endTime = end;
  double elapsedMs = 0.0;
  long long startt = 0, endt = 0;
  startt = beginTime->tv_sec * 1000000 + beginTime->tv_usec;
  endt = endTime->tv_sec * 1000000 + endTime->tv_usec;

  elapsedMs = (double)(endt - startt) / 1000.0;

  clearTime(end);
  return elapsedMs;
}

void clearTime(time_handle_t handle) { free(handle); }