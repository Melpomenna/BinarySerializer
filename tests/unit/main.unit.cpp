#include "BinarySerializer/binarySerializer.h"
#include "BinarySerializer/mergeHashTable.h"
#include <gtest/gtest.h>

#if defined(BS_ENABLE_MI_MALLOC)
#include <mimalloc-override.h>
#else
#include <stdlib.h>
#endif

#include <math.h>
#include <stdio.h>

namespace {

#pragma region TestCase1Base
static StatData testCase1[] = {
    {.id = 90889, .count = 13, .cost = 3.567, .primary = 0, .mode = 3},
    {.id = 90089, .count = 1, .cost = 88.90, .primary = 1, .mode = 0}};
static StatData testCase2[] = {
    {.id = 90089, .count = 13, .cost = 0.011, .primary = 0, .mode = 2},
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode = 2}};

static StatData testCaseResult1[] = {
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode = 2},
    {.id = 90889, .count = 13, .cost = 3.567, .primary = 0, .mode = 3},
    {.id = 90089, .count = 14, .cost = 88.911, .primary = 0, .mode = 2}};
#pragma endregion

#pragma region TestCase3Base
static StatData testCase3[] = {
    {.id = 90889, .count = 13, .cost = 100, .primary = 1, .mode = 3},
    {.id = 90089, .count = 1, .cost = 88.90, .primary = 0, .mode = 5}};
static StatData testCase31[] = {
    {.id = 90089, .count = 13, .cost = 0.1, .primary = 1, .mode = 2},
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode = 2}};

static StatData testCaseResult3[] = {
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode = 2},
    {.id = 90089, .count = 14, .cost = 89, .primary = 0, .mode = 5},
    {.id = 90889, .count = 13, .cost = 100, .primary = 1, .mode = 3},
};

#pragma endregion

#pragma region TestCase4Base
static StatData testCase4[] = {
    {.id = 90889, .count = 13, .cost = 100, .primary = 1, .mode = 3},
    {.id = 90089, .count = 1, .cost = 88.90, .primary = 0, .mode = 5}};
static StatData testCase41[] = {
    {.id = 90089, .count = 13, .cost = 0.1, .primary = 1, .mode = 7},
    {.id = 90089, .count = 13, .cost = 0, .primary = 1, .mode = 4},
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode = 2}};

static StatData testCaseResult4[] = {
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode = 2},
    {.id = 90089, .count = 27, .cost = 89, .primary = 0, .mode = 7},
    {.id = 90889, .count = 13, .cost = 100, .primary = 1, .mode = 3},
};

#pragma endregion

#pragma region TestCase5Base
static StatData testCase5[] = {
    {.id = 90189, .count = 1, .cost = -8.008, .primary = 1, .mode = 3},
    {.id = 90189, .count = 1, .cost = 1.001, .primary = 0, .mode = 5},
    {.id = 90189, .count = 1, .cost = 1.001, .primary = 1, .mode = 3},
    {.id = 90189, .count = 1, .cost = 1.001, .primary = 0, .mode = 5},
    {.id = 90189, .count = 1, .cost = 1.001, .primary = 1, .mode = 3},
    {.id = 90189, .count = 1, .cost = 1.001, .primary = 0, .mode = 5}};
static StatData testCase51[] = {
    {.id = 90189, .count = 1, .cost = 1.001, .primary = 1, .mode = 7},
    {.id = 90189, .count = 1, .cost = 1.001, .primary = 1, .mode = 4},
    {.id = 90189, .count = 1, .cost = 1.001, .primary = 1, .mode = 2}};

static StatData testCaseResult5[] = {
    {.id = 90189, .count = 9, .cost = 0, .primary = 0, .mode = 7}};

#pragma endregion

typedef struct TestCase {
  const char *firstStorePath;
  const char *secondStorePath;
  const char *resultPath;
  StatData *firstIn;
  StatData *secondIn;
  StatData *resultData;
  size_t firstInSize;
  size_t secondInSize;
  size_t resultSize;
} TestCase;

#define DECLARE_CASE(__fpath, __spath, __rpath, __in1, __in2, __out)           \
  {                                                                            \
    __fpath, __spath, __rpath, __in1, __in2, __out, std::size(__in1),          \
        std::size(__in2), std::size(__out)                                     \
  }

