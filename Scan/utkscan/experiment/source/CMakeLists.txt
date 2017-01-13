set(EXPERIMENT_SOURCES
        TemplateExpProcessor.cpp
)

if(USE_ROOT)
list(APPEND EXPERIMENT_SOURCES
        IS600Processor.cpp
        TwoChanTimingProcessor.cpp)
endif(USE_ROOT)

add_library(ExperimentObjects OBJECT ${EXPERIMENT_SOURCES})
