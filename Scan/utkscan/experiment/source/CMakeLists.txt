set(EXPERIMENT_SOURCES
        TemplateExpProcessor.cpp
        VandleOrnl2012Processor.cpp
)

if(USE_ROOT)
list(APPEND EXPERIMENT_SOURCES
        Anl1471Processor.cpp
        IS600Processor.cpp
        TwoChanTimingProcessor.cpp)
endif(USE_ROOT)

add_library(ExperimentObjects OBJECT ${EXPERIMENT_SOURCES})
