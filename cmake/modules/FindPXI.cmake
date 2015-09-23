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
