# @authors C. R. Thornsberry, K. Smith
if (PAASS_USE_NCURSES)
    set(POLL2_SOURCES poll2.cpp poll2_core.cpp poll2_stats.cpp)
    add_executable(poll2 ${POLL2_SOURCES})
    target_link_libraries(poll2 PixieInterface PixieSupport Utility MCA_LIBRARY ${CMAKE_THREAD_LIBS_INIT})
    install(TARGETS poll2 DESTINATION bin)
else ()
    message(WARNING "Cannot build poll2 without ncurses!")
endif (PAASS_USE_NCURSES)

set(LISTENER_SOURCES listener.cpp)
add_executable(listener ${LISTENER_SOURCES})
target_link_libraries(listener PaassCoreStatic)

set(MONITOR_SOURCES original_monitor.cpp)
add_executable(monitor ${MONITOR_SOURCES})
target_link_libraries(monitor PaassCoreStatic)

set(SUBMONITORS_SOURCES submonitors.cpp monitor.cpp)
add_executable(submonitors ${SUBMONITORS_SOURCES})
target_link_libraries(submonitors PaassCoreStatic)

set(MAIN_MONITOR_SOURCES mainmonitor.cpp  monitor.cpp)
add_executable(mainmonitor ${MAIN_MONITOR_SOURCES})
target_link_libraries(mainmonitor PaassCoreStatic)

# set(EXEC_TEST exec_test.cpp  monitor.cpp)
# add_executable(exec_test ${EXEC_TEST})
# # target_link_libraries(exec_test PaassCoreStatic)

install(TARGETS listener monitor submonitors mainmonitor DESTINATION bin)
