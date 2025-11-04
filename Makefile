.PHONY: build tests benchmarks docs install

PRESET=release
PERF_RECORD_STATS=branch-misses,bus-cycles,cache-misses,cpu-cycles
FUZZ_TIME=30s
VALGRIND_OPTIONS=--leak-check=full --track-fds=yes --track-origins=yes --leak-check=full
MIMALLOC_SHOW_STATS=0

SRC_DIRS := src include cli/app cli/test include/internal include/BinarySerializer
COMPILE_COMMANDS := ./build/$(PRESET)/compile_commands.json
SRC_EXTENSIONS := c h
SOURCES := $(foreach dir,$(SRC_DIRS),$(foreach ext,$(SRC_EXTENSIONS),$(wildcard $(dir)/*.$(ext) $(dir)/**/*.$(ext))))
CPPCHECK_FLAGS := --enable=all \
                  --suppress=missingIncludeSystem \
                  --suppress=unusedFunction \
                  --inline-suppr \
                  --error-exitcode=1 \
				  --suppressions-list=cppcheck-suppressions.txt \
                  --std=c11 \
                  --language=c \
                  -I./include \
				  -I./include/internal \
				  -I./include/BinarySerializer \
                  -I./src \
				  -I./cli/ \
				  -I./cli/app \
				  -I./cli/test \
				  -I./cli/utility \
                  --quiet \
                  --verbose

all: configure

configure:
	@cmake --preset=${PRESET}

build:
	@cmake --build . --preset=${PRESET}

clean: uninstall
	@rm -rf build
	@rm -rf coverage

install:
	@cmake --install ./build/$(PRESET)

uninstall:
	@rm -rf install

clang-tidy:
	@echo "=== Running clang-tidy ==="
	@if [ ! -f "$(COMPILE_COMMANDS)" ]; then \
		echo "Error: compile_commands.json not found at $(COMPILE_COMMANDS)"; \
		echo "Run 'cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON' first"; \
		exit 1; \
	fi
	@if [ -z "$(SOURCES)" ]; then \
		echo "No source files found in: $(SRC_DIRS)"; \
		exit 1; \
	fi
	@echo "Checking $(words $(SOURCES)) files..."
	@clang-tidy -p ./build/$(PRESET) $(SOURCES)
	@echo "✓ Clang-tidy passed"

cppcheck:
	@echo "=== Running cppcheck ==="
	@if [ -z "$(SOURCES)" ]; then \
		echo "No source files found in: $(SRC_DIRS)"; \
		exit 1; \
	fi
	@echo "Checking $(words $(SOURCES)) files..."
	@cppcheck $(CPPCHECK_FLAGS) $(SOURCES)
	@echo "✓ Cppcheck passed"

style:
	@clang-format -i $(shell find src -type f -name "*.c")
	@clang-format -i $(shell find include -type f -name "*.h")
	@clang-format -i $(shell find cli -type f -name "*.c")
	@clang-format -i $(shell find tests -type f -name "*.cpp")
	@clang-format -i $(shell find benchmarks -type f -name "*.cpp")

check-style:
	@clang-format -n $(shell find src -type f -name "*.c")
	@clang-format -n $(shell find include -type f -name "*.h")
	@clang-format -n $(shell find cli -type f -name "*.c")
	@clang-format -n $(shell find tests -type f -name "*.cpp")
	@clang-format -n $(shell find benchmarks -type f -name "*.cpp")

benchmarks:
	@MIMALLOC_SHOW_STATS=$(MIMALLOC_SHOW_STATS)  ./build/$(PRESET)/WorkFolder/Benchmarks

unit:
	@MIMALLOC_SHOW_STATS=$(MIMALLOC_SHOW_STATS) ctest --preset=${PRESET}

fuzzing-list:
	@MIMALLOC_SHOW_STATS=$(MIMALLOC_SHOW_STATS) ./build/$(PRESET)/WorkFolder/Fuzzing --list_fuzz_tests

fuzzing:
	@FUZZ_TESTS=$$(./build/$(PRESET)/WorkFolder/Fuzzing --list_fuzz_tests  | grep -o "TestSuite*.*"); \
	for test in $$FUZZ_TESTS; do \
		MIMALLOC_SHOW_STATS=$(MIMALLOC_SHOW_STATS) ./build/$(PRESET)/WorkFolder/Fuzzing --fuzz=$$test --fuzz_for=$(FUZZ_TIME) || true; \
	done

valgrind:
	MIMALLOC_SHOW_STATS=$(MIMALLOC_SHOW_STATS) valgrind $(VALGRIND_OPTIONS) ./build/$(PRESET)/WorkFolder/Unit
	@FUZZ_TESTS=$$(./build/$(PRESET)/WorkFolder/Fuzzing --list_fuzz_tests  | grep -o "TestSuite*.*"); \
	for test in $$FUZZ_TESTS; do \
		MIMALLOC_SHOW_STATS=$(MIMALLOC_SHOW_STATS) valgrind $(VALGRIND_OPTIONS) ./build/$(PRESET)/WorkFolder/Fuzzing --fuzz=$$test --fuzz_for=$(FUZZ_TIME) || true; \
	done

valgrind-tu:
	@MIMALLOC_SHOW_STATS=$(MIMALLOC_SHOW_STATS) PATH="$(PATH):$(shell pwd)/build/$(PRESET)/WorkFolder" valgrind $(VALGRIND_OPTIONS) testBS

merge-profile:
	@llvm-profdata merge -output=./build/release/src/default.profdata ./build/release/src/default_*.profraw

docs:
	@doxygen ./docs/Doxygen

open-docs:
	@xdg-open ./docs/out/html/index.html

gcov: unit fuzzing benchmarks test-utile
	@mkdir -p ./coverage/$(PRESET)

	@find . -name "*.profraw" -type f > ./coverage/$(PRESET)/profraw_list.txt

	@llvm-profdata merge -sparse $$(cat ./coverage/$(PRESET)/profraw_list.txt) -o ./coverage/$(PRESET)/default.profdata

	@llvm-cov show ./build/$(PRESET)/WorkFolder/Unit \
		-instr-profile=./coverage/$(PRESET)/default.profdata \
		-format=html \
		-output-dir=./coverage/$(PRESET) \
		-object ./build/$(PRESET)/WorkFolder/libbs.so \
		$$(find ./build/$(PRESET)/tests -name "*.test" -o -name "Test" -type f | sed 's/^/-object /') \
		-show-line-counts-or-regions \
		-show-branches=count

open-report:
	@xdg-open ./coverage/$(PRESET)/index.html

test-utile:
	@MIMALLOC_SHOW_STATS=$(MIMALLOC_SHOW_STATS) PATH="$(PATH):$(shell pwd)/build/$(PRESET)/WorkFolder" testBS

# Need sudo!
perf-record:
	@perf record -e $(PERF_RECORD_STATS) -- ./build/$(PRESET)/WorkFolder/Benchmarks

perf-report:
	@perf report