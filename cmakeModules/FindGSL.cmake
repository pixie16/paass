# Try to find gnu scientific library GSL
# See 
# http://www.gnu.org/software/gsl/  and 
# http://gnuwin32.sourceforge.net/packages/gsl.htm
#
# Based on a script of Felix Woelk and Jan Woetzel
# (www.mip.informatik.uni-kiel.de)
# 
# It defines the following variables:
#  GSL_FOUND - system has GSL lib
#  GSL_INCLUDE_DIRS - where to find headers 
#  GSL_LIBRARIES - full path to the libraries
#  GSL_LIBRARY_DIRS, the directory where the PLplot library is found.
 
#  CMAKE_GSL_CXX_FLAGS  = Unix compiler flags for GSL, essentially "`gsl-config --cxxflags`"
#  GSL_LINK_DIRECTORIES = link directories, useful for rpath on Unix
#  GSL_EXE_LINKER_FLAGS = rpath on Unix

# Support the REQUIRED and QUIET arguments.
include (FindPackageHandleStandardArgs)

set( GSL_FOUND OFF )
set( GSL_CBLAS_FOUND OFF )
 
mark_as_advanced(GSL_INCLUDE_DIR GSL_LIBRARY GSL_CBLAS_LIBRARY)

find_program( GSL_CONFIG_EXECUTABLE gsl-config 
  /usr/bin/ 
  /usr/local/bin )

if( GSL_CONFIG_EXECUTABLE ) 
  set( GSL_FOUND ON )
  
  #run the gsl-config to get the GSL Version
  execute_process(
    COMMAND sh "${GSL_CONFIG_EXECUTABLE}" --version
    OUTPUT_VARIABLE GSL_VERSION
    RESULT_VARIABLE RET
    ERROR_QUIET )

  # run the gsl-config program to get cxxflags
  execute_process(
    COMMAND sh "${GSL_CONFIG_EXECUTABLE}" --cflags
    OUTPUT_VARIABLE GSL_CFLAGS
    RESULT_VARIABLE RET
    ERROR_QUIET )
  if( RET EQUAL 0 )
    string( STRIP "${GSL_CFLAGS}" GSL_CFLAGS )
    separate_arguments( GSL_CFLAGS )
    
    # parse definitions from cflags; drop -D* from CFLAGS
    string( REGEX MATCHALL "-D[^;]+"
      GSL_DEFINITIONS  "${GSL_CFLAGS}" )
    string( REGEX REPLACE "-D[^;]+;" ""
      GSL_CFLAGS "${GSL_CFLAGS}" )
    
    # parse include dirs from cflags; drop -I prefix
    string( REGEX MATCHALL "-I[^;]+"
      GSL_INCLUDE_DIRS "${GSL_CFLAGS}" )
    string( REPLACE "-I" ""
      GSL_INCLUDE_DIRS "${GSL_INCLUDE_DIRS}")
    string( REGEX REPLACE "-I[^;]+;" ""
      GSL_CFLAGS "${GSL_CFLAGS}")
    
    #      message("GSL_DEFINITIONS=${GSL_DEFINITIONS}")
    #      message("GSL_INCLUDE_DIRS=${GSL_INCLUDE_DIRS}")
    #      message("GSL_CFLAGS=${GSL_CFLAGS}")
  else( RET EQUAL 0 )
    set( GSL_FOUND OFF )
  endif( RET EQUAL 0 )
  
  # run the gsl-config program to get the libs
  execute_process(
    COMMAND sh "${GSL_CONFIG_EXECUTABLE}" --libs
    OUTPUT_VARIABLE GSL_LIBRARIES
    RESULT_VARIABLE RET
    ERROR_QUIET )
  if( RET EQUAL 0 )
    string(STRIP "${GSL_LIBRARIES}" GSL_LIBRARIES )
    separate_arguments( GSL_LIBRARIES )
    
    # extract linkdirs (-L) for rpath (i.e., LINK_DIRECTORIES)
    string( REGEX MATCHALL "-L[^;]+"
      GSL_LIBRARY_DIRS "${GSL_LIBRARIES}" )
    string( REPLACE "-L" ""
      GSL_LIBRARY_DIRS "${GSL_LIBRARY_DIRS}" )
  else( RET EQUAL 0 )
    set( GSL_FOUND OFF )
  endif( RET EQUAL 0 )
  
  #run the gsl-config program to get the GSL installation prefix
  execute_process(
    COMMAND sh "${GSL_CONFIG_EXECUTABLE}" --prefix
    OUTPUT_VARIABLE GSL_PREFIX
    RESULT_VARIABLE RET
    ERROR_QUIET )
  if( NOT RET EQUAL 0 )
    set ( GSL_FOUND OFF )
  endif( NOT RET EQUAL 0 )
  
  MARK_AS_ADVANCED(GSL_CFLAGS)
else( GSL_CONFIG_EXECUTABLE )
  message( STATUS "FindGSL: gsl-config not found.")
endif( GSL_CONFIG_EXECUTABLE )

if( GSL_FOUND )
  if( NOT GSL_FIND_QUIETLY )
    message( STATUS "FindGSL: Found both GSL headers and library" )
  endif( NOT GSL_FIND_QUIETLY )
else( GSL_FOUND )
  if( GSL_FIND_REQUIRED )
    message( FATAL_ERROR "FindGSL: Could not find GSL headers or library" )
  endif( GSL_FIND_REQUIRED )
endif( GSL_FOUND )