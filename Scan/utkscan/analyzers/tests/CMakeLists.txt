if(USE_GSL)
    if(${GSL_VERSION} GREATER 1.9)
        set(GSL_FITTER_SOURCES ../source/Gsl2Fitter.cpp)
    else(${GSL_VERSION} LESS 2.0)
        set(GSL_FITTER_SOURCES ../source/Gsl1Fitter.cpp)
    endif(${GSL_VERSION} GREATER 1.9)

    #Build the test to see if the GSL fitting algorithm is behaving.
    set(GSL_FITTER_SOURCES ${GSL_FITTER_SOURCES} test_gslfitter.cpp)
    add_executable(test_gslfitter ${GSL_FITTER_SOURCES})
    target_link_libraries(test_gslfitter ${GSL_LIBRARIES} UnitTest++)
endif(USE_GSL)