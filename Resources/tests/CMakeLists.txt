# @author S.V. Paulauskas

add_executable(unittest-HelperFunctions unittest-HelperFunctions.cpp)
target_link_libraries(unittest-HelperFunctions UnitTest++)
install(TARGETS unittest-HelperFunctions DESTINATION bin/unittests)

add_executable(unittest-StringManipulationFunctions
        unittest-StringManipulationFunctions.cpp)
target_link_libraries(unittest-StringManipulationFunctions UnitTest++)
install(TARGETS unittest-StringManipulationFunctions DESTINATION bin/unittests)
