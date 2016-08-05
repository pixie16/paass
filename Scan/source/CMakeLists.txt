#Add Subdirectories to the build
add_subdirectory(core)
add_subdirectory(analyzers)
add_subdirectory(processors)
add_subdirectory(experiment)

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
	#Add ROOT to utkscan
	target_link_libraries(utkscan ${ROOT_LIBRARIES})
endif()
#Install utkscan to the bin directory
install(TARGETS utkscan DESTINATION bin)
