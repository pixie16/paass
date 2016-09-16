option(USE_HRIBF "Use HRIBF library for scan base." OFF)

if(USE_HRIBF)
   #Find HRIBF Libraries
   find_package(HRIBF REQUIRED)
   link_directories(${UPAK_LIBRARY_DIR} ${ACQ2_LIBRARY_DIR})
	add_subdirectory(scanor)
endif(USE_HRIBF)

#We will always build this since it includes a static lib for the rest
add_subdirectory(ScanLib)

#Build utilities.
add_subdirectory(util)

if(BUILD_UTKSCAN)
    add_subdirectory(utkscan)
endif(BUILD_UTKSCAN)
