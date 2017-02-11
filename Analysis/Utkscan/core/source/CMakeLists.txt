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
        UtkScanInterface.cpp
        UtkUnpacker.cpp
        WalkCorrector.cpp
        XmlParser.cpp
)

set(CORRELATION_SOURCES
        PlaceBuilder.cpp
        Places.cpp
        TreeCorrelator.cpp)

set(PLOTTING_SOURCES
        Plots.cpp
        PlotsRegister.cpp)

if (NOT PAASS_USE_HRIBF)
    set(MAIN_SOURCES utkscan.cpp HisFile.cpp)
else (PAASS_USE_HRIBF)
    set(MAIN_SOURCES utkscanor.cpp)
endif (NOT PAASS_USE_HRIBF)

add_library(UtkscanMainObjects OBJECT ${MAIN_SOURCES})

add_library(UtkscanCoreObjects OBJECT ${CORE_SOURCES} ${CORRELATION_SOURCES}
    ${PLOTTING_SOURCES}
)
