option(USE_HRIBF "Use HRIBF library for scan base." OFF)

#Everything below is dependent on ScanLib, so we include the headers.
include_directories(ScanLib/include)

if(USE_HRIBF)
   #Find HRIBF Libraries
   find_package(HRIBF REQUIRED)
   add_definitions("-DUSE_HRIBF")

	#If we are using HRIBF interface we need to include the ScanorInterface header
	#	for the following code.
   include_directories(scanor/include)
   add_subdirectory(scanor)
endif(USE_HRIBF)

#We will always build this since it includes a static lib for the rest
include_directories(ScanLib/include)
add_subdirectory(ScanLib)

#Build utilities.
add_subdirectory(util)

if(BUILD_UTKSCAN)
    add_subdirectory(utkscan)
endif(BUILD_UTKSCAN)