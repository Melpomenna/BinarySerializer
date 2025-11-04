#ifndef TESTCASES_H
#define TESTCASES_H

#include "BinarySerializer/binarySerializer.h"

typedef struct TestCase
{
    const char* firstStorePath;
    const char* secondStorePath;
    const char* resultPath;
    StatData* firstIn;
    StatData* secondIn;
    StatData* resultData;
    size_t firstInSize;
    size_t secondInSize;
    size_t resultSize;
} TestCase;


BINARYSERIALIZER_NODISCARD TestCase* GetTestCase(size_t id);
BINARYSERIALIZER_NODISCARD size_t GetTestsCount();

#endif