cmake_minimum_required(VERSION 3.25.1...4.0)

enable_testing()

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)

project(BinarySerializer VERSION 1.0.0 LANGUAGES C CXX)


set(CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}:${CMAKE_MODULE_PATH}")
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(compileOptions)
GlobalCompileOptionsCXX()

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(cli)

add_subdirectory(tests)

if (${BS_ENABLE_BENCHMARKS})
	add_subdirectory(benchmarks)
endif()