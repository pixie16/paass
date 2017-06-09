#@authors S. V. Paulauskas
set(EXPERIMENT_SOURCES
        E11027Processor.cpp
        TemplateExpProcessor.cpp
        VandleOrnl2012Processor.cpp
        )

if (PAASS_USE_ROOT)
    list(APPEND EXPERIMENT_SOURCES
            Anl1471Processor.cpp
            IS600Processor.cpp
            TwoChanTimingProcessor.cpp
            )
endif (PAASS_USE_ROOT)

add_library(UtkscanExperimentObjects OBJECT ${EXPERIMENT_SOURCES})
