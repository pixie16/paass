add_executable(CTerminalTest CTerminalTest.cpp)
target_link_libraries(CTerminalTest PixieCore)
install (TARGETS CTerminalTest DESTINATION bin)
