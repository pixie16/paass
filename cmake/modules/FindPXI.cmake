# Find the PXI Library
#
# Sets the usual variables expected for find_package scripts:
#
# PXI_LIBRARY_DIR
# UPAK_FOUND - true if UPAK was found.
#

find_path(PXI_ROOT_DIR
	NAMES software firmware dsp configuration 
	PATHS /opt/PixieFirmware/current)

find_path(PXI_LIBRARY_DIR
	NAMES libPixie16App.a 
	HINTS ${PXI_ROOT_DIR}/software)

# Support the REQUIRED and QUIET arguments, and set PUGIXML_FOUND if found.
include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (PXI DEFAULT_MSG PXI_ROOT_DIR PXI_LIBRARY_DIR)

if (PXI_FOUND)
	set (PXI_INCLUDE_DIR ${PXI_LIBRARY_DIR}/inc ${PXI_LIBRARY_DIR}/sys ${PXI_LIBRARY_DIR}/app)
	set(PXI_LIBRARIES -lPixie16App -lPixie16Sys)
endif()

mark_as_advanced (PXI_ROOT_DIR PXI_LIBRARY_DIR)
