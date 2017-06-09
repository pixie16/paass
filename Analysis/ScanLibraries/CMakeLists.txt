# @author S. V. Paulauskas, K. Smith
#Install include directories to support the shared library.
if (PAASS_BUILD_SHARED_LIBS)
    install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})
endif (PAASS_BUILD_SHARED_LIBS)

if (PAASS_BUILD_TESTS)
    add_subdirectory(tests)
endif (PAASS_BUILD_TESTS)

add_subdirectory(source)
