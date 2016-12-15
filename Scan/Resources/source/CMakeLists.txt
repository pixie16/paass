#Set the utility sources that we will make a lib out of
set(UtilitySources PolynomialCfdCalculator.cpp)

if(USE_GSL)
    if(${GSL_VERSION} GREATER 1.9)
        set(UtilitySources ${UtilitySources} Gsl2Fitter.cpp)
    else(${GSL_VERSION} LESS 2.0)
        set(UtilitySources ${UtilitySources} Gsl1Fitter.cpp)
    endif(${GSL_VERSION} GREATER 1.9)
endif(USE_GSL)

#Add the sources to the library
add_library(UtilityObjects OBJECT ${UtilitySources})

if(BUILD_SHARED_LIBS)
    message(STATUS "Building Utility Shared Objects")
    add_library(UtilityLibrary SHARED $<TARGET_OBJECTS:UtilityObjects>)
    target_link_libraries(UtilityLibrary PixieCoreStatic
            ${CMAKE_THREAD_LIBS_INIT})
    if (${CURSES_FOUND})
        target_link_libraries(UtilityLibrary ${CURSES_LIBRARIES})
    endif()
    install(TARGETS UtilityLibrary DESTINATION lib)
endif(BUILD_SHARED_LIBS)

#Create Utility static library and add ncurses if we have it
add_library(UtilityStatic STATIC $<TARGET_OBJECTS:UtilityObjects>)
target_link_libraries(UtilityStatic ${CMAKE_THREAD_LIBS_INIT})