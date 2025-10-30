.PHONY: build tests benchmarks

PRESET=debug

all: configure

configure:
	cmake --preset=${PRESET}

build:
	cmake --build . --preset=${PRESET}

clean:
	rm -rf build

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
	./build/$(PRESET)/WorkFolder/Benchmarks

unit:
	ctest --preset=${PRESET}

fuzzing:
	./build/$(PRESET)/WorkFolder/Fuzzing

valgrind:
	valgrind ./build/$(PRESET)/WorkFolder/Unit

docs:

gcov: