option(USE_GSL "Use GSL for Pulse Fitting" ON)

#Check if GSL is installed
if(USE_GSL)
    find_package(GSL REQUIRED)
    if(${GSL_VERSION} LESS 2)
        message(FATAL_ERROR "GSL 2 or higher required. Found GSL ${GSL_VERSION}")
    endif(${GSL_VERSION} LESS 2)
    add_definitions("-D usegsl")
endif(USE_GSL)

include_directories(include ../ScanLib/include)
add_subdirectory(source)
