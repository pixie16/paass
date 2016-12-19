#Set the utility sources that we will make a lib out of
set(ResourceSources PolynomialCfd.cpp)

if (USE_GSL)
    if (${GSL_VERSION} GREATER 1.9)
        set(ResourceSources ${ResourceSources} Gsl2Fitter.cpp)
    else (${GSL_VERSION} LESS 2.0)
        set(ResourceSources ${ResourceSources} Gsl1Fitter.cpp)
    endif (${GSL_VERSION} GREATER 1.9)
endif (USE_GSL)

if (USE_ROOT)
    set(ResourceSources ${ResourceSources} RootFitter.cpp
            VandleTimingFunction.cpp)
endif (USE_ROOT)

#Add the sources to the library
add_library(ResourceObjects OBJECT ${ResourceSources})

if (BUILD_SHARED_LIBS)
    message(STATUS "Building Utility Shared Objects")
    add_library(UtilityLibrary SHARED $<TARGET_OBJECTS:ResourceObjects>)
    target_link_libraries(UtilityLibrary PixieCoreStatic
            ${CMAKE_THREAD_LIBS_INIT})
    if (USE_ROOT)
        target_link_libraries(UtilityLibrary ${ROOT_LIBRARIES})
    endif (USE_ROOT)
    if (${CURSES_FOUND})
        target_link_libraries(UtilityLibrary ${CURSES_LIBRARIES})
    endif ()
    install(TARGETS UtilityLibrary DESTINATION lib)
endif (BUILD_SHARED_LIBS)

#Create Utility static library and add ncurses if we have it
add_library(ResourceStatic STATIC $<TARGET_OBJECTS:ResourceObjects>)
target_link_libraries(ResourceStatic ${CMAKE_THREAD_LIBS_INIT})
if (USE_ROOT)
    target_link_libraries(ResourceStatic ${ROOT_LIBRARIES})
endif (USE_ROOT)