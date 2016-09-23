#We will always build this since it includes a static lib for the rest
include_directories(ScanLib/include)
add_subdirectory(ScanLib)

#Build utilities.
add_subdirectory(util)

if(BUILD_UTKSCAN)
    add_subdirectory(utkscan)
endif(BUILD_UTKSCAN)