static TestCase cases[] = {
    DECLARE_CASE("case_1_in_a.dat", "case_1_in_b.dat", "case_1_out.dat",
                 testCase1, testCase2, testCaseResult1), // SUCCESS TestCase 0
    DECLARE_CASE("case_3_in_a.dat", "case_3_in_b.dat", "case_3_out.dat",
                 testCase3, testCase31, testCaseResult3), // SUCCESS TestCase 1
    DECLARE_CASE("case_4_in_a.dat", "case_4_in_b.dat", "case_4_out.dat",
                 testCase4, testCase41, testCaseResult4), // SUCCESS TestCase 2
    DECLARE_CASE("case_5_in_a.dat", "case_5_in_b.dat", "case_5_out.dat",
                 testCase5, testCase51, testCaseResult5), // SUCCESS TestCase 3
};

static int SortStatDataFunc(const void *__restrict lhs,
                            const void *__restrict rhs) {
  const StatData *sdlhs = reinterpret_cast<const StatData *>(lhs);
  const StatData *sdrhs = reinterpret_cast<const StatData *>(rhs);
  return sdlhs->cost > sdrhs->cost;
}

int SortStatDataByID(const void *__restrict lhs, const void *__restrict rhs) {
  const StatData *dlhs = reinterpret_cast<const StatData *>(lhs);
  const StatData *drhs = reinterpret_cast<const StatData *>(rhs);
  return dlhs->id < drhs->id;
}

HashT HashFunctionBase(const StatData *data) { return data->id % 5; }

void MergeFunctionBase(StatData *__restrict lhs,
                       const StatData *__restrict rhs) {
  lhs->cost += rhs->cost;
}

int StatDataCompareFunctionDefault(const StatData *__restrict lhs,
                                   const StatData *__restrict rhs) {
  return lhs->id == rhs->id;
}

void CheckEqualData(const StatData *d1, size_t size1, const StatData *d2,
                    size_t size2) {
  ASSERT_EQ(size1, size2);
  for (size_t i = 0; i < size1; ++i) {
    ASSERT_EQ(d1->id, d2->id);
    ASSERT_LE(fabsf(d1->cost - d2->cost), 1e-5);
    ASSERT_EQ(d1->count, d2->count);
    ASSERT_EQ(d1->primary, d2->primary);
    ASSERT_EQ(d1->mode, d2->mode);
  }
}

} // namespace

TEST(MergeHashTable, InitializeSuccess) {

  MergeHashTable table;
  int result = InitHashTable(&table, nullptr, nullptr, nullptr);
  ASSERT_EQ(result, 1);
  ASSERT_NE(table.buckets, nullptr);
  ASSERT_EQ(table.bucketsCount, BINARYSERIALIZER_DEFUALT_BUCKETS_COUNT);
  ASSERT_NE(table.comparator, nullptr);
  ASSERT_NE(table.merge, nullptr);
  ASSERT_NE(table.hash, nullptr);
  ClearHashTable(&table);
  ASSERT_EQ(table.buckets, nullptr);
  ASSERT_EQ(table.bucketsCount, 0);
  ASSERT_EQ(table.comparator, nullptr);
  ASSERT_EQ(table.merge, nullptr);
  ASSERT_EQ(table.hash, nullptr);
}

TEST(MergeHashTable, InitializeSuccessWithCustomFunctions) {

  MergeHashTable table;
  int result = InitHashTable(&table, &HashFunctionBase, &MergeFunctionBase,
                             &StatDataCompareFunctionDefault);
  ASSERT_EQ(result, 1);
  ASSERT_NE(table.buckets, nullptr);
  ASSERT_EQ(table.bucketsCount, BINARYSERIALIZER_DEFUALT_BUCKETS_COUNT);
  ASSERT_EQ(table.comparator, &StatDataCompareFunctionDefault);
  ASSERT_EQ(table.merge, &MergeFunctionBase);
  ASSERT_EQ(table.hash, &HashFunctionBase);
  ClearHashTable(&table);
  ASSERT_EQ(table.buckets, nullptr);
  ASSERT_EQ(table.bucketsCount, 0);
  ASSERT_EQ(table.comparator, nullptr);
  ASSERT_EQ(table.merge, nullptr);
  ASSERT_EQ(table.hash, nullptr);
}

TEST(MergeHashTable, InitHashTableNullPointer) {
  int result = InitHashTable(nullptr, nullptr, nullptr, nullptr);

  EXPECT_EQ(result, 0);
}

