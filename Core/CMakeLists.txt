#@authors K. Smith
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})

add_subdirectory(source)

if (PAASS_BUILD_TESTS)
    add_subdirectory(tests)
endif (PAASS_BUILD_TESTS)
