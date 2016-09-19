#We need fPIC below to create shared libraries.
set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fsecond-underscore -fPIC")

set(SCANOR_SOURCES 
  set2cc.f messlog.f mildatim.f scanor.f
  ScanorInterface.cpp
)

add_library(ScanorObjects OBJECT ${SCANOR_SOURCES})

if(BUILD_SHARED_LIBS)
    add_library(ScanHHIRF SHARED $<TARGET_OBJECTS:ScanorObjects>)
    install(TARGETS ScanHHIRF DESTINATION lib)
endif(BUILD_SHARED_LIBS)
