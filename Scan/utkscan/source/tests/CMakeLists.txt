#Build the program to test the GslFitter classes and its derived classes
if(USE_GSL)
    include_directories(../include)
    if(GSL_VERSION VERSION_GREATER 1.9)
        set(GSL_FITTER_SOURCE Gsl2Fitter.cpp)
    else()
        set(GSL_FITTER_SOURCE Gsl1Fitter.cpp)
    endif(GSL_VERSION VERSION_GREATER 1.9)

    add_executable(test_gslfitter test_gslfitter.cpp)

    target_link_libraries(test_gslfitter ${GSL_LIBRARIES})
endif(USE_GSL)