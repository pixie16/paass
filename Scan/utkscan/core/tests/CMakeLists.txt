add_executable(unittest-WalkCorrector unittest-WalkCorrector.cpp
        ../source/WalkCorrector.cpp ../source/Identifier.cpp)
target_link_libraries(unittest-WalkCorrector UnitTest++ ${LIBS})
install(TARGETS unittest-WalkCorrector DESTINATION bin/unittests)

add_executable(unittest-Identifier unittest-Identifier ../source/Identifier.cpp)
target_link_libraries(unittest-Identifier UnitTest++ ${LIBS})
install(TARGETS unittest-Identifier DESTINATION bin/unittests)