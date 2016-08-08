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
  if(${GSL_VERSION} GREATER 1.9)
      set(ANALYZER_SOURCES ${ANALYZER_SOURCES} Gsl2Fitter.cpp)
  else(${GSL_VERSION} LESS 2.0)
      set(ANALYZER_SOURCES ${ANALYZER_SOURCES} Gsl1Fitter.cpp)
  endif(${GSL_VERSION} GREATER 1.9)
endif(USE_GSL)

add_library(AnalyzerObjects OBJECT ${ANALYZER_SOURCES})



