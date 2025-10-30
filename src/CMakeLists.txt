set(target bs)

add_library(${target} SHARED
	binarySerializer.c
    hashTable.c
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