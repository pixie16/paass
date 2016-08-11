if(USE_ROOT AND BUILD_SCOPE)
	set(SCOPE_SOURCES scope.cpp)
	add_executable(scope ${SCOPE_SOURCES})
	target_link_libraries(scope ScanStatic ${ROOT_LIBRARIES})
    install(TARGETS scope DESTINATION bin)
endif(USE_ROOT AND BUILD_SCOPE)

if(BUILD_SKELETON)
    add_executable(skeleton Skeleton.cpp)
    target_link_libraries(skeleton ScanStatic)
    install (TARGETS skeleton DESTINATION bin)
endif(BUILD_SKELETON)

# Install ldfReader executable.
add_executable(ldfReader ldfReader.cpp)
target_link_libraries(ldfReader ScanStatic)
install (TARGETS ldfReader DESTINATION bin)
