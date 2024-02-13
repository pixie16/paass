#@authors K. Smith, S. V. Paulauskas, C. R. Thornsberry
option(PAASS_USE_HRIBF "Use HRIBF library for scan base." OFF)
CMAKE_DEPENDENT_OPTION(PAASS_USE_GSL "Compile with GSL" ON
        "PAASS_BUILD_UTKSCAN" OFF)
mark_as_advanced(PAASS_USE_GSL)

#Check if GSL is installed
if (PAASS_USE_GSL)
    find_package(GSL REQUIRED)
    add_definitions("-D usegsl")
endif (PAASS_USE_GSL)

#Everything below is dependent on these two sets of libaries so we include the
#headers.
include_directories(Resources/include)
include_directories(ScanLibraries/include)

if (PAASS_USE_HRIBF)
    #Find HRIBF Libraries
    find_package(HRIBF REQUIRED)
    add_definitions("-D USE_HRIBF")
    set(CMAKE_Fortran_FLAGS ${CMAKE_Fortran_FLAGS} "-Wl,-no_compact_unwinding")

    #If we are using HRIBF interface we need to include the ScanorInterface header
    #	for the following code.
    include_directories(Scanor/include)
    add_subdirectory(Scanor)
endif (PAASS_USE_HRIBF)

#We will always build these two since they include static lib for the rest
add_subdirectory(ScanLibraries)
add_subdirectory(Resources)

#Build utilities.
if(PAASS_BUILD_SCAN_UTILITIES)
    add_subdirectory(Utilities)
endif(PAASS_BUILD_SCAN_UTILITIES)

if (PAASS_BUILD_UTKSCAN)
    add_subdirectory(Utkscan)
endif (PAASS_BUILD_UTKSCAN)