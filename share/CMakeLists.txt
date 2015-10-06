configure_file("modulefiles/pixieSuite" pixieSuite @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pixieSuite DESTINATION share/modulefiles/)

