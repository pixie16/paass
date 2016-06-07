cmake_minimum_required(VERSION 2.8.8)

project(PixieSuite)

#Compile with C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
if (CMAKE_COMPILER_IS_GNUCXX)
	if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "4.9")
		#Make compiler messages nice with colored tags.
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=auto")
	endif()

	if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
	   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
	endif()
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
endif()

#if user does not specify prefix we assign it to the exec directory
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   message(STATUS "Install Prefix not specified.")
   file(MAKE_DIRECTORY exec)
   get_filename_component(INSTALL_DIR ${CMAKE_SOURCE_DIR}/exec REALPATH)
   set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH
         "Install Prefix" FORCE)
endif ()
message(STATUS "Installing to ${CMAKE_INSTALL_PREFIX}")

#Define the default build type
IF(NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Build type, options are: None Debug Release
         RelWithDebInfo MinSizeRel."
      FORCE)
   message(STATUS "Build type not defined, using default.")
ENDIF(NOT CMAKE_BUILD_TYPE)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#Install options
option(USE_ROOT "Use ROOT for MCA." ON)
option(USE_DAMM "Use DAMM for MCA." ON)
option(USE_SETUP "Include the older setup programs in installation." OFF)
option(USE_NCURSES "Use ncurses for terminal." ON)
option(USE_SCAN "Install PixieScan." ON)
option(USE_POLL "Install Poll." ON)

#The following line has backslashes to escape the backslashes which escape the double quotes.
add_definitions(-D INSTALL_PREFIX="\\"${CMAKE_INSTALL_PREFIX}\\"")

#Find packages needed for poll2
#Load additional find_package scripts.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

#Find thread library for poll2
find_package (Threads REQUIRED)

#Find curses library used for poll2 prompt
if(${USE_NCURSES}) 
	find_package(Curses)
endif()
if (${CURSES_FOUND})
	add_definitions("-D USE_NCURSES")
	mark_as_advanced(FORCE CURSES_CURSES_H_PATH CURSES_FORM_LIBRARY)
else()
	message(STATUS "Curses unavailable, basic terminal will be used.")
	set(USE_NCURSES OFF)
endif()

#Find ROOT if USE_ROOT was set.
if (${USE_ROOT})
   find_package (ROOT REQUIRED)
   mark_as_advanced(FORCE GENREFLEX_EXECUTABLE ROOTCINT_EXECUTABLE ROOT_CONFIG_EXECUTABLE)

	include_directories(${ROOT_INCLUDE_DIR})
	link_directories(${ROOT_LIBRARY_DIR})
	add_definitions("-D USE_ROOT")
endif()

if(${USE_DAMM})
	add_definitions("-D USE_DAMM")
endif()

if (${USE_POLL})
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
	#Use the cmake script created by PXI_CONFIG to install the files it created when make config is typed
	add_custom_target(config ${CMAKE_COMMAND} -P pixie_cfg.cmake)	

endif()

#Build the core library
include_directories(Core/include)
add_subdirectory(Core)

#Build the scan library
if (${USE_SCAN})
	include_directories(Scan/include)
	add_subdirectory(Scan)
endif()

#Building polling tools
if (${USE_POLL})
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

#Build PxiDump
add_subdirectory(PxiDump)

#Build the share objects
add_subdirectory(share)

