# - Finds UnitTestCpp instalation
# It defines:
#    UNITTEST++_INCLUDE_DIR - The Directory containing the headers
#    UNITTEST++_LIBRARY_DIR - The directory containing the libraries.
#Last updated by S. V. Paulauskas (spaulaus AT utk DOT edu) on November 22, 2016

#Locate the Include directory
find_path(UNITTEST++_INCLUDE_DIR
        NAMES UnitTest++.h
        PATHS /opt/UnitTest++/include
        PATH_SUFFIXES UnitTest++)
#Locate the library
find_path(UNITTEST++_LIBRARY_DIR
        NAMES libUnitTest++.a
        PATHS /opt/UnitTest++
        PATH_SUFFIXES lib)

include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (UnitTest++ DEFAULT_MSG
        UNITTEST++_INCLUDE_DIR)
mark_as_advanced (UNITTEST++_INCLUDE_DIR UNITTEST++_LIBRARY_DIR)
