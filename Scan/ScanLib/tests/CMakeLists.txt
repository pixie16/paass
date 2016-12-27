add_executable(unittest-XiaListModeDataDecoder
        unittest-XiaListModeDataDecoder.cpp
        ../source/XiaData.cpp
        ../source/XiaListModeDataDecoder.cpp)
target_link_libraries(unittest-XiaListModeDataDecoder UnitTest++ ${LIBS})
install(TARGETS unittest-XiaListModeDataDecoder DESTINATION bin/unittests)