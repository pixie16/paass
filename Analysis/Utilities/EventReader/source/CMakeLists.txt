# @authors C. R. Thornsberry and S. V. Paulauskas

if (USE_HRIBF)
    add_executable(eventReader eventReader.cpp EventReaderInterface.cpp EventReaderUnpacker.cpp
            $<TARGET_OBJECTS:ScanorObjects>)
    target_link_libraries(eventReader ${HRIBF_LIBRARIES})
else ()
    add_executable(eventReader eventReader.cpp EventReaderInterface.cpp EventReaderUnpacker.cpp)
endif (USE_HRIBF)

target_link_libraries(eventReader PaassScanStatic ${ROOT_LIBRARIES})
install(TARGETS eventReader DESTINATION bin)
