#Find the PixieSuite libraries
#
#Sets the usual variables for find_package scripts
#
#Authored: S. V. Paulauskas June 14, 2016

# Support the REQUIRED and QUIET arguments.
include (FindPackageHandleStandardArgs)

find_path(PIXIE_SUITE_DIR
  NAMES libPixieCore.so libPixieScan.so
  HINTS $ENV{PIXIE_SUITE_DIR}
  PATHS /opt/PixieSuite2/lib)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(PIXIE_SUITE DEFAULT_MSG 
  PIXIE_SUITE_DIR)

if(PIXIE_SUITE_FOUND)
  set (PIXIE_SUITE_LIBS
    ${PIXIE_SUITE_DIR}/libPixieCore.so 
    ${PIXIE_SUITE_DIR}/libPixieScan.so)
endif(PIXIE_SUITE_FOUND)

mark_as_advanced(PIXIE_SUITE_LIBS)