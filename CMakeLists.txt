cmake_minimum_required(VERSION 2.8.8) 

project(Pixie_Scan CXX Fortran)

if (CMAKE_COMPILER_IS_GNUCXX)
	if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "4.9")
		#Make compiler messages nice with colored tags.
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=auto")
	endif()
	
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -fPIC -std=c++0x")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -std=c++0x")
endif()

set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fsecond-underscore")
add_definitions(-D LINK_GFORTRAN)

#if user does not specify prefix we assign it to the exec directory
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   message(STATUS "Install Prefix not specified.")
   get_filename_component(INSTALL_DIR ${CMAKE_SOURCE_DIR} REALPATH)
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
   message(STATUS "Build type not defined, using default: ${CMAKE_BUILD_TYPE}")
ENDIF(NOT CMAKE_BUILD_TYPE)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#Install options
option(USE_ROOT "Use ROOT for analysis" ON)
option(USE_NEWREADOUT "Use new readout format" ON)
option(USE_GSL "Use GSL for Pulse Fitting" ON)

#Find packages needed for pixie_scan
#Load additional find_package scripts.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmakeModules/")

#Find HRIBF Libraries
find_package(HRIBF REQUIRED)
link_directories(${UPAK_LIBRARY_DIR} ${ACQ2_LIBRARY_DIR})

#Find ROOT if USE_ROOT was set.
if (USE_ROOT)
   find_package (ROOT REQUIRED)
   mark_as_advanced(FORCE GENREFLEX_EXECUTABLE ROOTCINT_EXECUTABLE ROOT_CONFIG_EXECUTABLE)

   include_directories(${ROOT_INCLUDE_DIR})
   link_directories(${ROOT_LIBRARY_DIR})
   add_definitions("-D useroot")
endif(USE_ROOT)

if (USE_GSL)
   set(GSL_LIBS -lgsl -lgslcblas)
endif(USE_GSL)

#Build the core library
include_directories(include)
add_subdirectory(src)