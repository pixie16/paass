add_subdirectory(scan)
add_subdirectory(core)
add_subdirectory(analyzers)
add_subdirectory(processors)
add_subdirectory(experiment)

add_executable(pixie_ldf_c 
  $<TARGET_OBJECTS:ScanorObjects> 
  $<TARGET_OBJECTS:CoreObjects> 
  $<TARGET_OBJECTS:AnalyzerObjects> 
  $<TARGET_OBJECTS:ProcessorObjects>
  $<TARGET_OBJECTS:ExperimentObjects> )

set_target_properties(pixie_ldf_c PROPERTIES LINKER_LANGUAGE Fortran)
target_link_libraries(pixie_ldf_c ${HRIBF_LIBRARIES})

if(USE_GFORTRAN)
  target_link_libraries(pixie_ldf_c -lgfortran)
else(USE_G77)
  target_link_libraries(pixie_ldf_c -lg2c -g77libs)
endif(USE_GFORTRAN)

if(USE_ROOT)
    target_link_libraries(pixie_ldf_c ${ROOT_LIBRARIES})
endif(USE_ROOT)

if(USE_GSL)
    target_link_libraries(pixie_ldf_c ${LIBS})
endif(USE_GSL)

install(TARGETS pixie_ldf_c DESTINATION ${CMAKE_INSTALL_PREFIX})
