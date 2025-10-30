include(FetchContent)

FetchContent_Declare(
        fuzztest
        GIT_REPOSITORY https://github.com/google/fuzztest.git
        GIT_TAG 2025-08-05
        GIT_SHALLOW TRUE
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}
)

FetchContent_MakeAvailable(fuzztest)