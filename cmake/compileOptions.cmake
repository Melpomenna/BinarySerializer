function(SetCompileOptionsLibC target)
    target_compile_options(${target} PRIVATE 
                        -v
                        -std=c11
                        -Wall
                        -Werror
                        -Wextra
                        -fpic
                        -fvisibility=hidden
                        -fstrict-aliasing
                        -m64)
    if (${BS_ENABLE_GCOV})
        message(STATUS "Gcov enabled")
        target_compile_options(${target} PRIVATE -fprofile-arcs -ftest-coverage)
    endif()
    if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
        message(STATUS "Debug type")
        target_compile_options(${target} PRIVATE -g  -ggdb -fsanitize=address,undefined -O0)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    else()
        if (${REL_WITH_DEBUG_INFO})
            target_compile_options(${target} PRIVATE -g)
        endif()
        message(STATUS "Release type")
        target_compile_options(${target} PRIVATE
                        -O3
                        -ffast-math
                        -mavx2
                        -march=corei7
                        -mtune=corei7
                        -funroll-loops
                        -flto)
        find_library(MEMPROF_LIB memprof)
        if(MEMPROF_LIB)
            message(STATUS "Memprofile lib founded")
            target_link_libraries(${target} PRIVATE ${MEMPROF_LIB})
            if (${BS_USE_PROFILE})
                target_compile_options(${target} PRIVATE  -fmemory-profile-use=${CMAKE_CURRENT_BINARY_DIR})
            else()
                target_compile_options(${target} PRIVATE -fmemory-profile=${CMAKE_CURRENT_BINARY_DIR})
            endif()
        else()
            message(STATUS "Memprofile not founded")
        endif()

        if (${BS_USE_PROFILE})
            message(STATUS "Using profile")
            target_compile_options(${target} PRIVATE -fprofile-use=${CMAKE_CURRENT_BINARY_DIR})
        else()
            message(STATUS "Generate profile")
            target_compile_options(${target} PRIVATE -fprofile-generate=${CMAKE_CURRENT_BINARY_DIR})
            target_link_options(${target} PRIVATE -fprofile-generate=${CMAKE_CURRENT_BINARY_DIR})
        endif()
        target_link_options(${target} PRIVATE -flto)
    endif()
endfunction()

function(SetCompileOptionsC target)
    target_compile_options(${target} PRIVATE 
                        -std=c11
                        -Wall
                        -Werror
                        -Wextra
                        -fstrict-aliasing
                        -m64)
    if (${BS_ENABLE_GCOV})
        message(STATUS "Gcov enabled")
        target_compile_options(${target} PRIVATE -fprofile-arcs -ftest-coverage)
    endif()
    if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
        message(STATUS "Debug type")
        target_compile_options(${target} PRIVATE -g  -ggdb -fsanitize=address,undefined -O0)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    else()
        if (${REL_WITH_DEBUG_INFO})
            target_compile_options(${target} PRIVATE -g)
        endif()
        message(STATUS "Release type")
        target_compile_options(${target} PRIVATE
                        -O3
                        -ffast-math
                        -mavx2
                        -march=corei7
                        -mtune=corei7
                        -funroll-loops
                        -flto)
        target_link_options(${target} PRIVATE -flto)
    endif()
endfunction()

function(SetCompileOptionsCXX target)
    target_compile_options(${target} PRIVATE -std=c++20)
    if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
        message(STATUS "Debug type")
        target_compile_options(${target} PRIVATE -g -fsanitize=address,undefined -O0)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    else()
        if (${REL_WITH_DEBUG_INFO})
            target_compile_options(${target} PRIVATE -g)
        endif()
        message(STATUS "Release type")
        target_compile_options(${target} PRIVATE -O2 -flto -mavx2 -march=corei7 -mtune=corei7)
        target_link_options(${target} PRIVATE -flto)
    endif()
endfunction()

function(GlobalCompileOptionsCXX)
    add_definitions(-D_XOPEN_SOURCE=700)
    if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
        message(STATUS "Debug type")
        add_compile_options(-g -fsanitize=address,undefined -O0)
        add_link_options(-fsanitize=address,undefined)
    else()
        if (${REL_WITH_DEBUG_INFO})
            add_compile_options(-g)
        endif()
        message(STATUS "Release type")
        add_compile_options(-O2 -flto -mavx2 -march=corei7 -mtune=corei7)
        add_link_options(-flto)
    endif()
endfunction()