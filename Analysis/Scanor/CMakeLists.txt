# @author C. R. Thornsberry
#Install include directories to support the shared library.
if (PAASS_BUILD_SHARED_LIBS)
    install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})
endif (PAASS_BUILD_SHARED_LIBS)

add_subdirectory(source)
