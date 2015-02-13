cmake_minimum_required(VERSION 2.8)

project(PixieSuite)

#Compile with C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

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

#Find packages needed for poll2
#Load additional find_package scripts.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

#Find thread library for poll2
find_package (Threads REQUIRED)

#Find curses library used for poll2 prompt
find_package(Curses)
if (${CURSES_FOUND})
	add_definitions("-D USE_NCURSES")
else()
	message(STATUS "Curses unavailable, basic terminal will be used.")
endif()

#Find ROOT if USE_ROOT was set.
if (${USE_ROOT})
   find_package (ROOT REQUIRED)

	include_directories(${ROOT_INCLUDE_DIR})
	link_directories(${ROOT_LIBRARY_DIR})
	add_definitions("-D USE_ROOT")
endif()

if(${USE_DAMM})
	add_definitions("-D USE_DAMM")
endif()

#Find the PLX Library
set(PLX_LIB_DIR /opt/PLX_API/PLX/PlxSdk/PlxApi/Library)
link_directories(${PLX_LIB_DIR})
set(PLX_LIBRARIES -lPlxApi -ldl)
#add_definitions("-DPLX_LINUX -DPCI_CODE -DPLX_LITTLE_ENDIAN")

#Find the Pixie Firmware
set(PXI_ROOT /opt/PixieFirmware/current)
include_directories(${PXI_ROOT}/software/inc ${PXI_ROOT}/software/sys ${PXI_ROOT}/software/app)
link_directories(${PXI_ROOT}/software)
set(PXI_LIBRARIES -lPixie16App -lPixie16Sys)
#The following configuration file path is hardcoded into the PXI software :-(
install(FILES ${PXI_ROOT}/test/pxisys.ini DESTINATION ${CMAKE_INSTALL_PREFIX})

include_directories(Interface/include)
add_subdirectory(Interface/source)

include_directories(Setup/include)
include_directories(MCA/include)
add_subdirectory(MCA)
add_subdirectory(Setup/source)


add_subdirectory(Poll)

add_subdirectory(PxiDump/source)
