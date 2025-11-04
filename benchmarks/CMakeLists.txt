set(target Benchmarks)

add_executable(${target} main.benchmarks.cpp)

if (${BS_ENABLE_MI_MALLOC})
    message(STATUS "Link bs library with mimalloc allocator")

    target_compile_definitions(${target} PRIVATE BS_ENABLE_MI_MALLOC)

    target_link_libraries(${target} PRIVATE mimalloc)
endif()


target_link_libraries(${target} PRIVATE bs benchmark::benchmark benchmark::benchmark_main)

target_include_directories(${target} PRIVATE
						   ${CMAKE_SOURCE_DIR}/include
)

include(compileOptions)
SetCompileOptionsCXX(${target})

install(TARGETS ${target} DESTINATION ${CMAKE_SOURCE_DIR}/install/${CMAKE_BUILD_TYPE}/bin)
