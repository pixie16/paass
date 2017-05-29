add_executable(unittest-DetectorSummary unittest-DetectorSummary.cpp ../source/DetectorSummary.cpp ../source/ChanEvent.cpp
        ../source/DetectorLibrary.cpp ../source/MapNodeXmlParser.cpp ../source/TreeCorrelator.cpp
        ../source/Identifier.cpp ../source/TreeCorrelatorXmlParser.cpp ../source/WalkCorrector.cpp ../source/Calibrator.cpp
        ../source/PlaceBuilder.cpp ../source/Places.cpp ../source/RawEvent.cpp)
target_link_libraries(unittest-DetectorSummary UnitTest++ PaassScanStatic PaassCoreStatic PugixmlStatic
        PaassResourceStatic ${LIBS})
install(TARGETS unittest-DetectorSummary DESTINATION bin/unittests)

add_executable(unittest-Identifier unittest-Identifier.cpp ../source/Identifier.cpp)
target_link_libraries(unittest-Identifier UnitTest++ ${LIBS})
install(TARGETS unittest-Identifier DESTINATION bin/unittests)

add_executable(unittest-WalkCorrector unittest-WalkCorrector.cpp ../source/WalkCorrector.cpp ../source/Identifier.cpp)
target_link_libraries(unittest-WalkCorrector UnitTest++ ${LIBS})
install(TARGETS unittest-WalkCorrector DESTINATION bin/unittests)