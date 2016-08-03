#Add Subdirectories to the build
add_subdirectory(core)
add_subdirectory(analyzers)
add_subdirectory(processors)
add_subdirectory(experiment)

#Set the PixieCore sources that we will make a lib out of
set(PixieCore_SOURCES core/ScanInterface.cpp core/Unpacker.cpp core/XiaData.cpp)

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

#Create utkscan program
add_executable(utkscan
		$<TARGET_OBJECTS:CoreObjects>
		$<TARGET_OBJECTS:AnalyzerObjects>
		$<TARGET_OBJECTS:ProcessorObjects>
		$<TARGET_OBJECTS:ExperimentObjects>)

#Add libraries to be linked with utkscan
target_link_libraries(utkscan ${LIBS} PixieScanStatic)
#If we have GSL installed add that to the linking too
if(USE_GSL)
	target_link_libraries(utkscan ${GSL_LIBRARIES})
endif(USE_GSL)

#Build the scope program and the filterer program
if(USE_ROOT)
	#Compile and install the scope program
	set(SCOPE_SOURCES util/scope.cpp)
	add_executable(scope ${SCOPE_SOURCES})
	target_link_libraries(scope PixieScanStatic ${ROOT_LIBRARIES})
#
#	add_executable(filterer filterer.cpp)
#	target_link_libraries(filterer PixieScanStatic ${ROOT_LIBRARIES})
#	
	install(TARGETS scope DESTINATION bin)
	#Add ROOT to utkscan
	target_link_libraries(utkscan ${ROOT_LIBRARIES})
endif()
#Install utkscan to the bin directory
install(TARGETS utkscan DESTINATION bin)

# Build the scanner skeleton source.
add_executable(skeleton util/Skeleton.cpp)
target_link_libraries(skeleton PixieScanStatic)
install (TARGETS skeleton DESTINATION bin)

