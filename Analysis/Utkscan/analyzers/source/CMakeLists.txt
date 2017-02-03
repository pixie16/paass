set(UTKSCAN_ANALYZER_SOURCES
        CfdAnalyzer.cpp
        TauAnalyzer.cpp
        TraceExtractor.cpp
        TraceFilter.cpp
        TraceFilterAnalyzer.cpp
        TraceAnalyzer.cpp
        WaaAnalyzer.cpp
        WaveformAnalyzer.cpp)

if(USE_GSL)
    list(APPEND UTKSCAN_ANALYZER_SOURCES FittingAnalyzer.cpp)
endif(USE_GSL)

add_library(UtkscanAnalyzerObjects OBJECT ${UTKSCAN_ANALYZER_SOURCES})



