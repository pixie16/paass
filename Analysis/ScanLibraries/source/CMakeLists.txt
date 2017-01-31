#Set the scan sources that we will make a lib out of
set(PaassScanSources ScanInterface.cpp Unpacker.cpp XiaData.cpp ChannelData.cpp
        ChannelEvent.cpp XiaListModeDataMask.cpp XiaListModeDataDecoder.cpp
        XiaListModeDataEncoder.cpp)

if(USE_ROOT)
    list(APPEND PaassScanSources RootScanner.cpp)
endif(USE_ROOT)

#Add the sources to the library
add_library(PaassScanObjects OBJECT ${PaassScanSources})

if(BUILD_SHARED_LIBS)
    message(STATUS "Building Scan Shared Objects")
    add_library(PaassScan SHARED $<TARGET_OBJECTS:PaassScanObjects>)
    target_link_libraries(PaassScan PaassCoreStatic ${CMAKE_THREAD_LIBS_INIT})
    if (${CURSES_FOUND})
        target_link_libraries(PaassScan ${CURSES_LIBRARIES})
    endif()
    install(TARGETS PaassScan DESTINATION lib)
endif(BUILD_SHARED_LIBS)

#Create PixieScan static library and add ncurses if we have it
add_library(PaassScanStatic STATIC $<TARGET_OBJECTS:PaassScanObjects>)
target_link_libraries(PaassScanStatic PaassCoreStatic ${CMAKE_THREAD_LIBS_INIT})
if (${CURSES_FOUND})
    target_link_libraries(PaassScanStatic ${CURSES_LIBRARIES})
endif()
