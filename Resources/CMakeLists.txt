add_subdirectory(source)
if (PAASS_BUILD_TESTS OR PAASS_BUILD_UNITTESTS)
    add_subdirectory(tests)
endif (PAASS_BUILD_TESTS OR PAASS_BUILD_UNITTESTS)