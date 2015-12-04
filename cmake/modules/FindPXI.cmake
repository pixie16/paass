# Find the PXI Library
#
# Sets the usual variables expected for find_package scripts:
#
# PXI_LIBRARY_DIR
# PXI_FOUND - true if PXI was found.
#

find_path(PXI_ROOT_DIR
	NAMES software firmware dsp configuration 
	HINTS ENV PXI_ROOT
        PATHS /opt/xia/current
	DOC "Path to pixie firmware.")

find_path(PXI_LIBRARY_DIR
	NAMES libPixie16App.a 
	HINTS ${PXI_ROOT_DIR}/software
	DOC "Path to pixie library.")

# Support the REQUIRED and QUIET arguments, and set PXI_FOUND if found.
include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (PXI DEFAULT_MSG PXI_ROOT_DIR PXI_LIBRARY_DIR)

if (PXI_FOUND)
	set (PXI_INCLUDE_DIR ${PXI_LIBRARY_DIR}/inc ${PXI_LIBRARY_DIR}/sys ${PXI_LIBRARY_DIR}/app)
	set(PXI_LIBRARIES -lPixie16App -lPixie16Sys)
endif()

mark_as_advanced(PXI_LIBRARY_DIR)

function(PXI_CONFIG)
	message(STATUS "Creating Pixie configuration.")
	#Write the base directory
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "PixieBaseDir\t\t${PXI_ROOT_DIR}\n")

	#create an installer that can be invoked by
	#add_custom_target(config ${CMAKE_COMMAND} -P pixie_cfg.cmake)	
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pixie_cfg.cmake 
		"file(INSTALL pixie.cfg DESTINATION ${CMAKE_INSTALL_PREFIX}/share/config)\n" 
	)

	#Following are lists of keys and the glob expr to find the files
	set(CONFIG_NAME SpFpgaFile ComFpgaFile DspConfFile DspVarFile CrateConfig SlotFile DspSetFile)
	set(CONFIG_EXPR 
		firmware/fippixie16*.bin #SpFpgaFile
		firmware/syspixie16*.bin #ComFpgaFile
		dsp/Pixie16DSP*.ldr #DspConfFile
		dsp/Pixie16DSP*.var #DspVarFile
		test/pxisys.ini #CrateConfig
		configuration/slot_def.set #SlotFile
		configuration/default.set #DspSetFile
	)

	#We loop over each item in the list and search for a matching file
	foreach(CONFIG_STEP RANGE 0 6)
		#Get key name and expression form the list
		list(GET CONFIG_NAME ${CONFIG_STEP} KEY)
		list(GET CONFIG_EXPR ${CONFIG_STEP} GLOB_EXPR)
		
		#Find all files matching hte expression
		# Returns the path of the file relative to the base directory.
		file(GLOB FILE_MATCHES RELATIVE ${PXI_ROOT_DIR} ${PXI_ROOT_DIR}/${GLOB_EXPR})
	
		#Check that a unique match was found
		list(LENGTH FILE_MATCHES NUM_MATCHES)
		if (NOT NUM_MATCHES EQUAL 1)
			message(STATUS "Warning: Unable to complete configuration! Unique ${KEY} file (${GLOB_EXPR}) not found!")
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
					"file(INSTALL default.set DESTINATION ${CMAKE_INSTALL_PREFIX}/share/config)\n")
				set(FILE_MATCHES ./default.set)
			endif()
		endif ()
		#Append the config file
		file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "${KEY}\t\t${FILE_MATCHES}\n")
	endforeach()

	#Added the working set file name
	file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/pixie.cfg "DspWorkingSetFile\t./default.set")

endfunction()
