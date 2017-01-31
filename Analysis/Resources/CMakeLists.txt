add_subdirectory(source)

if(BUILD_TESTS OR BUILD_UNITTESTS)
    add_subdirectory(tests)
endif(BUILD_TESTS OR BUILD_UNITTESTS)