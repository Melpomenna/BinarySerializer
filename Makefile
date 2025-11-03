.PHONY: build tests benchmarks docs install

PRESET=release
PERF_RECORD_STATS=branch-misses,bus-cycles,cache-misses,cpu-cycles
FUZZ_TIME=120s
VALGRIND_OPTIONS=--leak-check=full --track-fds=yes --track-origins=yes --leak-check=full
MIMALLOC_SHOW_STATS=1
SERIALIZE_UTILE_ARGS=

all: configure

configure:
	cmake --preset=${PRESET}

build:
	cmake --build . --preset=${PRESET}

clean:
	rm -rf build

install:
	cmake --install ./build/$(PRESET)

uninstall:
	rm -rf install

clang-tidy:

cppcheck:

style:
	clang-format -i $(shell find src -type f -name "*.c")
	clang-format -i $(shell find include -type f -name "*.h")
	clang-format -i $(shell find cli -type f -name "*.c")
	clang-format -i $(shell find tests -type f -name "*.cpp")
	clang-format -i $(shell find benchmarks -type f -name "*.cpp")

check-style:
	clang-format -n $(shell find src -type f -name "*.c")
	clang-format -n $(shell find include -type f -name "*.h")
	clang-format -n $(shell find cli -type f -name "*.c")
	clang-format -n $(shell find tests -type f -name "*.cpp")
	clang-format -n $(shell find benchmarks -type f -name "*.cpp")

benchmarks:
	MIMALLOC_SHOW_STATS=$(MIMALLOC_SHOW_STATS)  ./build/$(PRESET)/WorkFolder/Benchmarks

unit:
	MIMALLOC_SHOW_STATS=$(MIMALLOC_SHOW_STATS) ctest --preset=${PRESET}

fuzzing-list:
	MIMALLOC_SHOW_STATS=$(MIMALLOC_SHOW_STATS) ./build/$(PRESET)/WorkFolder/Fuzzing --list_fuzz_tests

fuzzing:
	@FUZZ_TESTS=$$(./build/$(PRESET)/WorkFolder/Fuzzing --list_fuzz_tests  | grep -o "TestSuite*.*"); \
	for test in $$FUZZ_TESTS; do \
		MIMALLOC_SHOW_STATS=$(MIMALLOC_SHOW_STATS) ./build/$(PRESET)/WorkFolder/Fuzzing --fuzz=$$test --fuzz_for=$(FUZZ_TIME) || true; \
	done

valgrind:
	MIMALLOC_SHOW_STATS=$(MIMALLOC_SHOW_STATS) valgrind $(VALGRIND_OPTIONS) ./build/$(PRESET)/WorkFolder/Unit
	@FUZZ_TESTS=$$(./build/$(PRESET)/WorkFolder/Fuzzing --list_fuzz_tests  | grep -o "TestSuite*.*"); \
	for test in $$FUZZ_TESTS; do \
		MIMALLOC_SHOW_STATS=$(MIMALLOC_SHOW_STATS) valgrind $(VALGRIND_OPTIONS) --leak-check=full --track-fds=yes -v ./build/$(PRESET)/WorkFolder/Fuzzing --fuzz=$$test --fuzz_for=$(FUZZ_TIME) || true; \
	done

merge-profile:
	llvm-profdata merge -output=./build/release/src/default.profdata ./build/release/src/default_*.profraw

docs:
	doxygen ./docs/Doxygen

open-docs:
	xdg-open ./docs/out/html/index.html

gcov:

open-report:

test-utile:
	PATH="$(PATH):$(shell pwd)/build/$(PRESET)/WorkFolder" testBS

serializer-utile:
	./build/$(PRESET)/WorkFolder/serializeData $(SERIALIZE_UTILE_ARGS)

perf-record:
	perf record -e $(PERF_RECORD_STATS) -- ./build/$(PRESET)/WorkFolder/Benchmarks

perf-report:
	perf report