TEST(MergeHashTable, InsertNoNullData) {

  MergeHashTable table;
  int result = InitHashTable(&table, nullptr, nullptr, nullptr);
  ASSERT_EQ(result, 1);
  ASSERT_NE(table.buckets, nullptr);
  ASSERT_EQ(table.bucketsCount, BINARYSERIALIZER_DEFUALT_BUCKETS_COUNT);
  ASSERT_NE(table.comparator, nullptr);
  ASSERT_NE(table.merge, nullptr);
  ASSERT_NE(table.hash, nullptr);

  StatData data;
  data.id = 5;
  data.cost = 15;
  data.count = 15;
  data.mode = 0;
  data.primary = 1;
  result = InsertToHashTable(&table, &data);
  ASSERT_EQ(result, 1);
  struct Node *node = FindInHashTable(&table, &data);
  ASSERT_NE(node, nullptr);
  ClearHashTable(&table);
  ASSERT_EQ(table.buckets, nullptr);
  ASSERT_EQ(table.bucketsCount, 0);
  ASSERT_EQ(table.comparator, nullptr);
  ASSERT_EQ(table.merge, nullptr);
  ASSERT_EQ(table.hash, nullptr);
}

TEST(MergeHashTable, InsertNullData) {

  MergeHashTable table;
  int result = InitHashTable(&table, nullptr, nullptr, nullptr);
  ASSERT_EQ(result, 1);
  ASSERT_NE(table.buckets, nullptr);
  ASSERT_EQ(table.bucketsCount, BINARYSERIALIZER_DEFUALT_BUCKETS_COUNT);
  ASSERT_NE(table.comparator, nullptr);
  ASSERT_NE(table.merge, nullptr);
  ASSERT_NE(table.hash, nullptr);

  result = InsertToHashTable(&table, NULL);
  ASSERT_EQ(result, 0);
  ClearHashTable(&table);
  ASSERT_EQ(table.buckets, nullptr);
  ASSERT_EQ(table.bucketsCount, 0);
  ASSERT_EQ(table.comparator, nullptr);
  ASSERT_EQ(table.merge, nullptr);
  ASSERT_EQ(table.hash, nullptr);
}

TEST(MergeHashTable, InsertDublicateData) {

  MergeHashTable table;
  int result = InitHashTable(&table, nullptr, nullptr, nullptr);
  ASSERT_EQ(result, 1);
  ASSERT_NE(table.buckets, nullptr);
  ASSERT_EQ(table.bucketsCount, BINARYSERIALIZER_DEFUALT_BUCKETS_COUNT);
  ASSERT_NE(table.comparator, nullptr);
  ASSERT_NE(table.merge, nullptr);
  ASSERT_NE(table.hash, nullptr);

  StatData data;
  data.id = 5;
  data.cost = 15;
  data.count = 15;
  data.mode = 0;
  data.primary = 1;
  result = InsertToHashTable(&table, &data);
  ASSERT_EQ(result, 1);
  struct Node *node = FindInHashTable(&table, &data);
  result = InsertToHashTable(&table, &data);
  ASSERT_EQ(result, 1);
  node = FindInHashTable(&table, &data);
  ASSERT_NE(node, nullptr);

  StatData *rdata = nullptr;
  size_t size = 0;
  result = HashTableToArray(&table, &rdata, &size);
  ASSERT_EQ(result, 1);
  ASSERT_EQ(size, 1);
  ASSERT_EQ(rdata->id, 5);
  ASSERT_EQ(rdata->cost, 30);
  ASSERT_EQ(rdata->count, 30);
  ASSERT_EQ(rdata->mode, 0);
  ASSERT_EQ(rdata->primary, 1);

  free(rdata);

  ClearHashTable(&table);
  ASSERT_EQ(table.buckets, nullptr);
  ASSERT_EQ(table.bucketsCount, 0);
  ASSERT_EQ(table.comparator, nullptr);
  ASSERT_EQ(table.merge, nullptr);
  ASSERT_EQ(table.hash, nullptr);
}

TEST(MergeHashTable, ClearNullTable) {
  ClearHashTable(nullptr); // do not terminate
  ASSERT_EQ(1, 1);
}

TEST(MergeHashTable, EraseDataNullTable) {
  StatData data;
  EraseFromHashTable(nullptr, &data); // do not terminate
  ASSERT_EQ(1, 1);
}

