set(ANALYZER_SOURCES 
  CfdAnalyzer.cpp 
  FittingAnalyzer.cpp 
  TauAnalyzer.cpp 
  TraceExtractor.cpp
  TraceFilter.cpp
  TraceFilterAnalyzer.cpp 
  TraceAnalyzer.cpp 
  WaaAnalyzer.cpp 
  WaveformAnalyzer.cpp
)

add_library(AnalyzerObjects OBJECT ${ANALYZER_SOURCES})