set(ANALYZER_SOURCES
        CfdAnalyzer.cpp
        TauAnalyzer.cpp
        TraceExtractor.cpp
        TraceFilter.cpp
        TraceFilterAnalyzer.cpp
        TraceAnalyzer.cpp
        WaaAnalyzer.cpp
        WaveformAnalyzer.cpp
  )

if(USE_GSL)
  set(ANALYZER_SOURCES ${ANALYZER_SOURCES} FittingAnalyzer.cpp)
  if(GSL_VERSION VERSION_GREATER 2.0)
      set(ANALYZER_SOURCES ${ANALYZER_SOURCES} Gsl2Fitter.cpp)
  else()
      set(ANALYZER_SOURCES ${ANALYZER_SOURCES} Gsl1Fitter.cpp)
  endif(GSL_VERSION VERSION_GREATER 2.0)
endif(USE_GSL)

add_library(AnalyzerObjects OBJECT ${ANALYZER_SOURCES})



