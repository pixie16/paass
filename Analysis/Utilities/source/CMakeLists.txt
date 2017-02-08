if(PAASS_USE_ROOT AND PAASS_BUILD_SCOPE)
	if(PAASS_USE_HRIBF)
		add_executable(scope scope.cpp $<TARGET_OBJECTS:ScanorObjects>)
		target_link_libraries(scope ${HRIBF_LIBRARIES})
	else()
		add_executable(scope scope.cpp)
	endif(PAASS_USE_HRIBF)

	target_link_libraries(scope PaassScanStatic ${ROOT_LIBRARIES})
   install(TARGETS scope DESTINATION bin)
endif(PAASS_USE_ROOT AND PAASS_BUILD_SCOPE)

if(PAASS_BUILD_SKELETON)
	if(USE_HRIBF)
		add_executable(skeleton Skeleton.cpp $<TARGET_OBJECTS:ScanorObjects>)
		target_link_libraries(skeleton ${HRIBF_LIBRARIES})
	else()
	    add_executable(skeleton Skeleton.cpp)
	endif(USE_HRIBF)

   target_link_libraries(skeleton PaassScanStatic)
   install (TARGETS skeleton DESTINATION bin)
endif(PAASS_BUILD_SKELETON)

# Install headReader executable.
add_executable(headReader headReader.cpp)
target_link_libraries(headReader PaassScanStatic)
install (TARGETS headReader DESTINATION bin)
