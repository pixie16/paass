add_executable(unittest-XiaListModeDataDecoder
        unittest-XiaListModeDataDecoder.cpp
        ../source/XiaData.cpp
        ../source/XiaListModeDataDecoder.cpp)
target_link_libraries(unittest-XiaListModeDataDecoder UnitTest++ ${LIBS})
install(TARGETS unittest-XiaListModeDataDecoder DESTINATION bin/unittests)

add_executable(unittest-XiaListModeDataMask
        unittest-XiaListModeDataMask.cpp
        ../source/XiaData.cpp
        ../source/XiaListModeDataMask.cpp)
target_link_libraries(unittest-XiaListModeDataMask UnitTest++ ${LIBS})
install(TARGETS unittest-XiaListModeDataMask DESTINATION bin/unittests)

add_executable(unittest-XiaData unittest-XiaData.cpp ../source/XiaData.cpp)
target_link_libraries(unittest-XiaData UnitTest++ ${LIBS})
install(TARGETS unittest-XiaData DESTINATION bin/unittests)