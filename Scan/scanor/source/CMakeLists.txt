set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fsecond-underscore")

set(SCANOR_SOURCES 
  set2cc.f 
  messlog.f 
  mildatim.f 
  scanor.f
  ScanorInterface.cpp
)

add_library(ScanorObjects OBJECT ${SCANOR_SOURCES})
