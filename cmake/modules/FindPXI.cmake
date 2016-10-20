# Find the PXI Library
#
# Sets the usual variables expected for find_package scripts:
#
# PXI_LIBRARY_DIR
# PXI_FOUND - true if PXI was found.
#

#Unset any cached value to ensure a fresh search is performed. 
#This permits the user to change the PXI_ROOT_DIR and have subsequent paths updated.
unset(PXI_LIBRARY_DIR CACHE)

#Find the library path by looking for the library.
find_path(PXI_LIBRARY_DIR
	NAMES libPixie16App.a libPixie16Sys.a 
	HINTS ${PXI_ROOT_DIR}
	PATHS /opt/xia/current
	PATH_SUFFIXES software
	DOC "Path to pixie library.")

if(NOT PXI_FIRMWARE_DIR)
	get_filename_component(PXI_FIRMWARE_DIR "${PXI_LIBRARY_DIR}/../.." REALPATH)
endif(NOT PXI_FIRMWARE_DIR)
set(PXI_FIRMWARE_DIR ${PXI_FIRMWARE_DIR} CACHE PATH "Path to folder containing XIA firmware.")

#The order is strange here as we are really interested in the libraries first
# then we determine the root directory from there.

# Support the REQUIRED and QUIET arguments, and set PXI_FOUND if found.
include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (PXI DEFAULT_MSG PXI_LIBRARY_DIR)

if (PXI_FOUND)
	set (PXI_INCLUDE_DIR ${PXI_LIBRARY_DIR}/inc ${PXI_LIBRARY_DIR}/sys ${PXI_LIBRARY_DIR}/app)
	set(PXI_LIBRARIES -lPixie16App -lPixie16Sys)
endif()

