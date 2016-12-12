add_executable(CTerminalTest CTerminalTest.cpp)
target_link_libraries(CTerminalTest PixieCoreStatic)
install (TARGETS CTerminalTest DESTINATION bin)
