add_executable(MCA MCA.cpp HisDrr.cpp)
install(TARGETS MCA DESTINATION ${CMAKE_INSTALL_PREFIX})
target_link_libraries(MCA PixieInterface Utility)
