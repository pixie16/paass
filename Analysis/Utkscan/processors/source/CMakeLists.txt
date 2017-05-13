set(PROCESSOR_SOURCES
        #  BetaProcessor.cpp
        BetaScintProcessor.cpp
        DoubleBetaProcessor.cpp
        CloverCalibProcessor.cpp
        CloverProcessor.cpp
        #  DssdProcessor.cpp
        EventProcessor.cpp
        GeProcessor.cpp
        Hen3Processor.cpp
        #  ImplantSsdProcessor.cpp
        IonChamberProcessor.cpp
        #  LiquidProcessor.cpp
        LiquidScintProcessor.cpp
        #  LitePositionProcessor.cpp
        LogicProcessor.cpp
        #  McpProcessor.cpp
        #  NeutronProcessor.cpp
        NeutronScintProcessor.cpp
        PositionProcessor.cpp
        PspmtProcessor.cpp
        #  ScintProcessor.cpp
        #  SsdBetaProcessor.cpp
        SsdProcessor.cpp
        TeenyVandleProcessor.cpp
        TemplateProcessor.cpp
        #  ValidProcessor.cpp
        VandleProcessor.cpp
        )

if (PAASS_USE_ROOT)
    list(APPEND PROCESSOR_SOURCES RootProcessor.cpp)
endif (PAASS_USE_ROOT)

add_library(UtkscanProcessorObjects OBJECT ${PROCESSOR_SOURCES})