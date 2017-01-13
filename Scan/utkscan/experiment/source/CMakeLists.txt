set(EXPERIMENT_SOURCES
        TemplateExpProcessor.cpp
        TwoChanTimingProcessor.cpp
)

add_library(ExperimentObjects OBJECT ${EXPERIMENT_SOURCES})
