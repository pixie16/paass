# Find the XIA Library
#
# Sets the usual variables expected for find_package scripts:
#
# XIA_LIBRARY_DIR
# XIA_FOUND - true if XIA was found.
#
# @authors K. Smith, C. R. Thornsberry, S. V. Paulauskas

#Unset any cached value to ensure a fresh search is performed. 
#This permits the user to change the XIA_ROOT_DIR and have subsequent paths updated.
unset(XIA_LIBRARY_DIR CACHE)

#Find the library path by looking for the library.
find_path(XIA_LIBRARY_DIR
        NAMES libPixie16App.a libPixie16Sys.a
        HINTS ${XIA_ROOT_DIR}
        PATHS /opt/xia/current /opt/xia/software
        PATH_SUFFIXES software
        DOC "Path to pixie library.")

get_filename_component(XIA_LIBRARY_DIR "${XIA_LIBRARY_DIR}" REALPATH)

if (NOT XIA_FIRMWARE_DIR)
    get_filename_component(XIA_FIRMWARE_DIR "${XIA_LIBRARY_DIR}/../.." REALPATH)
endif (NOT XIA_FIRMWARE_DIR)
set(XIA_FIRMWARE_DIR ${XIA_FIRMWARE_DIR} CACHE PATH "Path to folder containing XIA firmware.")

#The order is strange here as we are really interested in the libraries first
# then we determine the root directory from there.

# Support the REQUIRED and QUIET arguments, and set XIA_FOUND if found.
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(XIA DEFAULT_MSG XIA_LIBRARY_DIR)

if (XIA_FOUND)
    set(XIA_INCLUDE_DIR ${XIA_LIBRARY_DIR}/inc ${XIA_LIBRARY_DIR}/sys ${XIA_LIBRARY_DIR}/app)
    set(XIA_LIBRARIES -lPixie16App -lPixie16Sys)
endif ()

