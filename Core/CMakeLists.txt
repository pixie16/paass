install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})

add_subdirectory(source)

if (BUILD_TESTS)
	add_subdirectory(tests)
endif(BUILD_TESTS)
