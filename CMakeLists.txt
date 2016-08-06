cmake_minimum_required(VERSION 2.8.8)

project(PixieSuite)

#Compile with c++11 or c++0x depending on the compiler version
if (CMAKE_COMPILER_IS_GNUCXX)
	if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "4.3" AND
            ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "4.7")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    elseif(${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "4.6")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif(${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "4.3" AND
            ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "4.7")

    #Make compiler messages nice with colored tags.
	if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "4.9")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=auto")
	endif()

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -fPIC")
endif()

#if user does not specify prefix we assign it to the exec directory
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   message(STATUS "Install Prefix not specified.")
   file(MAKE_DIRECTORY exec)
   get_filename_component(INSTALL_DIR ${CMAKE_SOURCE_DIR}/exec REALPATH)
   set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH "Install Prefix" FORCE)
endif()
message(STATUS "Installing to ${CMAKE_INSTALL_PREFIX}")

#Define the default build type to be Release
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Build type, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
   message(STATUS "Build type not defined, using default: ${CMAKE_BUILD_TYPE}")
endif(NOT CMAKE_BUILD_TYPE)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#Add additional CXX flags if we use the Debug option
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
endif(CMAKE_BUILD_TYPE MATCHES "Debug")

#------------------------------------------------------------------------------

#Install options
option(BUILD_SHARED_LIBS "Install only scan libraries" ON)
option(BUILD_SKELETON "Build and install the skeleton scan" ON)
option(BUILD_SCOPE "Build and install the scope program" ON)
option(BUILD_UTKSCAN "Install utkscan" ON)
option(USE_DAMM "Use DAMM for MCA" ON)
option(USE_GAMMA_GATES "Gamma-Gamma gates in GeProcessor" OFF)
option(USE_GSL "Use GSL for Pulse Fitting" ON)
option(USE_NCURSES "Use ncurses for terminal" ON)
option(USE_ONLINE "Options for online scans" OFF)
option(USE_POLL "Install Poll" ON)
option(USE_ROOT "Use ROOT" ON)
option(USE_SETUP "Include the older setup programs in installation" OFF)
option(USE_TREE_DEBUG "Debugging info for TreeCorrelator" OFF)
option(USE_VERBOSE "Make Scan More Verbose" OFF)

#------------------------------------------------------------------------------

#Definitions without options

# newreadout is needed to account for a change to pixie16 readout
# structure change on 03/20/08. Is is REQUIRED!!
add_definitions(-D newreadout)

#Adds the install prefix for referencing in the source code
add_definitions(-D INSTALL_PREFIX="\\"${CMAKE_INSTALL_PREFIX}\\"")

#Definitions with options

#The MCA will write DAMM histograms as output
if(USE_DAMM)
    add_definitions("-D USE_DAMM")
endif()

#utkscan will have Gamma-Gamma gating in the GeProcessor
if(USE_GAMMA_GATES)
    add_definitions(-D GGATES)
endif(USE_GAMMA_GATES)

#utkscan will be streamlined for online processing
if(USE_ONLINE)
    add_definitions(-D ONLINE)
endif(USE_ONLINE)

#utkscan will have debugging for the TreeCorrelator
if(USE_TREE_DEBUG)
    add_definitions(-D TREE_DEBUG)
endif(USE_TREE_DEBUG)

#utkscan will be more verbose in its output
if(USE_VERBOSE)
    add_definitions(-D VERBOSE)
endif(USE_VERBOSE)

#------------------------------------------------------------------------------

#Find packages needed for poll2
#Load additional find_package scripts.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

#Find thread library for poll2
find_package (Threads REQUIRED)

#Find curses library used for poll2/scan/skeleton/scope/etc
if(USE_NCURSES)
	find_package(Curses)
endif()
if (CURSES_FOUND)
	add_definitions("-D USE_NCURSES")
	mark_as_advanced(FORCE CURSES_CURSES_H_PATH CURSES_FORM_LIBRARY)
else()
	message(STATUS "Curses unavailable, basic terminal will be used.")
	set(USE_NCURSES OFF)
endif()

#Check if GSL is installed
if(USE_GSL)
    find_package(GSL REQUIRED)
    if(${GSL_VERSION} LESS 1.14)
        message(FATAL_ERROR "GSL 1.14 or higher required. Found GSL ${GSL_VERSION}")
    endif(${GSL_VERSION} LESS 1.14)
    add_definitions("-D usegsl")
endif(USE_GSL)

if (USE_POLL)
    #Find the PLX Library
    find_package(PLX REQUIRED)
    link_directories(${PLX_LIBRARY_DIR})
    #add_definitions("-DPLX_LINUX -DPCI_CODE -DPLX_LITTLE_ENDIAN")

    #Find the Pixie Firmware
    find_package(PXI REQUIRED)
    include_directories(${PXI_INCLUDE_DIR})
    link_directories(${PXI_LIBRARY_DIR})

    #Create pixie.cfg and copy slot_def.set as well as default.set to current.set
    PXI_CONFIG()
    #Use the cmake script created by PXI_CONFIG to install the files it created when make scancfg is typed
    add_custom_target(config ${CMAKE_COMMAND} -P pixie_cfg.cmake)

endif()

#Find ROOT if USE_ROOT was set.
if (USE_ROOT)
    find_package (ROOT REQUIRED)
    mark_as_advanced(FORCE GENREFLEX_EXECUTABLE ROOTCINT_EXECUTABLE
            ROOT_CONFIG_EXECUTABLE)
	include_directories(${ROOT_INCLUDE_DIR})
	link_directories(${ROOT_LIBRARY_DIR})
	add_definitions("-D USE_ROOT")
    add_definitions("-D useroot")
endif()

#------------------------------------------------------------------------------
#Build the core library
include_directories(Core/include)
add_subdirectory(Core)

#Build the scan library
include_directories(Scan/include)
add_subdirectory(Scan)

#Building polling tools
if (USE_POLL)
	#Build the pixie interface
	include_directories(Interface/include)
	add_subdirectory(Interface/source)

	#Build the MCA objects
	include_directories(MCA/include)
	add_subdirectory(MCA)

	#Build the setup tools
	if (USE_SETUP) 
		include_directories(Setup/include)
		add_subdirectory(Setup)
	endif()

	#Build poll
	add_subdirectory(Poll)
endif()

#Build the share objects
add_subdirectory(share)
