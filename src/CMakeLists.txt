set(target bs)

add_library(${target} SHARED
	binarySerializer.c
    mergeHashTable.c
    tableView.c
)

include(compileOptions)
SetCompileOptionsLibC(${target})

target_include_directories(${target} PRIVATE 
						   ${CMAKE_SOURCE_DIR}/include
                           ${CMAKE_SOURCE_DIR}/include/internal
)

target_compile_definitions(${target} PRIVATE
                           BINARYSERIALIZER_BUTCHE_SIZE=${BINARYSERIALIZER_BUTCHE_SIZE}
)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    if (${BS_ENABLE_LOG})
        target_compile_definitions(${target} PRIVATE BS_ENABLE_LOG)
    endif()
endif()

if (${BS_ENABLE_MI_MALLOC})
    message(STATUS "Link bs library with mimalloc allocator")

    target_compile_definitions(${target} PRIVATE BS_ENABLE_MI_MALLOC)

    target_link_libraries(${target} PRIVATE mimalloc)
endif()