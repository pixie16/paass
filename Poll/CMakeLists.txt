
include_directories(include)
add_subdirectory(source)

install(PROGRAMS monitor.bash DESTINATION ${CMAKE_INSTALL_PREFIX})
