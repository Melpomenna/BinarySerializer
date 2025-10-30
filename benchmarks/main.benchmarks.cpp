#include "BinarySerializer/binarySerializer.h"
#include "internal/hashTable.h"

#include <benchmark/benchmark.h>
#include <memory>
#include <random>

static void
TestInsertElementsWithUniquesId([[maybe_unused]] benchmark::State &state) {
  for ([[maybe_unused]] const auto &_ : state) {
    MergeHashTable table;
    benchmark::DoNotOptimize(initHashTable(&table, NULL, NULL, NULL));

    size_t size = state.range(0);
    std::unique_ptr<StatData[]> mem = std::make_unique<StatData[]>(size);
    for (size_t i = 0; i < size; ++i) {
      mem[i].id = i;
      mem[i].cost = 50;
      mem[i].count = 5;
      mem[i].mode = 0;
      mem[i].primary = 1;
      insertToHashTable(&table, &mem[i]);
    }

    clearHashTable(&table);
  }
}

static void
TestInsertElementsWithNonUniquesId([[maybe_unused]] benchmark::State &state) {
  for ([[maybe_unused]] const auto &_ : state) {
    MergeHashTable table;
    benchmark::DoNotOptimize(initHashTable(&table, NULL, NULL, NULL));

    size_t size = state.range(0);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 500000);

    std::unique_ptr<StatData[]> mem = std::make_unique<StatData[]>(size);
    for (size_t i = 0; i < size; ++i) {
      mem[i].id = i % 50;
      mem[i].cost = distrib(gen);
      mem[i].count = distrib(gen);
      mem[i].mode = 0;
      mem[i].primary = 1;
      insertToHashTable(&table, &mem[i]);
    }

    clearHashTable(&table);
  }
}

static void
TestInsertElementsWithRandomId([[maybe_unused]] benchmark::State &state) {
  for ([[maybe_unused]] const auto &_ : state) {
    MergeHashTable table;
    benchmark::DoNotOptimize(initHashTable(&table, NULL, NULL, NULL));

    size_t size = state.range(0);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, size);

    std::unique_ptr<StatData[]> mem = std::make_unique<StatData[]>(size);
    for (size_t i = 0; i < size; ++i) {
      mem[i].id = distrib(gen);
      mem[i].cost = distrib(gen);
      mem[i].count = distrib(gen);
      mem[i].mode = 0;
      mem[i].primary = 1;
      insertToHashTable(&table, &mem[i]);
    }

    clearHashTable(&table);
  }
}

BENCHMARK(TestInsertElementsWithUniquesId)
    ->Arg(1000)
    ->Arg(0)
    ->Arg(50000)
    ->Arg(100000)
    ->Arg(200000)
    ->Arg(500000)
    ->Arg(1000000)
    ->Arg(2000000)
    ->Arg(5000000)
    ->Arg(10000000);

BENCHMARK(TestInsertElementsWithRandomId)
    ->Arg(1000)
    ->Arg(0)
    ->Arg(50000)
    ->Arg(100000)
    ->Arg(200000)
    ->Arg(500000)
    ->Arg(1000000)
    ->Arg(2000000)
    ->Arg(5000000)
    ->Arg(10000000);

BENCHMARK(TestInsertElementsWithNonUniquesId)
    ->Arg(1000)
    ->Arg(0)
    ->Arg(50000)
    ->Arg(100000)
    ->Arg(200000)
    ->Arg(500000)
    ->Arg(1000000)
    ->Arg(2000000)
    ->Arg(5000000)
    ->Arg(10000000);
