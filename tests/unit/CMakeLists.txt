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

if (${BS_ENABLE_MI_MALLOC})
    message(STATUS "Link bs library with mimalloc allocator")

    target_compile_definitions(${target} PRIVATE BS_ENABLE_MI_MALLOC)

    target_link_libraries(${target} PRIVATE mimalloc)
endif()

include(compileOptions)
SetCompileOptionsCXX(${target})

include(GoogleTest)
gtest_discover_tests(${target})

install(TARGETS ${target} DESTINATION ${CMAKE_SOURCE_DIR}/install/${CMAKE_BUILD_TYPE}/bin)