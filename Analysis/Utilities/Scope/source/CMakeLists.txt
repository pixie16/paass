# @authors C. R. Thornsberry, K. Smith, S. V. Paulauskas
if (PAASS_USE_HRIBF)
    set(CMAKE_Fortran_FLAGS ${CMAKE_Fortran_FLAGS} "-Wl,-no_compact_unwinding")
    add_executable(scope scope.cpp ScopeUnpacker.cpp ScopeScanner.cpp $<TARGET_OBJECTS:ScanorObjects>)
    target_link_libraries(scope ${HRIBF_LIBRARIES})
else ()
    add_executable(scope scope.cpp ScopeUnpacker.cpp ScopeScanner.cpp)
endif (PAASS_USE_HRIBF)

target_link_libraries(scope PaassResourceStatic PaassScanStatic PugixmlStatic ResourceStatic ${ROOT_LIBRARIES})
install(TARGETS scope DESTINATION bin)