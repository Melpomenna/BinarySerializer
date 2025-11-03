set(target testBS)

add_executable(${target}
	main.c
	app.c
	testCases.c
	${CMAKE_SOURCE_DIR}/cli/utility/timeHelper.c
)

include(compileOptions)
SetCompileOptionsC(${target})

if (${CMAKE_BUILD_TYPE} MATCHES "debug")
    if (${BS_ENABLE_LOG})
        target_compile_definitions(${target} PRIVATE BS_ENABLE_LOG)
    endif()
endif()

target_include_directories(${target} PRIVATE 
						   ${CMAKE_SOURCE_DIR}/include
						   ${CMAKE_SOURCE_DIR}/cli
)
target_link_libraries(${target} PRIVATE bs)

install(TARGETS ${target} DESTINATION ${CMAKE_SOURCE_DIR}/install/${CMAKE_BUILD_TYPE}/bin)