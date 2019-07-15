#@authors K. Smith
include_directories(include)
add_subdirectory(source)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/send_alarm
    PERMISSIONS OWNER_EXECUTE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
    DESTINATION bin)