# @authors C. R. Thornsberry, K. Smith
include_directories(include)
add_subdirectory(source)
install(FILES share/mca_input.txt DESTINATION share)
