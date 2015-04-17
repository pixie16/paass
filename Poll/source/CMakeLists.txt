set(POLL_SOURCES poll.cpp sendbuf.c spkt_io_udp.c StatsHandler.cpp)
add_executable(poll ${POLL_SOURCES})
target_link_libraries(poll PixieInterface Utility)

set(POLL2_SOURCES poll2.cpp poll2_core.cpp hribf_buffers.cpp CTerminal.cpp poll2_socket.cpp StatsHandler.cpp)
add_executable(poll2 ${POLL2_SOURCES})
target_link_libraries(poll2 PixieInterface Utility MCA_LIBRARY ${CMAKE_THREAD_LIBS_INIT}
${CURSES_LIBRARIES})

set(PULSER_SOURCES pulser.cpp sendbuf.c spkt_io_udp.c)
add_executable(pulser ${PULSER_SOURCES})
target_link_libraries(pulser PixieInterface Utility)

set(COMTEST_SOURCES commtest.cpp sendbuf.c spkt_io_udp.c)
add_executable(commtest ${COMTEST_SOURCES})

install(TARGETS poll poll2 pulser commtest DESTINATION ${CMAKE_INSTALL_PREFIX})
