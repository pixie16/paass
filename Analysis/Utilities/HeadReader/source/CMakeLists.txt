# Install headReader executable.
add_executable(headReader headReader.cpp)
target_link_libraries(headReader PaassScanStatic)
install(TARGETS headReader DESTINATION bin)