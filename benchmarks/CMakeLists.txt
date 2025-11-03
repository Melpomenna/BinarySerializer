set(target Benchmarks)

add_executable(${target} main.benchmarks.cpp)

target_link_libraries(${target} PRIVATE bs benchmark::benchmark benchmark::benchmark_main)

target_include_directories(${target} PRIVATE
						   ${CMAKE_SOURCE_DIR}/include
)

include(compileOptions)
SetCompileOptionsCXX(${target})

install(TARGETS ${target} DESTINATION ${CMAKE_SOURCE_DIR}/install/${CMAKE_BUILD_TYPE}/bin)