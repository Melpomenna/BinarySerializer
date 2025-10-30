
if (${BS_ENABLE_TEST})
    message(STATUS "Testing enabled")
    add_subdirectory(googletest)
endif()

if (${BS_ENABLE_BENCHMARKS})
    message(STATUS "Testing enabled")
    add_subdirectory(benchmark)
endif()

if (${BS_ENABLE_FUZZ_TEST})
    message(STATUS "Fuzz Testing enabled")
    add_subdirectory(fuzztest)
endif()