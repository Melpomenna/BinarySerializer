#include "BinarySerializer/binarySerializer.h"
#include "BinarySerializer/mergeHashTable.h"

#include <benchmark/benchmark.h>
#include <memory>
#include <random>
#include <vector>

#if defined(BS_ENABLE_MI_MALLOC)
#include <mimalloc-override.h>
#else
#include <stdlib.h>
#endif

namespace {
std::vector<long> ids;
std::unique_ptr<StatData[]> benchData;
std::unique_ptr<StatData[]> firstJoin;
std::unique_ptr<StatData[]> secondJoin;

} // namespace

static void DoSetup(const benchmark::State &state) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(0, state.range(0));
  for (size_t i = 0; i < state.range(0); ++i) {
    ids.push_back(distrib(gen));
  }
}

static void DoTeardown(const benchmark::State &state) { ids.clear(); }

static void DoSetupStore(const benchmark::State &state) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(0, state.range(0));
  benchData = std::make_unique<StatData[]>(state.range(0));
  for (size_t i = 0; i < state.range(0); ++i) {
    benchData[i].id = distrib(gen);
    benchData[i].cost = 25;
    benchData[i].count = 1.0;
    benchData[i].mode = 0;
    benchData[i].primary = 1;
  }
}

static void DoTeardownStore(const benchmark::State &state) { benchData = {}; }

static void DoSetupJoin(const benchmark::State &state) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(0, state.range(0));
  firstJoin = std::make_unique<StatData[]>(state.range(0));
  secondJoin = std::make_unique<StatData[]>(state.range(0));
  for (size_t i = 0; i < state.range(0); ++i) {
    firstJoin[i].id = distrib(gen);
    firstJoin[i].cost = 25;
    firstJoin[i].count = 1.0;
    firstJoin[i].mode = 0;
    firstJoin[i].primary = 1;
    secondJoin[i].id = distrib(gen);
    secondJoin[i].cost = 25;
    secondJoin[i].count = 1.0;
    secondJoin[i].mode = 0;
    secondJoin[i].primary = 1;
  }
}

static void DoTeardownJoin(const benchmark::State &state) {
  firstJoin = {};
  secondJoin = {};
}

static void
TestInsertElementsWithUniquesId([[maybe_unused]] benchmark::State &state) {
  for ([[maybe_unused]] const auto &_ : state) {
    MergeHashTable table;
    benchmark::DoNotOptimize(InitHashTable(&table, NULL, NULL, NULL));

    size_t size = state.range(0);
    std::unique_ptr<StatData[]> mem = std::make_unique<StatData[]>(size);
    for (size_t i = 0; i < size; ++i) {
      mem[i].id = i;
      mem[i].cost = 50;
      mem[i].count = 5;
      mem[i].mode = 0;
      mem[i].primary = 1;
      benchmark::DoNotOptimize(InsertToHashTable(&table, &mem[i]));
    }

    ClearHashTable(&table);
  }
}

static void
TestInsertElementsWithNonUniquesId([[maybe_unused]] benchmark::State &state) {
  for ([[maybe_unused]] const auto &_ : state) {
    MergeHashTable table;
    benchmark::DoNotOptimize(InitHashTable(&table, NULL, NULL, NULL));

    size_t size = state.range(0);

    std::unique_ptr<StatData[]> mem = std::make_unique<StatData[]>(size);
    for (size_t i = 0; i < size; ++i) {
      mem[i].id = i % 50;
      mem[i].cost = 20;
      mem[i].count = 50.0;
      mem[i].mode = 0;
      mem[i].primary = 1;
      benchmark::DoNotOptimize(InsertToHashTable(&table, &mem[i]));
    }

    ClearHashTable(&table);
  }
}

static void
TestInsertElementsWithRandomId([[maybe_unused]] benchmark::State &state) {
  for ([[maybe_unused]] const auto &_ : state) {
    MergeHashTable table;
    benchmark::DoNotOptimize(InitHashTable(&table, NULL, NULL, NULL));

    size_t size = state.range(0);

    std::unique_ptr<StatData[]> mem = std::make_unique<StatData[]>(size);
    for (size_t i = 0; i < size; ++i) {
      mem[i].id = ids[i];
      mem[i].cost = 25;
      mem[i].count = 1.0;
      mem[i].mode = 0;
      mem[i].primary = 1;
      benchmark::DoNotOptimize(InsertToHashTable(&table, &mem[i]));
    }

    ClearHashTable(&table);
  }
}

static void
TestStoreAndLoadDataWithRandomIDs([[maybe_unused]] benchmark::State &state) {
  for ([[maybe_unused]] const auto &_ : state) {

    FILE *fd = fopen("out.dat", "ab+");
    fclose(fd);
    benchmark::DoNotOptimize(
        StoreDump("out.dat", benchData.get(), state.range(0)));
    StatData *data = NULL;
    size_t inSize = 0;
    benchmark::DoNotOptimize(LoadDump("out.dat", &data, &inSize));

    remove("out.dat");
  }
}

static void TestJoinData([[maybe_unused]] benchmark::State &state) {
  for ([[maybe_unused]] const auto &_ : state) {

    StatData *dt = NULL;
    size_t size = 0;
    benchmark::DoNotOptimize(JoinDump(firstJoin.get(), state.range(0),
                                      secondJoin.get(), state.range(0), &dt,
                                      &size));
    free(dt);
  }
}

static int SortStatDataFunc(const void *__restrict lhs,
                            const void *__restrict rhs) {
  const StatData *sdlhs = reinterpret_cast<const StatData *>(lhs);
  const StatData *sdrhs = reinterpret_cast<const StatData *>(rhs);
  return sdlhs->cost > sdrhs->cost;
}

static void TestJoinAndSortData([[maybe_unused]] benchmark::State &state) {
  for ([[maybe_unused]] const auto &_ : state) {

    StatData *dt = NULL;
    size_t size = 0;
    benchmark::DoNotOptimize(JoinDump(firstJoin.get(), state.range(0),
                                      secondJoin.get(), state.range(0), &dt,
                                      &size));
    benchmark::DoNotOptimize(SortDump(dt, size, &SortStatDataFunc));
    free(dt);
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

BENCHMARK(TestStoreAndLoadDataWithRandomIDs)
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
    ->Setup(DoSetupStore)
    ->Teardown(DoTeardownStore);

BENCHMARK(TestJoinData)
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
    ->Setup(DoSetupJoin)
    ->Teardown(DoTeardownJoin);

BENCHMARK(TestJoinAndSortData)
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
    ->Setup(DoSetupJoin)
    ->Teardown(DoTeardownJoin);