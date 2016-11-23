cmake_minimum_required(VERSION 2.8.8)

project(PixieSuite C CXX Fortran)

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
   file(MAKE_DIRECTORY install)
   get_filename_component(INSTALL_DIR ${CMAKE_SOURCE_DIR}/install REALPATH)
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
option(BUILD_SCOPE "Build and install the scope program." ON)
option(BUILD_SETUP "Include the older setup programs in installation" OFF)
option(BUILD_SHARED_LIBS "Install only scan libraries" ON)
option(BUILD_SKELETON "Build and install the skeleton scan" OFF)
option(BUILD_SUITE "Build and install PixieSuite" ON)
option(BUILD_TESTS "Builds programs designed to test the package." OFF)
option(BUILD_UTKSCAN "Build utkscan" OFF)

option(USE_DAMM "Use DAMM for MCA" ON)
option(USE_NCURSES "Use ncurses for terminal" ON)
mark_as_advanced(USE_NCURSES)
option(USE_ROOT "Use ROOT" ON)

#------------------------------------------------------------------------------

#Definitions without options

#Adds the install prefix for referencing in the source code
add_definitions(-D INSTALL_PREFIX="\\"${CMAKE_INSTALL_PREFIX}\\"")

#Definitions with options

#The MCA will write DAMM histograms as output
if(USE_DAMM)
    add_definitions("-D USE_DAMM")
endif()

#------------------------------------------------------------------------------

#Find packages needed for poll2
#Load additional find_package scripts.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

#Find thread library for poll2 and scanLib
find_package (Threads REQUIRED)

#Find the PLX Library
find_package (PLX)

#Find the Pixie Firmware
find_package (PXI)

#Find the UnitTest++ Package. This package can be obtained from
#https://github.com/unittest-cpp/unittest-cpp.git
if(BUILD_TESTS)
	find_package(UnitTest++)
	link_directories(${UNITTEST++_LIBRARY_DIR})
	include_directories(${UNITTEST++_INCLUDE_DIR})
endif(BUILD_TESTS)

#Find curses library used for poll2/scan/skeleton/scope/etc
if(USE_NCURSES)
	find_package(Curses REQUIRED)
endif()
if (CURSES_FOUND)
	add_definitions("-D USE_NCURSES")
	mark_as_advanced(FORCE CURSES_HAVE_CURSES_H CURSES_CURSES_H_PATH CURSES_FORM_LIBRARY)
else()
	message(STATUS "Curses unavailable, basic terminal will be used.")
	set(USE_NCURSES OFF)
endif()

if (BUILD_SUITE)
	if(NOT BUILD_SUITE_ATTEMPTED AND NOT PLX_FOUND AND NOT PXI_FOUND)
		set (BUILD_SUITE OFF CACHE BOOL "Build and install PixieSuite" FORCE)
	else (PLX_FOUND OR PXI_FOUND)
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
	   #Use the cmake script created by PXI_CONFIG to install the files it
	   #created when make cfg is typed
	   add_custom_target(config ${CMAKE_COMMAND} -P pixie_cfg.cmake)
	endif()
	set (BUILD_SUITE_ATTEMPTED ON CACHE INTERNAL "Build Suite Attempted")
endif(BUILD_SUITE)

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
#Build the Core library
include_directories(Core/include)
add_subdirectory(Core)

#Build the scan library
add_subdirectory(Scan)

#Building polling tools
if (BUILD_SUITE)
	#Build the pixie interface
	include_directories(Interface/include)
	add_subdirectory(Interface/source)

	#Build the MCA objects
	include_directories(MCA/include)
	add_subdirectory(MCA)

	#Build the setup tools
	if (BUILD_SETUP) 
		include_directories(Setup/include)
		add_subdirectory(Setup)
	endif()

	#Build poll
	add_subdirectory(Poll)
endif()

#Build PxiDump
add_subdirectory(PxiDump)

#Build/install the miscellaneous stuff
add_subdirectory(share)
