#We need fPIC below to create shared libraries.
set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fsecond-underscore -fPIC")

set(SCANOR_SOURCES 
  set2cc.f 
  messlog.f 
  mildatim.f 
  scanor.f
  ScanorInterface.cpp
)

add_library(ScanorObjects OBJECT ${SCANOR_SOURCES})

# Kind of messy, but I didn't know how else to do this. CRT
install(FILES ../include/ScanorInterface.hpp DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

if(BUILD_SHARED_LIBS)
    message(STATUS "Building Scanor Shared Objects")
    add_library(ScanHHIRF SHARED $<TARGET_OBJECTS:ScanorObjects>)
    target_link_libraries(ScanHHIRF ${HRIBF_LIBRARIES})
    install(TARGETS ScanHHIRF DESTINATION lib)
endif(BUILD_SHARED_LIBS)