function(XIA_CONFIG)
    message(STATUS "Creating Pixie configuration.")

    get_filename_component(XIA_ROOT_DIR "${XIA_LIBRARY_DIR}/.." REALPATH)

    #Write some useful info.
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg
            "#Pixie Configuration\n"
            "#\n"
            "#The following lines provide the configuration for a XIA Pixie16 data acquistion  \n"
            "#system. The configuration file is broken into two sections a global section for  \n"
            "#tags that affect the entire system and a module specific section. The global tags\n"
            "#include: PixieBaseDir, CrateConfig, SlotFile, DspSetFileFile, and                \n"
            "#DspWorkingSetFile. The module	tags include: ModuleType, ModuleBaseDir,          \n"
            "#SpFpgaFile, ComFpgaFile, DspConfFile, and DspVarFile. The module tags are        \n"
            "#associated with the type specified prior to the tag. If no type is specified the \n"
            "#type 'default' is used.                                                          \n"
            "#\n"
            "#The tag values are prepended with a base directory unless the first character in \n"
            "#the value is forward slash, '/',  or a period '.', permiting the use of absolute \n"
            "#and relative paths. The global tags are prepended with the PixieBaseDir. Module  \n"
            "#tags are prepended with the ModuleBaseDir if specified otherwise the PixieBaseDir\n"
            "#is used. If no base directory is determined the path is assumed to be local to   \n"
            "#the running directory.\n"
            "\n"
            )

    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "# Global Tags\n\n")

    #Write the base directory
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "PixieBaseDir\t\t${XIA_ROOT_DIR}\n")

    #Following are lists of keys and the glob expr to find the files
    set(CONFIG_NAME CrateConfig SlotFile DspSetFile)
    set(CONFIG_EXPR
            test/pxisys*.ini #CrateConfig
            configuration/slot_def.set #SlotFile
            configuration/default.set #DspSetFile
            )

    foreach (CONFIG_STEP RANGE 0 2)
        #Get key name and expression form the list
        list(GET CONFIG_NAME ${CONFIG_STEP} KEY)
        list(GET CONFIG_EXPR ${CONFIG_STEP} GLOB_EXPR)

        #Find all files matching the expression
        # Returns the path of the file relative to the base directory.
        file(GLOB FILE_MATCHES RELATIVE ${XIA_ROOT_DIR} ${XIA_ROOT_DIR}/${GLOB_EXPR})

        #Check that a unique match was found
        list(LENGTH FILE_MATCHES NUM_MATCHES)
        if (NOT NUM_MATCHES EQUAL 1)
            message(STATUS "WARNING: Unable to autocomplete global configuration!\n\tUnique ${KEY} file (${GLOB_EXPR}) not found!")
            if (NUM_MATCHES EQUAL 0)
                file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg
                        "#ERROR: No ${KEY} found! Please specify and remove this comment.\n")
            else (NUM_MATCHES EQUAL 0)
                file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg
                        "#ERROR: Multiple ${KEY}s found! Please choose one and remove this comment.\n")
            endif (NUM_MATCHES EQUAL 0)
        endif ()

        if (${KEY} MATCHES "SlotFile")
            if (NUM_MATCHES EQUAL 1)
                configure_file(${XIA_ROOT_DIR}/${FILE_MATCHES} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
                install(FILES ${CMAKE_CURRENT_BINARY_DIR}/slot_def.set DESTINATION
                        ${CMAKE_INSTALL_PREFIX}/share/config)
                set(FILE_MATCHES "./slot_def.set")
            endif ()
        elseif (${KEY} MATCHES "DspSetFile")
            if (NUM_MATCHES EQUAL 1)
                install(FILES ${XIA_ROOT_DIR}/${FILE_MATCHES} PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
                        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/config)
                #Rename set file to current.set to maintain default.set for backup
                install(FILES ${XIA_ROOT_DIR}/${FILE_MATCHES} RENAME current.set DESTINATION
                        ${CMAKE_INSTALL_PREFIX}/share/config)
                set(FILE_MATCHES ./current.set)
            endif ()
        endif ()

        #Append the config file
        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "${KEY}\t\t${FILE_MATCHES}\n")
    endforeach (CONFIG_STEP RANGE 0 2)

    #Added the working set file name
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "DspWorkingSetFile\t./current.set\n")

    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "\n# Module Tags\n")

    #Look in the root directory for the XIA library
    if (NOT EXISTS ${XIA_FIRMWARE_DIR})
        message(WARNING "Configuration Error - Invalid Pixie firmware directory: ${XIA_FIRMWARE_DIR}")
        return()
    endif (NOT EXISTS ${XIA_FIRMWARE_DIR})
    subdirlist(XIA_FIRMWARE_DIRS ${XIA_FIRMWARE_DIR})

    #remove directories without subdirectories firmware and dsp.
    foreach (FIRMWARE_DIR ${XIA_FIRMWARE_DIRS})
        if (NOT (EXISTS ${FIRMWARE_DIR}/firmware OR EXISTS ${FIRMWARE_FIR}/dsp))
            list(REMOVE_ITEM XIA_FIRMWARE_DIRS ${FIRMWARE_DIR})
        endif (NOT (EXISTS ${FIRMWARE_DIR}/firmware OR EXISTS ${FIRMWARE_FIR}/dsp))
    endforeach (FIRMWARE_DIR ${XIA_FIRMWARE_DIRS})

    #Following are lists of keys and the glob expr to find the files
    set(CONFIG_NAME SpFpgaFile ComFpgaFile DspConfFile DspVarFile)
    set(CONFIG_EXPR
            firmware/fippixie16*.bin #SpFpgaFile
            firmware/syspixie16*.bin #ComFpgaFile
            dsp/Pixie16DSP*.ldr #DspConfFile
            dsp/Pixie16DSP*.var #DspVarFile
            )

    foreach (FIRMWARE_DIR ${XIA_FIRMWARE_DIRS})
        #determine the module type from the fippi SpFpga File
        unset(MODULE_TYPE)
        file(GLOB FILE_MATCHES RELATIVE ${FIRMWARE_DIR} ${FIRMWARE_DIR}/firmware/fippixie16*.bin)
        foreach (FILENAME ${FILE_MATCHES})
            string(REGEX MATCH "[01234567890]+b[0123456789]+m" TYPE ${FILENAME})
            string(REGEX MATCH "rev[abcdf]" REVISION ${FILENAME})
            #If type is missing we set it to "unknown"
            if ("${TYPE}" STREQUAL "")
                set(TYPE "unknown")
            endif ()

            #If revision is missing we set it to "revUnknown"
            if ("${REVISION}" STREQUAL "")
                set(REVISION "revUnknown")
            endif ()

            #Only add the module type if it is not in the list.
            if (NOT MODULE_TYPE MATCHES "${TYPE}-${REVISION}")
                list(APPEND MODULE_TYPE "${TYPE}-${REVISION}")
            endif (NOT MODULE_TYPE MATCHES "${TYPE}-${REVISION}")
        endforeach (FILENAME ${FILE_MATCHES})

        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "\n")

        message(STATUS "Autoconfiguring module type: ${MODULE_TYPE}.")

        list(LENGTH MODULE_TYPE NUM_MATCHES)
        if (NUM_MATCHES EQUAL 1)
            if (${MODULE_TYPE} MATCHES "unknown")
                message(STATUS "WARNING: Incomplete module type (${MODULE_TYPE}) found in:")
                message(STATUS "    ${FIRMWARE_DIR}")
                file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg
                        "#ERROR: Incomplete ModuleType found! Please correct and remove this comment.\n")
            endif (${MODULE_TYPE} MATCHES "unknown")
        else (NUM_MATCHES EQUAL 1)
            message(STATUS "WARNING: Multiple module types (${MODULE_TYPE}) found in:")
            message(STATUS "    ${FIRMWARE_DIR}")
            if (NUM_MATCHES EQUAL 0)
                file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg
                        "#ERROR: No ModuleType found! Please specify and remove this comment.\n")
            else (NUM_MATCHES EQUAL 0)
                file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg
                        "#ERROR: Multiple ModuleTypes found! Please choose one and remove this comment.\n")
            endif (NUM_MATCHES EQUAL 0)
        endif (NUM_MATCHES EQUAL 1)

        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "ModuleType\t\t${MODULE_TYPE}\n")
        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "ModuleBaseDir\t\t${FIRMWARE_DIR}\n")

        #We loop over each item in the list and search for a matching file
        foreach (CONFIG_STEP RANGE 0 3)
            #Get key name and expression form the list
            list(GET CONFIG_NAME ${CONFIG_STEP} KEY)
            list(GET CONFIG_EXPR ${CONFIG_STEP} GLOB_EXPR)

            #Find all files matching the expression
            # Returns the path of the file relative to the base directory.
            file(GLOB FILE_MATCHES RELATIVE ${FIRMWARE_DIR} ${FIRMWARE_DIR}/${GLOB_EXPR})

            #Check that a unique match was found
            list(LENGTH FILE_MATCHES NUM_MATCHES)
            if (NOT NUM_MATCHES EQUAL 1)
                if (NOT ERROR)
                    set(ERROR TRUE)
                    message(STATUS "WARNING: Unable to autocomplete ${MODULE_TYPE} configuration!")
                endif (NOT ERROR)
                message(STATUS "    Unique ${KEY} file (${GLOB_EXPR}) not found!")
                if (NUM_MATCHES EQUAL 0)
                    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg
                            "#ERROR: No ${KEY} found! Please specify and remove this comment.\n")
                else (NUM_MATCHES EQUAL 0)
                    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg
                            "#ERROR: Multiple ${KEY}s found! Please choose one and remove this comment.\n")
                endif (NUM_MATCHES EQUAL 0)
            endif ()

            #Append the config file
            file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "${KEY}\t\t${FILE_MATCHES}\n")
        endforeach (CONFIG_STEP RANGE 0 3)
    endforeach (FIRMWARE_DIR ${XIA_FIRMWARE_DIRS})

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg DESTINATION ${CMAKE_INSTALL_PREFIX}/share/config)
endfunction()

macro(SUBDIRLIST result curdir)
    FILE(GLOB children RELATIVE ${curdir} "${curdir}/*")
    FOREACH (child ${children})
        IF (IS_DIRECTORY ${curdir}/${child})
            GET_FILENAME_COMPONENT(child ${curdir}/${child} REALPATH)
            LIST(APPEND dirlist ${child})
        ENDIF ()
    ENDFOREACH ()
    LIST(REMOVE_DUPLICATES dirlist)
    SET(${result} ${dirlist})
endmacro(SUBDIRLIST)

