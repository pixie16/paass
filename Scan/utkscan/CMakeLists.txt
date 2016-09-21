#CMake file for UTKScan.

#------------------------------------------------------------------------------
option(USE_GSL "Use GSL for Pulse Fitting" ON)

option(UTKSCAN_GAMMA_GATES "Gamma-Gamma gates in GeProcessor" OFF)
option(UTKSCAN_ONLINE "Options for online scans" OFF)
option(UTKSCAN_TREE_DEBUG "Debugging info for TreeCorrelator" OFF)
option(UTKSCAN_VERBOSE "Make Scan More Verbose" OFF)

#------------------------------------------------------------------------------

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

#------------------------------------------------------------------------------

#Check if GSL is installed
if(USE_GSL)
    find_package(GSL REQUIRED)
    add_definitions("-D usegsl")
endif(USE_GSL)

#Install utkscan to the bin directory
install(DIRECTORY share/utkscan DESTINATION share/)

include_directories(include ../ScanLib/include)
add_subdirectory(source)
