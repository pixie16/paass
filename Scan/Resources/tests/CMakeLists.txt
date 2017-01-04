if(BUILD_UNITTESTS)
    if(USE_GSL)
        if(${GSL_VERSION} GREATER 1.9)
            set(GSL_FITTER_SOURCES ../source/Gsl2Fitter.cpp)
        else(${GSL_VERSION} LESS 2.0)
            set(GSL_FITTER_SOURCES ../source/Gsl1Fitter.cpp)
        endif(${GSL_VERSION} GREATER 1.9)

        #Build the test to see if the GSL fitting algorithm is behaving.
        set(GSL_FITTER_SOURCES ${GSL_FITTER_SOURCES} unittest-GslFitter.cpp)
        add_executable(unittest-GslFitter ${GSL_FITTER_SOURCES})
        target_link_libraries(unittest-GslFitter ${GSL_LIBRARIES} UnitTest++)
    endif(USE_GSL)

    add_executable(unittest-HelperFunctions unittest-HelperFunctions.cpp)
    target_link_libraries(unittest-HelperFunctions UnitTest++)
    install(TARGETS unittest-HelperFunctions DESTINATION bin/unittests)

    add_executable(unittest-PolynomialCfd unittest-PolynomialCfd.cpp
            ../source/PolynomialCfd.cpp)
    target_link_libraries(unittest-PolynomialCfd UnitTest++)
    install(TARGETS unittest-PolynomialCfd DESTINATION bin/unittests)

    add_executable(unittest-TraditionalCfd unittest-TraditionalCfd.cpp
            ../source/TraditionalCfd.cpp)
    target_link_libraries(unittest-TraditionalCfd UnitTest++)
    install(TARGETS unittest-TraditionalCfd DESTINATION bin/unittests)

    if(USE_ROOT)
        add_executable(unittest-RootFitter unittest-RootFitter.cpp
                ../source/RootFitter.cpp ../source/VandleTimingFunction.cpp)
        target_link_libraries(unittest-RootFitter
                ${ROOT_LIBRARIES})
        install(TARGETS unittest-RootFitter DESTINATION bin/unittests)
    endif(USE_ROOT)
endif(BUILD_UNITTESTS)