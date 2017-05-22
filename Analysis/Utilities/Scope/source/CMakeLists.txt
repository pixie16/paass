if (PAASS_USE_HRIBF)
    add_executable(scope scope.cpp ScopeUnpacker.cpp ScopeScanner.cpp
            $<TARGET_OBJECTS:ScanorObjects>)
    target_link_libraries(scope ${HRIBF_LIBRARIES})
else ()
    add_executable(scope scope.cpp ScopeUnpacker.cpp ScopeScanner.cpp)
endif (PAASS_USE_HRIBF)

target_link_libraries(scope ResourceStatic PaassResourceStatic PaassScanStatic
        ${ROOT_LIBRARIES})
install(TARGETS scope DESTINATION bin)