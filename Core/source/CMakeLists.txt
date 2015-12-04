set(PixieCore_SOURCES Display.cpp hribf_buffers.cpp poll2_socket.cpp)
if (${CURSES_FOUND})
	list(APPEND PixieCore_SOURCES CTerminal.cpp)
endif()

add_library(PixieCoreObjects OBJECT ${PixieCore_SOURCES})

add_library(PixieCore SHARED $<TARGET_OBJECTS:PixieCoreObjects>)

add_library(PixieCoreStatic STATIC $<TARGET_OBJECTS:PixieCoreObjects>)


if (${CURSES_FOUND})
	target_link_libraries(PixieCore ${CURSES_LIBRARIES})
	target_link_libraries(PixieCoreStatic ${CURSES_LIBRARIES})
endif()

install(TARGETS PixieCore DESTINATION lib)
