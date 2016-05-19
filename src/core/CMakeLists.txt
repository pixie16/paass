set(CORE_SOURCES 
  PixieStd.cpp 
  BarBuilder.cpp 
  Calibrator.cpp 
  ChanEvent.cpp 
  DetectorDriver.cpp
  DetectorLibrary.cpp 
  DetectorSummary.cpp 
  Globals.cpp 
  Identifier.cpp
  Initialize.cpp 
  Messenger.cpp 
  Notebook.cpp 
  RandomPool.cpp 
  RawEvent.cpp 
  StatsData.cpp 
  TimingCalibrator.cpp
  TimingMapBuilder.cpp 
  Trace.cpp 
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

set (READBUFFDATA_SOURCES 
  ReadBuffDataA.cpp 
  ReadBuffDataD.cpp 
  ReadBuffDataF.cpp
)

add_library(CoreObjects OBJECT 
  ${CORE_SOURCES} 
  ${CORRELATION_SOURCES} 
  ${PLOTTING_SOURCES} 
  ${XMLPARSER_SOURCES} 
  ${READBUFFDATA_SOURCES}
)