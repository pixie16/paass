#Build the program to test the GslFitter classes and its derived classes
if(USE_GSL)
    #I am going to point this guy to the specific sources that he needs.
    include_directories(../../analyzers/include)

    if(${GSL_VERSION} GREATER 1.9)
        set(GSL_FITTER_SOURCES ../../analyzers/source/Gsl2Fitter.cpp)
    else(${GSL_VERSION} LESS 2.0)
        set(GSL_FITTER_SOURCES ../../analyzers/source/Gsl1Fitter.cpp)
    endif(${GSL_VERSION} GREATER 1.9)

    set(GSL_FITTER_SOURCES ${GSL_FITTER_SOURCES} test_gslfitter.cpp)

    add_executable(test_gslfitter ${GSL_FITTER_SOURCES})

    target_link_libraries(test_gslfitter ${GSL_LIBRARIES})
endif(USE_GSL)