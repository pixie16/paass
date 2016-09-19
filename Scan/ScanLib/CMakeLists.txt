#Install include directories to support the shared library.
if(BUILD_SHARED_LIBS)
  install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})
endif(BUILD_SHARED_LIBS)

add_subdirectory(source)