TEST(MergeHashTable, EraseDataNullData) {
  MergeHashTable table;
  int result = InitHashTable(&table, nullptr, nullptr, nullptr);
  ASSERT_EQ(result, 1);
  ASSERT_NE(table.buckets, nullptr);
  ASSERT_EQ(table.bucketsCount, BINARYSERIALIZER_DEFUALT_BUCKETS_COUNT);
  ASSERT_NE(table.comparator, nullptr);
  ASSERT_NE(table.merge, nullptr);
  ASSERT_NE(table.hash, nullptr);

  EraseFromHashTable(&table, nullptr); // do not terminate

  ClearHashTable(&table);
  ASSERT_EQ(table.buckets, nullptr);
  ASSERT_EQ(table.bucketsCount, 0);
  ASSERT_EQ(table.comparator, nullptr);
  ASSERT_EQ(table.merge, nullptr);
  ASSERT_EQ(table.hash, nullptr);
}

TEST(MergeHashTable, EraseDataOneSize) {
  MergeHashTable table;
  int result = InitHashTable(&table, nullptr, nullptr, nullptr);
  ASSERT_EQ(result, 1);
  ASSERT_NE(table.buckets, nullptr);
  ASSERT_EQ(table.bucketsCount, BINARYSERIALIZER_DEFUALT_BUCKETS_COUNT);
  ASSERT_NE(table.comparator, nullptr);
  ASSERT_NE(table.merge, nullptr);
  ASSERT_NE(table.hash, nullptr);

  StatData data;
  data.id = 5;
  data.cost = 15;
  data.count = 15;
  data.mode = 0;
  data.primary = 1;
  result = InsertToHashTable(&table, &data);
  ASSERT_EQ(result, 1);
  struct Node *node = FindInHashTable(&table, &data);
  ASSERT_NE(node, nullptr);

  EraseFromHashTable(&table, &data);
  node = FindInHashTable(&table, &data);
  ASSERT_EQ(node, nullptr);

  ClearHashTable(&table);
  ASSERT_EQ(table.buckets, nullptr);
  ASSERT_EQ(table.bucketsCount, 0);
  ASSERT_EQ(table.comparator, nullptr);
  ASSERT_EQ(table.merge, nullptr);
  ASSERT_EQ(table.hash, nullptr);
}

TEST(MergeHashTable, InsertNotEqualData) {

  MergeHashTable table;
  int result = InitHashTable(&table, nullptr, nullptr, nullptr);
  ASSERT_EQ(result, 1);
  ASSERT_NE(table.buckets, nullptr);
  ASSERT_EQ(table.bucketsCount, BINARYSERIALIZER_DEFUALT_BUCKETS_COUNT);
  ASSERT_NE(table.comparator, nullptr);
  ASSERT_NE(table.merge, nullptr);
  ASSERT_NE(table.hash, nullptr);

  StatData data;
  data.id = 5;
  data.cost = 15;
  data.count = 15;
  data.mode = 0;
  data.primary = 1;
  result = InsertToHashTable(&table, &data);
  ASSERT_EQ(result, 1);
  struct Node *node = FindInHashTable(&table, &data);
  data.id = 15;
  result = InsertToHashTable(&table, &data);
  ASSERT_EQ(result, 1);
  node = FindInHashTable(&table, &data);
  ASSERT_NE(node, nullptr);

  StatData *rdata = nullptr;
  size_t size = 0;
  result = HashTableToArray(&table, &rdata, &size);
  ASSERT_EQ(result, 1);
  result = SortDump(rdata, size, &SortStatDataByID);
  ASSERT_EQ(size, 2);
  ASSERT_EQ(rdata[0].id, 15);
  ASSERT_EQ(rdata[0].cost, 15);
  ASSERT_EQ(rdata[0].count, 15);
  ASSERT_EQ(rdata[0].mode, 0);
  ASSERT_EQ(rdata[0].primary, 1);

  ASSERT_EQ(rdata[1].id, 5);
  ASSERT_EQ(rdata[1].cost, 15);
  ASSERT_EQ(rdata[1].count, 15);
  ASSERT_EQ(rdata[1].mode, 0);
  ASSERT_EQ(rdata[1].primary, 1);

  free(rdata);

  ClearHashTable(&table);
  ASSERT_EQ(table.buckets, nullptr);
  ASSERT_EQ(table.bucketsCount, 0);
  ASSERT_EQ(table.comparator, nullptr);
  ASSERT_EQ(table.merge, nullptr);
  ASSERT_EQ(table.hash, nullptr);
}