function(PXI_CONFIG)
	message(STATUS "Creating Pixie configuration.")

	get_filename_component(PXI_ROOT_DIR "${PXI_LIBRARY_DIR}/.." REALPATH)

	#create an installer that can be invoked by
	#add_custom_target(config ${CMAKE_COMMAND} -P pixie_cfg.cmake)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pixie_cfg.cmake 
		"file(INSTALL pixie.cfg DESTINATION ${CMAKE_INSTALL_PREFIX}/share/config)\n" 
	)

	#Write the base directory
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "PixieBaseDir\t\t${PXI_ROOT_DIR}\n")

	#Following are lists of keys and the glob expr to find the files
	set(CONFIG_NAME CrateConfig SlotFile DspSetFile)
	set(CONFIG_EXPR 
		test/pxisys*.ini #CrateConfig
		configuration/slot_def.set #SlotFile
		configuration/default.set #DspSetFile
	)

	foreach(CONFIG_STEP RANGE 0 2)
		#Get key name and expression form the list
		list(GET CONFIG_NAME ${CONFIG_STEP} KEY)
		list(GET CONFIG_EXPR ${CONFIG_STEP} GLOB_EXPR)

		#Find all files matching the expression
		# Returns the path of the file relative to the base directory.
		file(GLOB FILE_MATCHES RELATIVE ${PXI_ROOT_DIR} ${PXI_ROOT_DIR}/${GLOB_EXPR})

		#Check that a unique match was found
		list(LENGTH FILE_MATCHES NUM_MATCHES)
		if (NOT NUM_MATCHES EQUAL 1)
			message(STATUS "WARNING: Unable to autocomplete configuration!\n\tUnique ${KEY} file (${GLOB_EXPR}) not found!")
			file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg 
				"# Multiple / zero options found! Choose below and remove this comment.\n")
		endif()

		if (${KEY} MATCHES "SlotFile")
			if (NUM_MATCHES EQUAL 1)
				configure_file(${PXI_ROOT_DIR}/${FILE_MATCHES} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
				file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie_cfg.cmake 
					"file(INSTALL slot_def.set DESTINATION ${CMAKE_INSTALL_PREFIX}/share/config)\n")
				set(FILE_MATCHES "./slot_def.set")
			endif()
		elseif (${KEY} MATCHES "DspSetFile")
			if (NUM_MATCHES EQUAL 1)
				configure_file(${PXI_ROOT_DIR}/${FILE_MATCHES} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
				file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie_cfg.cmake 
					"file(INSTALL default.set 
						PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
						DESTINATION ${CMAKE_INSTALL_PREFIX}/share/config)\n")
				#Rename set file to current.set to maintain default.set for backup
				file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie_cfg.cmake 
					"file(INSTALL default.set 
						RENAME current.set
						DESTINATION ${CMAKE_INSTALL_PREFIX}/share/config)\n")
				set(FILE_MATCHES ./current.set)
			endif()
		endif ()

		#Append the config file
		file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "${KEY}\t\t${FILE_MATCHES}\n")
	endforeach(CONFIG_STEP RANGE 0 2)

	#Added the working set file name
	file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "DspWorkingSetFile\t./current.set\n")

	#Look in the root directory for the XIA library
	if(NOT EXISTS ${PXI_FIRMWARE_DIR})
		message(WARNING "Configuration Error - Invalid Pixie firmware directory: ${PXI_FIRMWARE_DIR}")
		return()
	endif(NOT EXISTS ${PXI_FIRMWARE_DIR})
	subdirlist(PXI_FIRMWARE_DIRS ${PXI_FIRMWARE_DIR})

	#Following are lists of keys and the glob expr to find the files
	set(CONFIG_NAME SpFpgaFile ComFpgaFile DspConfFile DspVarFile)
	set(CONFIG_EXPR 
		firmware/fippixie16*.bin #SpFpgaFile
		firmware/syspixie16*.bin #ComFpgaFile
		dsp/Pixie16DSP*.ldr #DspConfFile
		dsp/Pixie16DSP*.var #DspVarFile
	)

	foreach(FIRMWARE_DIR ${PXI_FIRMWARE_DIRS})
		#determine the module type from the fippi SpFpga File
		unset(MODULE_TYPE)
		file(GLOB FILE_MATCHES RELATIVE ${FIRMWARE_DIR} ${FIRMWARE_DIR}/firmware/fippixie16*.bin)
		foreach(FILENAME ${FILE_MATCHES})
			string(REGEX MATCH "[01234567890]+b[0123456789]+m" TYPE ${FILENAME})
			string(REGEX MATCH "rev[abcdf]" REVISION ${FILENAME})
			if (NOT MODULE_TYPE)
				set(MODULE_TYPE "${TYPE}-${REVISION}")
			else (NOT MODULE_TYPE)
				if(NOT MODULE_TYPE EQUAL "${TYPE}-${REVISION}")
					list(APPEND MODULE_TYPE "${TYPE}-${REVISION}")
				endif(NOT MODULE_TYPE EQUAL "${TYPE}-${REVISION}")
			endif (NOT MODULE_TYPE)
		endforeach(FILENAME ${FILE_MATCHES})

		file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "\n")

		list(LENGTH MODULE_TYPE NUM_MATCHES)
		if (NOT NUM_MATCHES EQUAL 1)
				message(STATUS "WARNING: Multiple module types found in ${FIRMWARE_DIR}")
			file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg 
				"# Multiple / zero options found! Choose below and remove this comment.\n")
		endif()

		file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "ModuleType\t\t${MODULE_TYPE}\n")
		file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "ModuleBaseDir\t\t${FIRMWARE_DIR}\n")

		#We loop over each item in the list and search for a matching file
		foreach(CONFIG_STEP RANGE 0 3)
			#Get key name and expression form the list
			list(GET CONFIG_NAME ${CONFIG_STEP} KEY)
			list(GET CONFIG_EXPR ${CONFIG_STEP} GLOB_EXPR)
		
			#Find all files matching the expression
			# Returns the path of the file relative to the base directory.
			file(GLOB FILE_MATCHES RELATIVE ${FIRMWARE_DIR} ${FIRMWARE_DIR}/${GLOB_EXPR})
		
			#Check that a unique match was found
			list(LENGTH FILE_MATCHES NUM_MATCHES)
			if (NOT NUM_MATCHES EQUAL 1)
				message(STATUS "WARNING: Unable to autocomplete configuration!\n\tUnique ${KEY} file (${GLOB_EXPR}) not found!")
				file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg 
					"# Multiple / zero options found! Choose below and remove this comment.\n")
			endif()

			#Append the config file
			file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "${KEY}\t\t${FILE_MATCHES}\n")
		endforeach(CONFIG_STEP RANGE 0 3)
	endforeach(FIRMWARE_DIR ${PXI_FIRMWARE_DIRS})

endfunction()

macro(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} "${curdir}/*")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
		GET_FILENAME_COMPONENT(child ${curdir}/${child} REALPATH)
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  LIST(REMOVE_DUPLICATES dirlist)
  SET(${result} ${dirlist})
endmacro(SUBDIRLIST)

