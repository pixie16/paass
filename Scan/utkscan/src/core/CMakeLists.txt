set(CORE_SOURCES 
  BarBuilder.cpp 
  Calibrator.cpp 
  ChanEvent.cpp 
  DetectorDriver.cpp
  DetectorLibrary.cpp 
  DetectorSummary.cpp 
  Globals.cpp 
  HisFile.cpp
  Identifier.cpp
  Messenger.cpp 
  Notebook.cpp 
  RandomPool.cpp 
  RawEvent.cpp 
#  StatsData.cpp 
  TimingCalibrator.cpp
  TimingMapBuilder.cpp 
  Trace.cpp 
  utkscan.cpp
  UtkScanInterface.cpp
  UtkUnpacker.cpp
  WalkCorrector.cpp
)

set (CORRELATION_SOURCES 
  PlaceBuilder.cpp 
  Places.cpp 
  TreeCorrelator.cpp
)

set (PLOTTING_SOURCES 
  Plots.cpp 
  PlotsRegister.cpp
)

set (XMLPARSER_SOURCES 
  pugixml.cpp
)

add_library(CoreObjects OBJECT 
  ${CORE_SOURCES} 
  ${CORRELATION_SOURCES} 
  ${PLOTTING_SOURCES} 
  ${XMLPARSER_SOURCES} 
)
