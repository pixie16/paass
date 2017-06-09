#@authors K. Smith
set(SET2ROOT_SOURCES set2root.cpp)

add_executable(set2ascii ${SET2ROOT_SOURCES})
target_link_libraries(set2ascii)
install(TARGETS set2ascii DESTINATION bin)

if (PAASS_USE_ROOT)
    add_executable(set2root ${SET2ROOT_SOURCES})
    set_target_properties(set2root PROPERTIES COMPILE_FLAGS "-DUSE_ROOT_OUTPUT")
    target_link_libraries(set2root ${ROOT_LIBRARIES})
    install(TARGETS set2root DESTINATION bin)
endif (PAASS_USE_ROOT)
