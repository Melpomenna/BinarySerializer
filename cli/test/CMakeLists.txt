set(target testBS)

add_executable(${target}
	main.c
)

include(compileOptions)
SetCompileOptionsC(${target})

target_include_directories(${target} PRIVATE 
						   ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(${target} PRIVATE bs)