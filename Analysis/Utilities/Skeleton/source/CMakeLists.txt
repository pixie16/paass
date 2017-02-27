if (USE_HRIBF)
    add_executable(skeleton Skeleton.cpp $<TARGET_OBJECTS:ScanorObjects>)
    target_link_libraries(skeleton ${HRIBF_LIBRARIES})
else ()
    add_executable(skeleton Skeleton.cpp)
endif (USE_HRIBF)

target_link_libraries(skeleton PaassScanStatic)
install(TARGETS skeleton DESTINATION bin)
