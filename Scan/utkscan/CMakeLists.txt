#CMake file for UTKScan.

option(USE_GSL "Use GSL for Pulse Fitting" ON)
option(UTKSCAN_GAMMA_GATES "Gamma-Gamma gates in GeProcessor" OFF)
option(UTKSCAN_ONLINE "Options for online scans" OFF)
option(UTKSCAN_TREE_DEBUG "Debugging info for TreeCorrelator" OFF)
option(UTKSCAN_VERBOSE "Make Scan More Verbose" OFF)

# newreadout is needed to account for a change to pixie16 readout
# structure change on 03/20/08. Is is REQUIRED!!
add_definitions(-D newreadout)

#utkscan will have Gamma-Gamma gating in the GeProcessor
if(UTKSCAN_GAMMA_GATES)
    add_definitions(-D GGATES)
endif(UTKSCAN_GAMMA_GATES)

#utkscan will be streamlined for online processing
if(UTKSCAN_ONLINE)
    add_definitions(-D ONLINE)
endif(UTKSCAN_ONLINE)

#utkscan will have debugging for the TreeCorrelator
if(UTKSCAN_TREE_DEBUG)
    add_definitions(-D TREE_DEBUG)
endif(UTKSCAN_TREE_DEBUG)

#utkscan will be more verbose in its output
if(UTKSCAN_VERBOSE)
    add_definitions(-D VERBOSE)
endif(UTKSCAN_VERBOSE)

#Check if GSL is installed
if(USE_GSL)
    find_package(GSL REQUIRED)
    add_definitions("-D usegsl")
endif(USE_GSL)

#------------------------------------------------------------------------------

#Add the local include directories to the build tree
include_directories(analyzers/include)
include_directories(core/include)
include_directories(experiment/include)
include_directories(processors/include)

#Add the Analyzers to the build
add_subdirectory(analyzers/source)
#Add the core classes to the build
add_subdirectory(core/source)
#Add the experiment specific processors to the build
add_subdirectory(experiment/source)
#Add the processors to the build
add_subdirectory(processors/source)

#------------------------------------------------------------------------------

#Create utkscan program
add_executable(utkscan
        $<TARGET_OBJECTS:AnalyzerObjects>
        $<TARGET_OBJECTS:CoreObjects>
        $<TARGET_OBJECTS:ExperimentObjects>
        $<TARGET_OBJECTS:ProcessorObjects>)

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

#------------------------------------------------------------------------------

#Install utkscan to the bin directory
install(TARGETS utkscan DESTINATION bin)
#Install configuration files to the share directory
install(DIRECTORY share/utkscan DESTINATION share/)

#------------------------------------------------------------------------------

#Build optional tests for the software
option(BUILD_UTKSCAN_TESTS "Build unit tests for utkscan" ON)
if(BUILD_UTKSCAN_TESTS)
    add_subdirectory(tests/source)
endif(BUILD_UTKSCAN_TESTS)
