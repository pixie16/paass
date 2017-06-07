# @authors S.V. Paulauskas and K. Smith

add_subdirectory(source)
if (PAASS_BUILD_TESTS)
    add_subdirectory(tests)
endif (PAASS_BUILD_TESTS)
