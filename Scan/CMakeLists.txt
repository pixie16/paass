option(USE_HRIBF "Use HRIBF library for scan base." OFF)

include_directories(ScanLib/include)

#Install include directories to support the shared library.
if(BUILD_SHARED_LIBS)
  install(DIRECTORY ScanLib/include DESTINATION ${CMAKE_INSTALL_PREFIX})
endif(BUILD_SHARED_LIBS)

if(USE_HRIBF)
   #Find HRIBF Libraries
   include_directories(scanor/include)
   add_definitions("-DUSE_HRIBF")
   find_package(HRIBF REQUIRED)
   link_directories(${UPAK_LIBRARY_DIR} ${ACQ2_LIBRARY_DIR})
   add_subdirectory(scanor/source)
endif(USE_HRIBF)

#We will always build this since it includes a static lib for the rest
add_subdirectory(ScanLib/source)

#Build utilities.
add_subdirectory(util/source)

if(BUILD_UTKSCAN)
    add_subdirectory(utkscan)
endif(BUILD_UTKSCAN)
