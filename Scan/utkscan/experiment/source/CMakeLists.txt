set(EXPERIMENT_SOURCES
        IS600Processor.cpp
        TemplateExpProcessor.cpp
        TwoChanTimingProcessor.cpp
)

add_library(ExperimentObjects OBJECT ${EXPERIMENT_SOURCES})