TEST(MergeHashTable, ForeachElementNullTable) {
  ForeachElementInHashTable(nullptr, nullptr, nullptr);
  ASSERT_EQ(1, 1);
}

TEST(BaseAPI, TestCaseBase1Success) {
  size_t i = 0;
  FILE *fd1 = fopen(cases[i].firstStorePath, "wb+");
  FILE *fd2 = fopen(cases[i].secondStorePath, "wb+");
  FILE *fd3 = fopen(cases[i].resultPath, "ab+");
  ASSERT_NE(fd1, nullptr);
  ASSERT_NE(fd2, nullptr);
  ASSERT_NE(fd3, nullptr);
  fclose(fd1);
  fclose(fd2);
  fclose(fd3);
  Status result = StoreDump(cases[i].firstStorePath, cases[i].firstIn,
                            cases[i].firstInSize);
  ASSERT_EQ(result, SUCCESS);
  result = StoreDump(cases[i].secondStorePath, cases[i].secondIn,
                     cases[i].secondInSize);
  ASSERT_EQ(result, SUCCESS);
  StatData *r1 = nullptr;
  size_t r1Size = 0;
  result = LoadDump(cases[i].firstStorePath, &r1, &r1Size);
  ASSERT_EQ(result, SUCCESS);
  ASSERT_NE(r1, nullptr);
  ASSERT_EQ(r1Size, cases[i].firstInSize);

  CheckEqualData(r1, r1Size, cases[i].firstIn, cases[i].firstInSize);

  StatData *r2 = nullptr;
  size_t r2Size = 0;
  result = LoadDump(cases[i].secondStorePath, &r2, &r2Size);
  ASSERT_EQ(result, SUCCESS);
  ASSERT_NE(r2, nullptr);
  ASSERT_EQ(r2Size, cases[i].secondInSize);

  CheckEqualData(r2, r2Size, cases[i].secondIn, cases[i].secondInSize);

  StatData *joinedData = nullptr;
  size_t joinedSize = 0;
  result = JoinDump(r1, r1Size, r2, r2Size, &joinedData, &joinedSize);
  ASSERT_EQ(result, SUCCESS);
  ASSERT_NE(joinedData, nullptr);
  ASSERT_EQ(joinedSize, cases[i].resultSize);
  result = SortDump(joinedData, joinedSize, &SortStatDataFunc);
  ASSERT_EQ(result, SUCCESS);

  CheckEqualData(joinedData, joinedSize, cases[i].resultData,
                 cases[i].resultSize);

  result = StoreDump(cases[i].resultPath, joinedData, joinedSize);
  ASSERT_EQ(result, SUCCESS);

  StatData *loaded = nullptr;
  size_t loadedSize = 0;
  result = LoadDump(cases[i].resultPath, &loaded, &loadedSize);
  ASSERT_EQ(result, SUCCESS);
  ASSERT_NE(loaded, nullptr);
  ASSERT_EQ(loadedSize, cases[i].resultSize);

  CheckEqualData(loaded, loadedSize, cases[i].resultData, cases[i].resultSize);

  free(r1);
  free(r2);
  free(joinedData);
  free(loaded);
  joinedData = nullptr;
  r1 = nullptr;
  r2 = nullptr;

  remove(cases[i].firstStorePath);
  remove(cases[i].secondStorePath);
  remove(cases[i].resultPath);
}

