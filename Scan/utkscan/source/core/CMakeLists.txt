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
        UtkScanInterface.cpp
        UtkUnpacker.cpp
        WalkCorrector.cpp
)

if(NOT USE_HRIBF)
    set(CORE_SOURCES ${CORE_SOURCES} utkscan.cpp)
else(USE_HRIBF)
    set(CORE_SOURCES ${CORE_SOURCES} utkscanor.cpp)
endif(NOT USE_HRIBF)

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
