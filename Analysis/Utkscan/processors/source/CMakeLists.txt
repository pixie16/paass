# @authors S. V. Paulauskas
set(PROCESSOR_SOURCES
        BatoProcessor.cpp
        BetaScintProcessor.cpp
        DoubleBetaProcessor.cpp
        CloverCalibProcessor.cpp
        CloverFragProcessor.cpp
        CloverProcessor.cpp
        DssdProcessor.cpp
        EventProcessor.cpp
        ExtTSSenderProcessor.cpp
        GammaScintProcessor.cpp
        GammaScintFragProcessor.cpp
        GeProcessor.cpp
        Hen3Processor.cpp
        ImplantSsdProcessor.cpp
        IonChamberProcessor.cpp
        LiquidScintProcessor.cpp
        LitePositionProcessor.cpp
        LogicProcessor.cpp
        MtasImplantSipmProcessor.cpp
        McpProcessor.cpp
        MtasProcessor.cpp
        NeutronScintProcessor.cpp
        PidProcessor.cpp
        PositionProcessor.cpp
        PspmtProcessor.cpp
        RootDevProcessor.cpp
        SingleBetaProcessor.cpp
        TeenyVandleProcessor.cpp
        TemplateProcessor.cpp
        VandleProcessor.cpp
        )

if (PAASS_USE_ROOT)
    list(APPEND PROCESSOR_SOURCES RootProcessor.cpp)
endif (PAASS_USE_ROOT)

add_library(UtkscanProcessorObjects OBJECT ${PROCESSOR_SOURCES})
