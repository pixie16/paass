#author S. V. Paulauskas
add_subdirectory(source)

if (PAASS_BUILD_TESTS)
    add_subdirectory(tests)
endif (PAASS_BUILD_TESTS)
