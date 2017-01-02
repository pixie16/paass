#Set the scan sources that we will make a lib out of
set(ScanSources ScanInterface.cpp Unpacker.cpp XiaData.cpp ChannelData.cpp
        ChannelEvent.cpp XiaListModeDataMask.cpp XiaListModeDataDecoder.cpp
        XiaListModeDataEncoder.cpp RootScanner.cpp)

#Add the sources to the library
add_library(ScanObjects OBJECT ${ScanSources})

if(BUILD_SHARED_LIBS)
    message(STATUS "Building Scan Shared Objects")
    add_library(Scan SHARED $<TARGET_OBJECTS:ScanObjects>)
    target_link_libraries(Scan PixieCoreStatic ${CMAKE_THREAD_LIBS_INIT})
    if (${CURSES_FOUND})
        target_link_libraries(Scan ${CURSES_LIBRARIES})
    endif()
    install(TARGETS Scan DESTINATION lib)
endif(BUILD_SHARED_LIBS)

#Create PixieScan static library and add ncurses if we have it
add_library(ScanStatic STATIC $<TARGET_OBJECTS:ScanObjects>)
target_link_libraries(ScanStatic PixieCoreStatic ${CMAKE_THREAD_LIBS_INIT})
if (${CURSES_FOUND})
    target_link_libraries(ScanStatic ${CURSES_LIBRARIES})
endif()
