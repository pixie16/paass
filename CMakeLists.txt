cmake_minimum_required(VERSION 2.8.8)

project(PixieSuite C CXX Fortran)

#Compile with c++11 or c++0x depending on the compiler version
if (CMAKE_COMPILER_IS_GNUCXX)
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "4.3" AND
            ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "4.7")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    elseif (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "4.6")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    else ()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "4.3" AND
            ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "4.7")

    #Make compiler messages nice with colored tags.
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "4.9")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=auto")
    endif ()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -fPIC")
endif ()

#if user does not specify prefix we assign it to the install directory
#@TODO I do not like the fact that it makes the install directory before I'm
#ready for it. There are cases where I do not want to actually install just
#build. This configuration adds additional junk to my directory that I may
#not actually want or need.
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(STATUS "Install Prefix not specified.")
    file(MAKE_DIRECTORY install)
    get_filename_component(INSTALL_DIR ${CMAKE_BINARY_DIR}/../install REALPATH)
    set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH "Install Prefix" FORCE)
endif ()
message(STATUS "Installing to ${CMAKE_INSTALL_PREFIX}")

#Define the default build type to be Release
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
            "Build type, options are: None Debug Release RelWithDebInfo MinSizeRel."
            FORCE)
    message(STATUS "Build type not defined, using default: ${CMAKE_BUILD_TYPE}")
endif (NOT CMAKE_BUILD_TYPE)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#Add additional CXX flags if we use the Debug option
if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
endif (CMAKE_BUILD_TYPE MATCHES "Debug")

#------------------------------------------------------------------------------
#We are going to include this additional module here since it could be useful
#when setting all of the following options.
include(CMakeDependentOption)

#Install options
option(PAASS_BUILD_ACQ "Build and install Acquisition software" ON)
option(PAASS_BUILD_SCAN_UTILITIES "Build and install the Analysis Utilities" ON)
option(PAASS_BUILD_SETUP "Include the older setup programs in installation" OFF)
option(PAASS_BUILD_SHARED_LIBS "Install only scan libraries" ON)
option(PAASS_BUILD_TESTS "Builds programs designed to test the package." OFF)
option(PAASS_BUILD_UNITTESTS "Bulid Unit Tests using UnitTest++ Framework" OFF)
option(PAASS_BUILD_UTKSCAN "Build utkscan" OFF)

option(PAASS_USE_DAMM "Use DAMM for MCA" ON)
option(PAASS_USE_NCURSES "Use ncurses for terminal" ON)
mark_as_advanced(PAASS_USE_NCURSES)
option(PAASS_USE_ROOT "Use ROOT" ON)

#------------------------------------------------------------------------------

#Definitions with compiler options
#The MCA will write DAMM histograms as output
if (PAASS_USE_DAMM)
    add_definitions("-D USE_DAMM")
endif (PAASS_USE_DAMM)

#------------------------------------------------------------------------------
#Find packages needed for the software.
#Load additional find_package scripts.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Cmake/modules/")

#Find thread library for poll2 and scanLib
find_package(Threads REQUIRED)

#Find the PLX Library
find_package(PLX)

#Find the Pixie Firmware
find_package(XIA)

#Find curses library used for pretty much everything at this point.
if (PAASS_USE_NCURSES)
    find_package(Curses REQUIRED)
endif (PAASS_USE_NCURSES)

if (CURSES_FOUND)
    add_definitions("-D USE_NCURSES")
    mark_as_advanced(FORCE CURSES_HAVE_CURSES_H CURSES_CURSES_H_PATH CURSES_FORM_LIBRARY)
else ()
    message(STATUS "Curses unavailable, basic terminal will be used.")
    set(PAASS_USE_NCURSES OFF)
endif (CURSES_FOUND)

#Find the UnitTest++ Package. This package can be obtained from
#https://github.com/unittest-cpp/unittest-cpp.git
if (PAASS_BUILD_UNITTESTS)
    find_package(UnitTest++ REQUIRED)
    link_directories(${UNITTEST++_LIBRARY_DIR})
    include_directories(${UNITTEST++_INCLUDE_DIR})
endif (PAASS_BUILD_UNITTESTS)

if (PAASS_BUILD_ACQ)
    if (NOT PAASS_BUILD_ACQ_ATTEMPTED AND NOT (PLX_FOUND AND XIA_FOUND))
        set(PAASS_BUILD_ACQ OFF CACHE BOOL "Build and install Acquisition" FORCE)
    else (PLX_FOUND OR XIA_FOUND)
        #Find the PLX Library
        find_package(PLX REQUIRED)
        link_directories(${PLX_LIBRARY_DIR})

        #Find the Pixie Firmware
        find_package(XIA REQUIRED)
        include_directories(${XIA_INCLUDE_DIR})
        link_directories(${XIA_LIBRARY_DIR})

        #Create pixie.cfg and copy slot_def.set as well as default.set to current.set
        XIA_CONFIG()
        #Use the cmake script created by XIA_CONFIG to install the files it
        #created when make cfg is typed
        add_custom_target(config ${CMAKE_COMMAND} -P pixie_cfg.cmake)
    endif ()
    set(PAASS_BUILD_ACQ_ATTEMPTED ON CACHE INTERNAL "Build Suite Attempted")
endif (PAASS_BUILD_ACQ)

#Find ROOT if USE_ROOT was set.
if (PAASS_USE_ROOT)
    find_package(ROOT REQUIRED)
    mark_as_advanced(FORCE GENREFLEX_EXECUTABLE ROOTCINT_EXECUTABLE
            ROOT_CONFIG_EXECUTABLE)
    include_directories(${ROOT_INCLUDE_DIR})
    link_directories(${ROOT_LIBRARY_DIR})
    add_definitions("-D USE_ROOT")
    add_definitions("-D useroot")
endif (PAASS_USE_ROOT)

#------------------------------------------------------------------------------
#Put all the include directories that we might need at this point. Ensure
#that the Third party includes come before the Resources.
include_directories(ThirdParty/include)
include_directories(Resources/include)
include_directories(Core/include)

#Build the Third party software since it's useful for other programs
add_subdirectory(ThirdParty)

#Add the resources to compilation
add_subdirectory(Resources)

#Build the PaassCoreLibrary.
add_subdirectory(Core)

#Build Acquisition software, we don't have this broken into as fine of a
#granularity as the Analysis software, so we just wrap the whole thing in an if.
if (PAASS_BUILD_ACQ)
    add_subdirectory(Acquisition)
else ()
    #Ensure that we can still build set2* even when BUILD_ACQ is off
    add_subdirectory(Acquisition/set2root)
endif (PAASS_BUILD_ACQ)

#Build any of the analysis related things that we need to build.
add_subdirectory(Analysis)

#Build/install the miscellaneous stuff
add_subdirectory(Share)
