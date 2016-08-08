#Build the program to test the GslFitter classes and its derived classes
if(USE_GSL)
    include_directories(../include)
    if(GSL_VERSION VERSION_GREATER 1.9)
        set(GSL_FITTER_SOURCES ../analyzers/Gsl2Fitter.cpp)
    else(GSL_VERSION VERSION_LESS 2.0)
        set(GSL_FITTER_SOURCES ../analyzers/Gsl1Fitter.cpp)
    endif(GSL_VERSION VERSION_GREATER 1.9)

    set(GSL_FITTER_SOURCES ${GSL_FITTER_SOURCES} test_gslfitter.cpp)

    add_executable(test_gslfitter ${GSL_FITTER_SOURCES})

    target_link_libraries(test_gslfitter ${GSL_LIBRARIES})
endif(USE_GSL)