add_executable(unittest-HelperFunctions unittest-HelperFunctions.cpp)
target_link_libraries(unittest-HelperFunctions UnitTest++)
install(TARGETS unittest-HelperFunctions DESTINATION bin/unittests)