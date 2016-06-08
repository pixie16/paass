#Determine the sources based on use flags
set (MCA_LIB_SOURCES MCA.cpp)
if (${USE_ROOT}) 
	set (MCA_LIB_SOURCES ${MCA_LIB_SOURCES} MCA_ROOT.cpp)
endif()
if (${USE_DAMM}) 
	set (MCA_LIB_SOURCES ${MCA_LIB_SOURCES} MCA_DAMM.cpp HisDrr.cpp)
endif()

#build the MCA library
add_library(MCA_LIBRARY STATIC ${MCA_LIB_SOURCES})
target_link_libraries(MCA_LIBRARY PixieInterface Utility)
if (${USE_ROOT}) 
	target_link_libraries(MCA_LIBRARY ${ROOT_LIBRARIES})
endif()

#If we can we build the MCA program
if (${USE_DAMM} OR ${USE_ROOT})
	add_executable(MCA MCA_exec.cpp)
	target_link_libraries(MCA MCA_LIBRARY)
	install(TARGETS MCA DESTINATION bin)
else()
	message(STATUS "MCA not built as no compatible histogramming routines selected.")
endif()