TEST(BaseAPI, TestCaseBase2Success) {
  size_t i = 1;
  FILE *fd1 = fopen(cases[i].firstStorePath, "wb+");
  FILE *fd2 = fopen(cases[i].secondStorePath, "wb+");
  FILE *fd3 = fopen(cases[i].resultPath, "ab+");
  ASSERT_NE(fd1, nullptr);
  ASSERT_NE(fd2, nullptr);
  ASSERT_NE(fd3, nullptr);
  fclose(fd1);
  fclose(fd2);
  fclose(fd3);
  Status result = StoreDump(cases[i].firstStorePath, cases[i].firstIn,
                            cases[i].firstInSize);
  ASSERT_EQ(result, SUCCESS);
  result = StoreDump(cases[i].secondStorePath, cases[i].secondIn,
                     cases[i].secondInSize);
  ASSERT_EQ(result, SUCCESS);
  StatData *r1 = nullptr;
  size_t r1Size = 0;
  result = LoadDump(cases[i].firstStorePath, &r1, &r1Size);
  ASSERT_EQ(result, SUCCESS);
  ASSERT_NE(r1, nullptr);
  ASSERT_EQ(r1Size, cases[i].firstInSize);

  CheckEqualData(r1, r1Size, cases[i].firstIn, cases[i].firstInSize);

  StatData *r2 = nullptr;
  size_t r2Size = 0;
  result = LoadDump(cases[i].secondStorePath, &r2, &r2Size);
  ASSERT_EQ(result, SUCCESS);
  ASSERT_NE(r2, nullptr);
  ASSERT_EQ(r2Size, cases[i].secondInSize);

  CheckEqualData(r2, r2Size, cases[i].secondIn, cases[i].secondInSize);

  StatData *joinedData = nullptr;
  size_t joinedSize = 0;
  result = JoinDump(r1, r1Size, r2, r2Size, &joinedData, &joinedSize);
  ASSERT_EQ(result, SUCCESS);
  ASSERT_NE(joinedData, nullptr);
  ASSERT_EQ(joinedSize, cases[i].resultSize);
  result = SortDump(joinedData, joinedSize, &SortStatDataFunc);
  ASSERT_EQ(result, SUCCESS);

  CheckEqualData(joinedData, joinedSize, cases[i].resultData,
                 cases[i].resultSize);

  result = StoreDump(cases[i].resultPath, joinedData, joinedSize);
  ASSERT_EQ(result, SUCCESS);

  StatData *loaded = nullptr;
  size_t loadedSize = 0;
  result = LoadDump(cases[i].resultPath, &loaded, &loadedSize);
  ASSERT_EQ(result, SUCCESS);
  ASSERT_NE(loaded, nullptr);
  ASSERT_EQ(loadedSize, cases[i].resultSize);

  CheckEqualData(loaded, loadedSize, cases[i].resultData, cases[i].resultSize);

  free(r1);
  free(r2);
  free(joinedData);
  free(loaded);
  joinedData = nullptr;
  r1 = nullptr;
  r2 = nullptr;

  remove(cases[i].firstStorePath);
  remove(cases[i].secondStorePath);
  remove(cases[i].resultPath);
}

TEST(BaseAPI, TestCaseBase3Success) {
  size_t i = 2;
  FILE *fd1 = fopen(cases[i].firstStorePath, "wb+");
  FILE *fd2 = fopen(cases[i].secondStorePath, "wb+");
  FILE *fd3 = fopen(cases[i].resultPath, "ab+");
  ASSERT_NE(fd1, nullptr);
  ASSERT_NE(fd2, nullptr);
  ASSERT_NE(fd3, nullptr);
  fclose(fd1);
  fclose(fd2);
  fclose(fd3);
  Status result = StoreDump(cases[i].firstStorePath, cases[i].firstIn,
                            cases[i].firstInSize);
  ASSERT_EQ(result, SUCCESS);
  result = StoreDump(cases[i].secondStorePath, cases[i].secondIn,
                     cases[i].secondInSize);
  ASSERT_EQ(result, SUCCESS);
  StatData *r1 = nullptr;
  size_t r1Size = 0;
  result = LoadDump(cases[i].firstStorePath, &r1, &r1Size);
  ASSERT_EQ(result, SUCCESS);
  ASSERT_NE(r1, nullptr);
  ASSERT_EQ(r1Size, cases[i].firstInSize);

  CheckEqualData(r1, r1Size, cases[i].firstIn, cases[i].firstInSize);

  StatData *r2 = nullptr;
  size_t r2Size = 0;
  result = LoadDump(cases[i].secondStorePath, &r2, &r2Size);
  ASSERT_EQ(result, SUCCESS);
  ASSERT_NE(r2, nullptr);
  ASSERT_EQ(r2Size, cases[i].secondInSize);

  CheckEqualData(r2, r2Size, cases[i].secondIn, cases[i].secondInSize);

  StatData *joinedData = nullptr;
  size_t joinedSize = 0;
  result = JoinDump(r1, r1Size, r2, r2Size, &joinedData, &joinedSize);
  ASSERT_EQ(result, SUCCESS);
  ASSERT_NE(joinedData, nullptr);
  ASSERT_EQ(joinedSize, cases[i].resultSize);
  result = SortDump(joinedData, joinedSize, &SortStatDataFunc);
  ASSERT_EQ(result, SUCCESS);

  CheckEqualData(joinedData, joinedSize, cases[i].resultData,
                 cases[i].resultSize);

  result = StoreDump(cases[i].resultPath, joinedData, joinedSize);
  ASSERT_EQ(result, SUCCESS);

  StatData *loaded = nullptr;
  size_t loadedSize = 0;
  result = LoadDump(cases[i].resultPath, &loaded, &loadedSize);
  ASSERT_EQ(result, SUCCESS);
  ASSERT_NE(loaded, nullptr);
  ASSERT_EQ(loadedSize, cases[i].resultSize);

  CheckEqualData(loaded, loadedSize, cases[i].resultData, cases[i].resultSize);

  free(r1);
  free(r2);
  free(joinedData);
  free(loaded);
  joinedData = nullptr;
  r1 = nullptr;
  r2 = nullptr;

  remove(cases[i].firstStorePath);
  remove(cases[i].secondStorePath);
  remove(cases[i].resultPath);
}

