add_subdirectory(scan)
add_subdirectory(core)
add_subdirectory(analyzers)
add_subdirectory(experiment)
add_subdirectory(processors)

add_executable(pixie_ldf_c $<TARGET_OBJECTS:CoreObjects> $<TARGET_OBJECTS:ScanorObjects> $<TARGET_OBJECTS:AnalyzerObjects> $<TARGET_OBJECTS:ExperimentObjects> $<TARGET_OBJECTS:ProcessorObjects>)

target_link_libraries(pixie_ldf_c -lgfortran ${HRIBF_LIBRARIES})
if(USE_ROOT)
    target_link_libraries(pixie_ldf_c ${ROOT_LIBRARIES})
endif(USE_ROOT)

if(USE_GSL)
    target_link_libraries(pixie_ldf_c ${GSL_LIBS})
endif(USE_GSL)

install(TARGETS pixie_ldf_c DESTINATION ${CMAKE_INSTALL_PREFIX})
