#We will always build this since it includes a static lib for the rest
add_subdirectory(ScanLib)

if(BUILD_SCOPE OR BUILD_SKELETON)
    add_subdirectory(util)
endif(BUILD_SCOPE OR BUILD_SKELETON)

if(BUILD_UTKSCAN)
    add_subdirectory(utkscan)
endif(BUILD_UTKSCAN)