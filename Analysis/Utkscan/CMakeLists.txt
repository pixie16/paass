# @author S. V. Paulauskas
if (NOT PAASS_USE_GSL)
    message(FATAL_ERROR "GSL is required to build Utkscan!")
endif (NOT PAASS_USE_GSL)

#CMake file for UTKScan.
option(PAASS_UTKSCAN_GAMMA_GATES "Gamma-Gamma gates in the Gamma-Ray Processors" OFF)
option(PAASS_UTKSCAN_ONLINE "Options for online scans" OFF)
option(PAASS_UTKSCAN_TREE_DEBUG "Debugging info for TreeCorrelator" OFF)
option(PAASS_UTKSCAN_VERBOSE "Make Scan More Verbose" OFF)
option(PAASS_OVERRIDE_EXE_PREFIX "Override the scan exe prefix" OFF)

mark_as_advanced(PAASS_UTKSCAN_GAMMA_GATES)
mark_as_advanced(PAASS_UTKSCAN_ONLINE)
mark_as_advanced(PAASS_UTKSCAN_TREE_DEBUG)
mark_as_advanced(PAASS_UTKSCAN_VERBOSE)
mark_as_advanced(PAASS_OVERRIDE_EXE_PREFIX)

# newreadout is needed to account for a change to pixie16 readout
# structure change on 03/20/08. Is is REQUIRED!!
add_definitions(-D newreadout)

#utkscan will have Gamma-Gamma gating in the CloverProcessor and GammaScintProcessor
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

if (PAASS_OVERRIDE_EXE_PREFIX)
    ##  Overrid exe name
    set(PAASS_SCANNAME_PREFIX utk CACHE STRING "Set Prefix name for scan")
    set(SCANOR_NAME "${PAASS_SCANNAME_PREFIX}scanor")
    set(SCAN_NAME "${PAASS_SCANNAME_PREFIX}scan")
else (PAASS_OVERRIDE_EXE_PREFIX)
    set(SCANOR_NAME "utkscanor")
    set(SCAN_NAME "utkscan")
endif (PAASS_OVERRIDE_EXE_PREFIX)

#------------------------------------------------------------------------------

include_directories(analyzers/include core/include experiment/include processors/include)
add_subdirectory(analyzers)
add_subdirectory(core)
add_subdirectory(experiment)
add_subdirectory(processors)

#added by T.T. King for Vector Root Libs
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_SKIP_INSTALL_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

ROOT_GENERATE_DICTIONARY(PaassRootStruct PaassRootStruct.hpp LINKDEF core/include/PaassRootLinkDef.hpp )
add_library(PaassRootStruct SHARED PaassRootStruct.cxx PaassRootStruct)

#End T.T. King Additions
#------------------------------------------------------------------------------

if (NOT PAASS_USE_HRIBF)
    set(SCAN_NAME ${SCAN_NAME})
    add_executable(${SCAN_NAME}
            $<TARGET_OBJECTS:UtkscanMainObjects>
            $<TARGET_OBJECTS:UtkscanCoreObjects>
            $<TARGET_OBJECTS:UtkscanAnalyzerObjects>
            $<TARGET_OBJECTS:UtkscanProcessorObjects>
            $<TARGET_OBJECTS:UtkscanExperimentObjects>
            PaassRootStruct.cxx)
else (PAASS_USE_HRIBF)
    set(SCAN_NAME ${SCANOR_NAME})
    add_executable(${SCAN_NAME}
            $<TARGET_OBJECTS:UtkscanMainObjects>
            $<TARGET_OBJECTS:UtkscanCoreObjects>
            $<TARGET_OBJECTS:UtkscanAnalyzerObjects>
            $<TARGET_OBJECTS:UtkscanProcessorObjects>
            $<TARGET_OBJECTS:UtkscanExperimentObjects>
            $<TARGET_OBJECTS:ScanorObjects>
            PaassRootStruct.cxx)
    target_link_libraries(${SCANOR_NAME} ${HRIBF_LIBRARIES})
endif (NOT PAASS_USE_HRIBF)

target_link_libraries(${SCAN_NAME} ${LIBS} PaassRootStruct PaassScanStatic ResourceStatic PaassCoreStatic PugixmlStatic PaassResourceStatic )

if (PAASS_USE_GSL)
    target_link_libraries(${SCAN_NAME} ${GSL_LIBRARIES})
endif (PAASS_USE_GSL)

if (PAASS_USE_ROOT)
    target_link_libraries(${SCAN_NAME} ${ROOT_LIBRARIES} PaassRootStruct)
endif (PAASS_USE_ROOT)

#------------------------------------------------------------------------------
if( PAASS_OVERRIDE_EXE_PREFIX )
	add_custom_command( TARGET ${SCAN_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan --bold "EXECUTABLE NAMES HAVE BEEN CHANGED FROM DEFAULT")
endif( PAASS_OVERRIDE_EXE_PREFIX )

install(TARGETS ${SCAN_NAME} DESTINATION bin)
install(DIRECTORY share/utkscan DESTINATION share)
install(TARGETS PaassRootStruct DESTINATION lib)
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/lib/PaassRootStruct_rdict.pcm ${CMAKE_INSTALL_PREFIX}/bin/PaassRootStruct_rdict.pcm)")

install(FILES ${CMAKE_SOURCE_DIR}/Analysis/Utkscan/core/include/PaassRootStruct.hpp DESTINATION include/)
install(FILES ${CMAKE_SOURCE_DIR}/Analysis/Utkscan/processors/include/GSaddback.hpp DESTINATION include/)
#Install the ROOT 6 PCM.
if (${ROOT_VERSION} VERSION_GREATER "6.0")
    install( FILES ${CMAKE_CURRENT_BINARY_DIR}/PaassRootStruct_rdict.pcm DESTINATION lib/)
endif (${ROOT_VERSION} VERSION_GREATER "6.0")
