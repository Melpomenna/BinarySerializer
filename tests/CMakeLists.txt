
if (${BS_ENABLE_TEST})
    add_subdirectory(unit)
endif()

if (${BS_ENABLE_FUZZ_TEST})
    add_subdirectory(fuzzing)
endif()