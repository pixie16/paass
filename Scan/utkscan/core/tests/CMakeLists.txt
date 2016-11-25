add_executable(unittest-WalkCorrector unittest-WalkCorrector.cpp
        ../source/WalkCorrector.cpp)
target_link_libraries(unittest-WalkCorrector UnitTest++ ${LIBS})
install(TARGETS unittest-WalkCorrector DESTINATION bin/unittests)