set(target serializeData)

add_executable(${target}
	main.c
)

include(compileOptions)
SetCompileOptionsC(${target})

target_include_directories(${target} PRIVATE 
						   ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(${target} PRIVATE bs)

if (${BS_ENABLE_MI_MALLOC})
    message(STATUS "Link serializeData with mimalloc allocator")

    target_compile_definitions(${target} PRIVATE BS_ENABLE_MI_MALLOC)

    target_link_libraries(${target} PRIVATE mimalloc)
endif()