set(PROCESSOR_SOURCES BetaScintProcessor.cpp DoubleBetaProcessor.cpp EventProcessor.cpp
    GeProcessor.cpp GeCalibProcessor.cpp Hen3Processor.cpp 
    IonChamberProcessor.cpp LiquidScintProcessor.cpp LogicProcessor.cpp 
    NeutronScintProcessor.cpp PositionProcessor.cpp RootProcessor.cpp 
    PulserProcessor.cpp PspmtProcessor.cpp SsdProcessor.cpp TeenyVandleProcessor.cpp 
    TemplateProcessor.cpp VandleProcessor.cpp )

add_library(ProcessorObjects OBJECT ${PROCESSOR_SOURCES})