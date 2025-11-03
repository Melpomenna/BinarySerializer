#include "BinarySerializer/binarySerializer.h"
#include "BinarySerializer/mergeHashTable.h"

#include <benchmark/benchmark.h>
#include <memory>
#include <random>
#include <vector>

namespace {
std::vector<long> ids;

}

static void DoSetup(const benchmark::State &state) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(0, state.range(0));
  for (size_t i = 0; i < state.range(0); ++i) {
    ids.push_back(distrib(gen));
  }
}

static void DoTeardown(const benchmark::State &state) { ids.clear(); }

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
      benchmark::DoNotOptimize(insertToHashTable(&table, &mem[i]));
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

    std::unique_ptr<StatData[]> mem = std::make_unique<StatData[]>(size);
    for (size_t i = 0; i < size; ++i) {
      mem[i].id = i % 50;
      mem[i].cost = 20;
      mem[i].count = 50.0;
      mem[i].mode = 0;
      mem[i].primary = 1;
      benchmark::DoNotOptimize(insertToHashTable(&table, &mem[i]));
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

    std::unique_ptr<StatData[]> mem = std::make_unique<StatData[]>(size);
    for (size_t i = 0; i < size; ++i) {
      mem[i].id = ids[i];
      mem[i].cost = 25;
      mem[i].count = 1.0;
      mem[i].mode = 0;
      mem[i].primary = 1;
      benchmark::DoNotOptimize(insertToHashTable(&table, &mem[i]));
    }

    clearHashTable(&table);
  }
}

BENCHMARK(TestInsertElementsWithUniquesId)
    ->Arg(0)
    ->Arg(1000)
    ->Arg(50000)
    ->Arg(100000)
    ->Arg(200000)
    ->Arg(500000)
    ->Arg(1000000)
    ->Arg(2000000)
    ->Iterations(10);

BENCHMARK(TestInsertElementsWithNonUniquesId)
    ->Arg(0)
    ->Arg(1000)
    ->Arg(50000)
    ->Arg(100000)
    ->Arg(200000)
    ->Arg(500000)
    ->Arg(1000000)
    ->Arg(2000000)
    ->Iterations(10);

BENCHMARK(TestInsertElementsWithRandomId)
    ->Arg(0)
    ->Arg(1000)
    ->Arg(50000)
    ->Arg(100000)
    ->Arg(200000)
    ->Arg(500000)
    ->Arg(750000)
    ->Arg(1000000)
    ->Arg(2000000)
    ->Iterations(10)
    ->Setup(DoSetup)
    ->Teardown(DoTeardown);