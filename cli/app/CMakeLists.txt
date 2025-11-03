set(target serializeData)

add_executable(${target}
	main.c
)

include(compileOptions)
SetCompileOptionsC(${target})

target_include_directories(${target} PRIVATE 
						   ${CMAKE_SOURCE_DIR}/include
						   ${CMAKE_SOURCE_DIR}/cli
)
target_link_libraries(${target} PRIVATE bs)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    if (${BS_ENABLE_LOG})
        target_compile_definitions(${target} PRIVATE BS_ENABLE_LOG)
    endif()
endif()

if (${BS_ENABLE_MI_MALLOC})
    message(STATUS "Link serializeData with mimalloc allocator")

    target_compile_definitions(${target} PRIVATE BS_ENABLE_MI_MALLOC)

    target_link_libraries(${target} PRIVATE mimalloc)
endif()

install(TARGETS ${target} DESTINATION ${CMAKE_SOURCE_DIR}/install/${CMAKE_BUILD_TYPE}/bin)