set(target Unit)

add_executable(${target}
	main.unit.cpp
)

target_link_libraries(${target} PRIVATE
	bs
	gtest
	gtest_main
)
target_include_directories(${target} PRIVATE
						   ${CMAKE_SOURCE_DIR}/include
)

include(compileOptions)
SetCompileOptionsCXX(${target})

include(GoogleTest)
gtest_discover_tests(${target})