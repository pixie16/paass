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
target_link_libraries(utkscan ${LIBS} ScanStatic)

#If we have GSL installed link
if(USE_GSL)
    target_link_libraries(utkscan ${GSL_LIBRARIES})
endif(USE_GSL)

#If ROOT is installed we'll link
if(USE_ROOT)
    target_link_libraries(utkscan ${ROOT_LIBRARIES})
endif(USE_ROOT)

#Install utkscan to the bin directory
install(TARGETS utkscan DESTINATION bin)

#Build optional unit tests for the software
option(BUILD_UTKSCAN_TESTS "Build unit tests for utkscan" ON)
if(BUILD_UTKSCAN_TESTS)
    add_subdirectory(tests)
endif(BUILD_UTKSCAN_TESTS)