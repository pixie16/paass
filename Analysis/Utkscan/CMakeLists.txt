# @author S. V. Paulauskas
if (NOT PAASS_USE_GSL)
    message(FATAL_ERROR "GSL is required to build Utkscan!")
endif (NOT PAASS_USE_GSL)

#CMake file for UTKScan.
option(PAASS_UTKSCAN_GAMMA_GATES "Gamma-Gamma gates in GeProcessor" OFF)
option(PAASS_UTKSCAN_ONLINE "Options for online scans" OFF)
option(PAASS_UTKSCAN_TREE_DEBUG "Debugging info for TreeCorrelator" OFF)
option(PAASS_UTKSCAN_VERBOSE "Make Scan More Verbose" OFF)

# newreadout is needed to account for a change to pixie16 readout
# structure change on 03/20/08. Is is REQUIRED!!
add_definitions(-D newreadout)

#utkscan will have Gamma-Gamma gating in the GeProcessor
if (PAASS_UTKSCAN_GAMMA_GATES)
    add_definitions(-D GGATES)
endif (PAASS_UTKSCAN_GAMMA_GATES)

#utkscan will be streamlined for online processing
if (PAASS_UTKSCAN_ONLINE)
    add_definitions(-D ONLINE)
endif (PAASS_UTKSCAN_ONLINE)

#utkscan will have debugging for the TreeCorrelator
if (PAASS_UTKSCAN_TREE_DEBUG)
    add_definitions(-D TREE_DEBUG)
endif (PAASS_UTKSCAN_TREE_DEBUG)

#utkscan will be more verbose in its output
if (PAASS_UTKSCAN_VERBOSE)
    add_definitions(-D VERBOSE)
endif (PAASS_UTKSCAN_VERBOSE)

#------------------------------------------------------------------------------

include_directories(analyzers/include core/include experiment/include processors/include)
add_subdirectory(analyzers)
add_subdirectory(core)
add_subdirectory(experiment)
add_subdirectory(processors)

#------------------------------------------------------------------------------

if (NOT PAASS_USE_HRIBF)
    set(SCAN_NAME utkscan)
    add_executable(${SCAN_NAME}
            $<TARGET_OBJECTS:UtkscanMainObjects>
            $<TARGET_OBJECTS:UtkscanCoreObjects>
            $<TARGET_OBJECTS:UtkscanAnalyzerObjects>
            $<TARGET_OBJECTS:UtkscanProcessorObjects>
            $<TARGET_OBJECTS:UtkscanExperimentObjects>)
else (PAASS_USE_HRIBF)
    set(SCAN_NAME utkscanor)
    add_executable(${SCAN_NAME}
            $<TARGET_OBJECTS:UtkscanMainObjects>
            $<TARGET_OBJECTS:UtkscanCoreObjects>
            $<TARGET_OBJECTS:UtkscanAnalyzerObjects>
            $<TARGET_OBJECTS:UtkscanProcessorObjects>
            $<TARGET_OBJECTS:UtkscanExperimentObjects>
            $<TARGET_OBJECTS:ScanorObjects>)
    target_link_libraries(utkscanor ${HRIBF_LIBRARIES})
endif (NOT PAASS_USE_HRIBF)

target_link_libraries(${SCAN_NAME} ${LIBS} PaassScanStatic ResourceStatic PaassCoreStatic PugixmlStatic PaassResourceStatic)

if (PAASS_USE_GSL)
    target_link_libraries(${SCAN_NAME} ${GSL_LIBRARIES})
endif (PAASS_USE_GSL)

if (PAASS_USE_ROOT)
    target_link_libraries(${SCAN_NAME} ${ROOT_LIBRARIES})
endif (PAASS_USE_ROOT)

#------------------------------------------------------------------------------

install(TARGETS ${SCAN_NAME} DESTINATION bin)
install(DIRECTORY share/utkscan DESTINATION share)