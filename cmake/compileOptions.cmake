function(SetCompileOptionsLibC target)
    target_compile_options(${target} PRIVATE 
                        -std=c11
                        -Wall
                        -Werror
                        -Wextra
                        -fpic
                        -fvisibility=hidden
                        -fstrict-aliasing
                        -m64)
    if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
        message(STATUS "Debug type")
        target_compile_options(${target} PRIVATE -g -fsanitize=address -O0)
        target_link_options(${target} PRIVATE -fsanitize=address)
    else()
        message(STATUS "Release type")
        target_compile_options(${target} PRIVATE
                        -O3
                        -ffast-math
                        -mavx2
                        -march=corei7
                        -mtune=corei7
                        -funroll-loops
                        -flto)
        if (${BS_USE_PROFILE})
            target_compile_options(${target} PRIVATE -fprofile-use=${CMAKE_CURRENT_BINARY_DIR})
        else()
            target_compile_options(${target} PRIVATE -fprofile-generate=${CMAKE_CURRENT_BINARY_DIR})
            target_link_options(${target} PRIVATE -fprofile-generate=${CMAKE_CURRENT_BINARY_DIR})
        endif()
        target_link_options(${target} PRIVATE -flto)
    endif()
endfunction()

function(SetCompileOptionsCXX target)
    target_compile_options(${target} PRIVATE -std=c++20)
    if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
        message(STATUS "Debug type")
        target_compile_options(${target} PRIVATE -g -fsanitize=address -O0)
        target_link_options(${target} PRIVATE -fsanitize=address)
    else()
        message(STATUS "Release type")
        target_compile_options(${target} PRIVATE -O2 -flto -mavx2 -march=corei7 -mtune=corei7)
        target_link_options(${target} PRIVATE -flto)
    endif()
endfunction()

function(GlobalCompileOptionsCXX)
    add_compile_options(-std=c++20)
    if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
        message(STATUS "Debug type")
        add_compile_options(-g -fsanitize=address -O0)
        add_link_options(-fsanitize=address)
    else()
        message(STATUS "Release type")
        add_compile_options(-O2 -mavx2 -march=corei7 -mtune=corei7)
        add_link_options(-flto)
    endif()
endfunction()