set(PROCESSOR_SOURCES 
#  BetaProcessor.cpp
  BetaScintProcessor.cpp 
  DoubleBetaProcessor.cpp 
#  DssdProcessor.cpp
  EventProcessor.cpp
  GeCalibProcessor.cpp 
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
  PulserProcessor.cpp 
#  ScintProcessor.cpp
#  SsdBetaProcessor.cpp
  SsdProcessor.cpp 
  TeenyVandleProcessor.cpp 
  TemplateProcessor.cpp 
#  ValidProcessor.cpp
  VandleProcessor.cpp
)
if(USE_ROOT)
set(PROCESSOR_SOURCES ${PROCESSOR_SOURCES} RootProcessor.cpp)
endif(USE_ROOT)

add_library(ProcessorObjects OBJECT ${PROCESSOR_SOURCES})