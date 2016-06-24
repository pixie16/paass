add_subdirectory(core)
add_subdirectory(analyzers)
add_subdirectory(processors)
add_subdirectory(experiment)

add_executable(utkscan 
  $<TARGET_OBJECTS:CoreObjects> 
  $<TARGET_OBJECTS:AnalyzerObjects> 
  $<TARGET_OBJECTS:ProcessorObjects>
  $<TARGET_OBJECTS:ExperimentObjects>)

target_link_libraries(utkscan ${LIBS})

if(USE_ROOT)
    target_link_libraries(utkscan ${ROOT_LIBRARIES})
endif(USE_ROOT)

if(USE_GSL)
    target_link_libraries(utkscan ${GSL_LIBRARIES})
endif(USE_GSL)

install(TARGETS utkscan DESTINATION ${CMAKE_INSTALL_PREFIX})
