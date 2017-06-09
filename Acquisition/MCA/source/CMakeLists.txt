# @authors K. Smith
#Determine the sources based on use flags
set(MCA_LIB_SOURCES MCA.cpp)

if (PAASS_USE_ROOT)
    set(MCA_LIB_SOURCES ${MCA_LIB_SOURCES} MCA_ROOT.cpp)
endif (PAASS_USE_ROOT)

if (PAASS_USE_DAMM)
    set(MCA_LIB_SOURCES ${MCA_LIB_SOURCES} MCA_DAMM.cpp HisDrr.cpp)
endif (PAASS_USE_DAMM)

#build the MCA library
add_library(MCA_LIBRARY STATIC ${MCA_LIB_SOURCES})
target_link_libraries(MCA_LIBRARY PixieInterface Utility)
if (PAASS_USE_ROOT)
    target_link_libraries(MCA_LIBRARY ${ROOT_LIBRARIES})
endif (PAASS_USE_ROOT)

#If we can we build the MCA program
if (PAASS_USE_DAMM OR PAASS_USE_ROOT)
    add_executable(MCA MCA_exec.cpp)
    target_link_libraries(MCA MCA_LIBRARY)
    install(TARGETS MCA DESTINATION bin)
else ()
    message(STATUS "MCA not built as no compatible histogramming routines selected.")
endif ()
