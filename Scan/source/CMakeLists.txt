set(PixieCore_SOURCES XiaData.cpp Unpacker.cpp ScanInterface.cpp)

add_library(PixieScanObjects OBJECT ${PixieCore_SOURCES})

add_library(PixieScan SHARED $<TARGET_OBJECTS:PixieScanObjects>)
target_link_libraries(PixieScan PixieCoreStatic ${CMAKE_THREAD_LIBS_INIT})

add_library(PixieScanStatic STATIC $<TARGET_OBJECTS:PixieScanObjects>)
target_link_libraries(PixieScanStatic PixieCoreStatic ${CMAKE_THREAD_LIBS_INIT})

if (${CURSES_FOUND})
	target_link_libraries(PixieScan ${CURSES_LIBRARIES})
	target_link_libraries(PixieScanStatic ${CURSES_LIBRARIES})
endif()

install(TARGETS PixieScan DESTINATION lib)

# Build the scanner skeleton source.
add_executable(skeleton Skeleton.cpp)
target_link_libraries(skeleton PixieScanStatic)
install (TARGETS skeleton DESTINATION bin)

#Build the scope program and the filterer program
if(USE_ROOT)
	set(SCOPE_SOURCES scope.cpp)
	add_executable(scope ${SCOPE_SOURCES})
	target_link_libraries(scope PixieScanStatic ${ROOT_LIBRARIES})
#
#	add_executable(filterer filterer.cpp)
#	target_link_libraries(filterer PixieScanStatic ${ROOT_LIBRARIES})
#	
	install(TARGETS scope DESTINATION bin)
endif()
