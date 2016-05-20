set(SCANOR_SOURCES 
  set2cc.f 
  messlog.f 
  mildatim.f 
  scanor.f
)

add_library(ScanorObjects OBJECT ${SCANOR_SOURCES})