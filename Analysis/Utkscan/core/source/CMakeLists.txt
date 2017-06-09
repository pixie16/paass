# @author S. V. Paulauskas
set(CORE_SOURCES
        BarBuilder.cpp
        Calibrator.cpp
        DetectorDriver.cpp
        DetectorDriverXmlParser.cpp
        DetectorLibrary.cpp
        DetectorSummary.cpp
        Globals.cpp
        GlobalsXmlParser.cpp
        MapNodeXmlParser.cpp
        RawEvent.cpp
        TimingCalibrator.cpp
        TimingMapBuilder.cpp
        UtkScanInterface.cpp
        UtkUnpacker.cpp
        WalkCorrector.cpp)

set(CORRELATION_SOURCES
        Correlator.cpp
        PlaceBuilder.cpp
        Places.cpp
        TreeCorrelator.cpp
        TreeCorrelatorXmlParser.cpp)

set(PLOTTING_SOURCES
        Plots.cpp
        PlotsRegister.cpp)

if (NOT PAASS_USE_HRIBF)
    set(MAIN_SOURCES utkscan.cpp HisFile.cpp)
else (PAASS_USE_HRIBF)
    set(MAIN_SOURCES utkscanor.cpp)
endif (NOT PAASS_USE_HRIBF)

add_library(UtkscanMainObjects OBJECT ${MAIN_SOURCES})

add_library(UtkscanCoreObjects OBJECT ${CORE_SOURCES} ${CORRELATION_SOURCES} ${PLOTTING_SOURCES})
