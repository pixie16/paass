set(CORE_SOURCES
        BarBuilder.cpp
        Calibrator.cpp
        ChanEvent.cpp
        DetectorDriver.cpp
        DetectorLibrary.cpp
        DetectorSummary.cpp
        Globals.cpp
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

set (CORRELATION_SOURCES
        PlaceBuilder.cpp
        Places.cpp
        TreeCorrelator.cpp
)

set (PLOTTING_SOURCES
        Plots.cpp
        PlotsRegister.cpp
)

if(NOT USE_HRIBF)
    set(MAIN_SOURCES utkscan.cpp HisFile.cpp)
else(USE_HRIBF)
    set(MAIN_SOURCES utkscanor.cpp)
endif(NOT USE_HRIBF)

add_library(MainObjects OBJECT ${MAIN_SOURCES})

add_library(CoreObjects OBJECT
    ${CORE_SOURCES}
    ${CORRELATION_SOURCES}
    ${PLOTTING_SOURCES}
)
