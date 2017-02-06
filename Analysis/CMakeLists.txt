option(USE_HRIBF "Use HRIBF library for scan base." OFF)
CMAKE_DEPENDENT_OPTION(USE_GSL "Compile with GSL" ON "BUILD_UTKSCAN" OFF)
mark_as_advanced(USE_GSL)

#Check if GSL is installed
if(USE_GSL)
   find_package(GSL REQUIRED)
   add_definitions("-D usegsl")
endif(USE_GSL)

#Everything below is dependent on these two sets of libaries so we include the
#headers.
include_directories(Resources/include)
include_directories(ScanLibraries/include)

if(USE_HRIBF)
   #Find HRIBF Libraries
   find_package(HRIBF REQUIRED)
   add_definitions("-D USE_HRIBF")

	#If we are using HRIBF interface we need to include the ScanorInterface header
	#	for the following code.
   include_directories(Scanor/include)
   add_subdirectory(Scanor)
endif(USE_HRIBF)

#We will always build these two since they include static lib for the rest
add_subdirectory(ScanLibraries)
add_subdirectory(Resources)

#Build utilities.
add_subdirectory(Utilities)

if(BUILD_UTKSCAN)
    add_subdirectory(Utkscan)
endif(BUILD_UTKSCAN)