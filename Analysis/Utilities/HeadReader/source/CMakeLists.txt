# @author C. R. Thornsberry
# Install headReader executable.
add_executable(headReader headReader.cpp)
target_link_libraries(headReader PaassScanStatic PugixmlStatic PaassResourceStatic)
install(TARGETS headReader DESTINATION bin)