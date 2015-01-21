set(POLL_SOURCES poll.cpp sendbuf.c spkt_io_udp.c StatsHandler.cpp)
add_executable(poll ${POLL_SOURCES})
target_link_libraries(poll PixieInterface Utility)

set(PULSER_SOURCES pulser.cpp sendbuf.c spkt_io_udp.c)
add_executable(pulser ${PULSER_SOURCES})
target_link_libraries(pulser PixieInterface Utility)

set(COMTEST_SOURCES commtest.cpp sendbuf.c spkt_io_udp.c)
add_executable(commtest ${COMTEST_SOURCES})

install(TARGETS poll pulser commtest DESTINATION ${CMAKE_INSTALL_PREFIX})
