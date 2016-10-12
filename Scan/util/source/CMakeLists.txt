if(USE_ROOT AND BUILD_SCOPE)
	if(USE_HRIBF)
		add_executable(scope scope.cpp $<TARGET_OBJECTS:ScanorObjects>)
		target_link_libraries(scope ${HRIBF_LIBRARIES})
	else()
		add_executable(scope scope.cpp)
	endif()

	target_link_libraries(scope ScanStatic ${ROOT_LIBRARIES})
   install(TARGETS scope DESTINATION bin)
endif(USE_ROOT AND BUILD_SCOPE)

if(BUILD_SKELETON)
	if(USE_HRIBF)
		add_executable(skeleton Skeleton.cpp $<TARGET_OBJECTS:ScanorObjects>)
		target_link_libraries(skeleton ${HRIBF_LIBRARIES})
	else()
	    add_executable(skeleton Skeleton.cpp)
	endif()

   target_link_libraries(skeleton ScanStatic)
   install (TARGETS skeleton DESTINATION bin)
endif(BUILD_SKELETON)

# Install headReader executable.
add_executable(headReader headReader.cpp)
target_link_libraries(headReader ScanStatic)
install (TARGETS headReader DESTINATION bin)
