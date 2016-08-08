option(USE_GSL "Use GSL for Pulse Fitting" ON)

#Check if GSL is installed
if(USE_GSL)
    find_package(GSL REQUIRED)
    add_definitions("-D usegsl")
endif(USE_GSL)

include_directories(include ../ScanLib/include)
add_subdirectory(source)
