# @authors S. V. Paulauskas, K. Smith, and C. R. Thornsberry

#Adds the install prefix for referencing in the source code
add_definitions(-D INSTALL_PREFIX="\\"${CMAKE_INSTALL_PREFIX}\\"")

#Build the pixie interface
include_directories(Interface/include)
add_subdirectory(Interface/source)

#Build the MCA objects
include_directories(MCA/include)
add_subdirectory(MCA)

#Build PxiDump
add_subdirectory(set2root)

#Build poll
add_subdirectory(Poll)

#Build the setup tools
if (PAASS_BUILD_SETUP)
    add_subdirectory(Setup)
endif (PAASS_BUILD_SETUP)