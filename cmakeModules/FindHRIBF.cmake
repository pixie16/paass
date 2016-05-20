#Find the HRIBF libraries
#
#Sets the usual variables for find_package scripts
#
#Authored: K. Smith and S. V. Paulauskas May 18, 2016
find_path(UPAK_LIBRARY_DIR
  NAMES scanorlib.a orphlib.a
  HINTS $ENV{HHIRF_DIR}
  PATHS /usr/hhirf
  /opt/hhirf)
find_path(ACQ2_LIBRARY_DIR
  NAMES acqlib.a ipclib.a
  HINTS $ENV{ACQ2_LIBDIR}
  PATHS /usr/acq2
  /opt/acq2)
# Support the REQUIRED and QUIET arguments.
include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HRIBF DEFAULT_MSG 
  UPAK_LIBRARY_DIR 
  ACQ2_LIBRARY_DIR)
if(HRIBF_FOUND)
  set (HRIBF_LIBRARIES 
    ${UPAK_LIBRARY_DIR}/scanorlib.a 
    ${UPAK_LIBRARY_DIR}/orphlib.a 
    ${ACQ2_LIBRARY_DIR}/acqlib.a 
    ${ACQ2_LIBRARY_DIR}/ipclib.a)
endif(HRIBF_FOUND)
mark_as_advanced(HRIBF_LIBRARIES)