TEST(BaseAPI, TestCaseBase4Success) {
  size_t i = 3;
  FILE *fd1 = fopen(cases[i].firstStorePath, "wb+");
  FILE *fd2 = fopen(cases[i].secondStorePath, "wb+");
  FILE *fd3 = fopen(cases[i].resultPath, "ab+");
  ASSERT_NE(fd1, nullptr);
  ASSERT_NE(fd2, nullptr);
  ASSERT_NE(fd3, nullptr);
  fclose(fd1);
  fclose(fd2);
  fclose(fd3);
  Status result = StoreDump(cases[i].firstStorePath, cases[i].firstIn,
                            cases[i].firstInSize);
  ASSERT_EQ(result, SUCCESS);
  result = StoreDump(cases[i].secondStorePath, cases[i].secondIn,
                     cases[i].secondInSize);
  ASSERT_EQ(result, SUCCESS);
  StatData *r1 = nullptr;
  size_t r1Size = 0;
  result = LoadDump(cases[i].firstStorePath, &r1, &r1Size);
  ASSERT_EQ(result, SUCCESS);
  ASSERT_NE(r1, nullptr);
  ASSERT_EQ(r1Size, cases[i].firstInSize);

  CheckEqualData(r1, r1Size, cases[i].firstIn, cases[i].firstInSize);

  StatData *r2 = nullptr;
  size_t r2Size = 0;
  result = LoadDump(cases[i].secondStorePath, &r2, &r2Size);
  ASSERT_EQ(result, SUCCESS);
  ASSERT_NE(r2, nullptr);
  ASSERT_EQ(r2Size, cases[i].secondInSize);

  CheckEqualData(r2, r2Size, cases[i].secondIn, cases[i].secondInSize);

  StatData *joinedData = nullptr;
  size_t joinedSize = 0;
  result = JoinDump(r1, r1Size, r2, r2Size, &joinedData, &joinedSize);
  ASSERT_EQ(result, SUCCESS);
  ASSERT_NE(joinedData, nullptr);
  ASSERT_EQ(joinedSize, cases[i].resultSize);
  result = SortDump(joinedData, joinedSize, &SortStatDataFunc);
  ASSERT_EQ(result, SUCCESS);

  CheckEqualData(joinedData, joinedSize, cases[i].resultData,
                 cases[i].resultSize);

  result = StoreDump(cases[i].resultPath, joinedData, joinedSize);
  ASSERT_EQ(result, SUCCESS);

  StatData *loaded = nullptr;
  size_t loadedSize = 0;
  result = LoadDump(cases[i].resultPath, &loaded, &loadedSize);
  ASSERT_EQ(result, SUCCESS);
  ASSERT_NE(loaded, nullptr);
  ASSERT_EQ(loadedSize, cases[i].resultSize);

  CheckEqualData(loaded, loadedSize, cases[i].resultData, cases[i].resultSize);

  free(r1);
  free(r2);
  free(joinedData);
  free(loaded);
  joinedData = nullptr;
  r1 = nullptr;
  r2 = nullptr;

  remove(cases[i].firstStorePath);
  remove(cases[i].secondStorePath);
  remove(cases[i].resultPath);
}