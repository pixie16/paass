#Set the scan sources that we will make a lib out of
set(ScanSources
        core/ScanInterface.cpp
        core/Unpacker.cpp
        core/XiaData.cpp)
#Add the sources to the library
add_library(ScanObjects OBJECT ${ScanSources})

if(BUILD_SHARED_LIBS)
    message(STATUS "Building Scan Shared Objects")
    add_library(Scan SHARED $<TARGET_OBJECTS:ScanObjects>)
    target_link_libraries(Scan PixieCoreStatic ${CMAKE_THREAD_LIBS_INIT})
    if (${CURSES_FOUND})
        target_link_libraries(Scan ${CURSES_LIBRARIES})
    endif()
    install(TARGETS Scan DESTINATION lib)
endif(BUILD_SHARED_LIBS)

#Create PixieScan static library and add ncurses if we have it
add_library(ScanStatic STATIC $<TARGET_OBJECTS:ScanObjects>)
target_link_libraries(ScanStatic PixieCoreStatic ${CMAKE_THREAD_LIBS_INIT})
if (${CURSES_FOUND})
    target_link_libraries(ScanStatic ${CURSES_LIBRARIES})
endif()

#Build and install utkscan
if(BUILD_UTKSCAN)
    #Add Subdirectories to the build
    add_subdirectory(core)
    add_subdirectory(analyzers)
    add_subdirectory(processors)
    add_subdirectory(experiment)

    #Create utkscan program
    add_executable(utkscan
            $<TARGET_OBJECTS:CoreObjects>
            $<TARGET_OBJECTS:AnalyzerObjects>
            $<TARGET_OBJECTS:ProcessorObjects>
            $<TARGET_OBJECTS:ExperimentObjects>)

    #Add libraries to be linked with utkscan
    target_link_libraries(utkscan ${LIBS} ScanStatic)
    #If we have GSL installed link it as well
    if(USE_GSL)
        target_link_libraries(utkscan ${GSL_LIBRARIES})
    endif(USE_GSL)
    if(USE_ROOT)
        target_link_libraries(utkscan ${ROOT_LIBRARIES})
    endif(USE_ROOT)
    #Install utkscan to the bin directory
    install(TARGETS utkscan DESTINATION bin)
endif(BUILD_UTKSCAN)

#Build and install the scope program
if(USE_ROOT AND BUILD_SCOPE)
    #Compile and install the scope program
    set(SCOPE_SOURCES util/scope.cpp)
    add_executable(scope ${SCOPE_SOURCES})
    target_link_libraries(scope ScanStatic ${ROOT_LIBRARIES})
    install(TARGETS scope DESTINATION bin)
endif(USE_ROOT AND BUILD_SCOPE)

if(BUILD_SKELETON)
# Build and install the scanner skeleton source.
add_executable(skeleton util/Skeleton.cpp)
target_link_libraries(skeleton ScanStatic)
install (TARGETS skeleton DESTINATION bin)
endif(BUILD_SKELETON)