#@authors K. Smith
add_executable(CTerminalTest CTerminalTest.cpp)
target_link_libraries(CTerminalTest PaassCoreStatic)
install(TARGETS CTerminalTest DESTINATION bin)
