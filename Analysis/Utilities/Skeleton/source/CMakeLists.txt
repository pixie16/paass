# @authors C. R. Thornsberry, S. V. Paulauskas
if (PAASS_USE_HRIBF)
    set(CMAKE_Fortran_FLAGS ${CMAKE_Fortran_FLAGS} "-Wl,-no_compact_unwinding")
    add_executable(skeleton Skeleton.cpp SkeletonUnpacker.cpp SkeletonInterface.cpp $<TARGET_OBJECTS:ScanorObjects>)
    target_link_libraries(skeleton ${HRIBF_LIBRARIES})
else ()
    add_executable(skeleton Skeleton.cpp SkeletonUnpacker.cpp SkeletonInterface.cpp)
endif (PAASS_USE_HRIBF)

target_link_libraries(skeleton PaassScanStatic PugixmlStatic PaassResourceStatic)
install(TARGETS skeleton DESTINATION bin)
