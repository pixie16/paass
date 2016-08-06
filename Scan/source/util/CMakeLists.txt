#Build the scope program and the filterer program
if(USE_ROOT)
	#Compile and install the scope program
	set(SCOPE_SOURCES scope.cpp)
	add_executable(scope ${SCOPE_SOURCES})
	target_link_libraries(scope PixieScanStatic ${ROOT_LIBRARIES})
#
#	add_executable(filterer filterer.cpp)
#	target_link_libraries(filterer PixieScanStatic ${ROOT_LIBRARIES})
#	
	install(TARGETS scope DESTINATION bin)
endif()

# Build the scanner skeleton source.
add_executable(skeleton Skeleton.cpp)
target_link_libraries(skeleton PixieScanStatic)
install (TARGETS skeleton DESTINATION bin)
