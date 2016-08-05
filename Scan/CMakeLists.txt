install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})

#Set the PixieCore sources that we will make a lib out of
set(PixieCore_SOURCES source/core/ScanInterface.cpp source/core/Unpacker.cpp source/core/XiaData.cpp)

#Add the sources to the library
add_library(PixieScanObjects OBJECT ${PixieCore_SOURCES})

#Create PixieScan.so
add_library(PixieScan SHARED $<TARGET_OBJECTS:PixieScanObjects>)
target_link_libraries(PixieScan PixieCoreStatic ${CMAKE_THREAD_LIBS_INIT})

#Create PixieScan static library
add_library(PixieScanStatic STATIC $<TARGET_OBJECTS:PixieScanObjects>)
target_link_libraries(PixieScanStatic PixieCoreStatic ${CMAKE_THREAD_LIBS_INIT})

#If we have ncurses then we'll add those to the library as well
if (${CURSES_FOUND})
	target_link_libraries(PixieScan ${CURSES_LIBRARIES})
	target_link_libraries(PixieScanStatic ${CURSES_LIBRARIES})
endif()

#Install the PixieScan library to the lib folder
install(TARGETS PixieScan DESTINATION lib)

#Build the scan utilities
add_subdirectory(source/util)

#Build the utk scan library
if (USE_SCAN)
	add_subdirectory(source)
endif()
