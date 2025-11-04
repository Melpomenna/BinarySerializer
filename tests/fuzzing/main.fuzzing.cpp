#include "BinarySerializer/mergeHashTable.h"
#include <fuzztest/fuzztest.h>
#include <gtest/gtest.h>

void TestCreateHashTableWithIDs(const std::vector<int> &ids) {
  MergeHashTable table;
  int result = InitHashTable(&table, NULL, NULL, NULL);
  EXPECT_EQ(result, 1);
  StatData data;
  for (auto id : ids) {
    data.id = id;
    data.cost = 5;
    data.count = 1;
    data.mode = 1;
    data.primary = 0;
    result = InsertToHashTable(&table, &data);
    EXPECT_EQ(result, 1);
  }
  ClearHashTable(&table);
  EXPECT_EQ(table.buckets, nullptr);
  EXPECT_EQ(table.bucketsCount, 0);
  EXPECT_EQ(table.comparator, nullptr);
  EXPECT_EQ(table.merge, nullptr);
  EXPECT_EQ(table.hash, nullptr);
}

FUZZ_TEST(TestSuiteHashTable, TestCreateHashTableWithIDs)
    .WithDomains(fuzztest::VectorOf(fuzztest::InRange(0, 200000)));