set(target Fuzzing)

add_executable(${target}
	main.fuzzing.cpp
)

target_compile_definitions(${target} PRIVATE FUZZTEST_FUZZING_MODE=on)
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
	target_compile_definitions(${target} PRIVATE ADDRESS_SANITIZER)
endif()

target_link_libraries(${target} PRIVATE
	bs
)

target_include_directories(${target} PRIVATE
						   ${CMAKE_SOURCE_DIR}/include
                           ${CMAKE_SOURCE_DIR}/include/internal
)


include(GoogleTest)
link_fuzztest(${target})
gtest_discover_tests(${target})