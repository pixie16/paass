option(USE_HRIBF "Use HRIBF library for scan base." OFF)
option(USE_GSL "Use GSL for Pulse Fitting" OFF)

#Check if GSL is installed
if(USE_GSL)
   find_package(GSL REQUIRED)
   add_definitions("-D usegsl")
endif(USE_GSL)

#Everything below is dependent on these two sets of libaries so we include the
#headers.
include_directories(Resources/include)
include_directories(ScanLib/include)

if(USE_HRIBF)
   #Find HRIBF Libraries
   find_package(HRIBF REQUIRED)
   add_definitions("-D USE_HRIBF")

	#If we are using HRIBF interface we need to include the ScanorInterface header
	#	for the following code.
   include_directories(scanor/include)
   add_subdirectory(scanor)
endif(USE_HRIBF)

#We will always build these two since they include static lib for the rest
add_subdirectory(ScanLib)
add_subdirectory(Resources)

#Build utilities.
add_subdirectory(util)


if(BUILD_UTKSCAN)
    add_subdirectory(utkscan)
endif(BUILD_UTKSCAN)