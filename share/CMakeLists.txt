#Configure the module file with the install prefix.
configure_file("modulefiles/pixieSuite" pixieSuite @ONLY)

#Install the configured file which is created in CMAKE_CURRENT_BINARY_DIR.
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pixieSuite DESTINATION share/modulefiles/)

