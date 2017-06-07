# - Finds ROOT instalation
# This module sets up ROOT information 
# It defines:
# ROOT_FOUND          If the ROOT is found
# ROOT_INCLUDE_DIR    PATH to the include directory
# ROOT_LIBRARIES      Most common libraries
# ROOT_GUI_LIBRARIES  Most common gui libraries
# ROOT_LIBRARY_DIR    PATH to the library directory 
#
#Last updated by K. Smith (ksmit218@utk.edu) on Apr 10, 2014

#Find the root-config executable
set(ROOTSYS $ENV{ROOTSYS} CACHE Path "ROOT directory.")
find_program(ROOT_CONFIG_EXECUTABLE root-config
  PATHS ${ROOTSYS}/bin)
find_program(ROOTCINT_EXECUTABLE rootcint PATHS $ENV{ROOTSYS}/bin)
find_program(GENREFLEX_EXECUTABLE genreflex PATHS $ENV{ROOTSYS}/bin)

#If we found root-config then get all relevent varaiables
if(ROOT_CONFIG_EXECUTABLE)
  execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --prefix 
    OUTPUT_VARIABLE ROOTSYS 
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --version 
    OUTPUT_VARIABLE ROOT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --incdir
    OUTPUT_VARIABLE ROOT_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --libs
    OUTPUT_VARIABLE ROOT_LIBRARIES
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --glibs
    OUTPUT_VARIABLE ROOT_GUI_LIBRARIES
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  set(ROOT_LIBRARY_DIR ${ROOTSYS}/lib)
endif()

#---Report the status of finding ROOT-------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ROOT DEFAULT_MSG 
		ROOTSYS ROOT_CONFIG_EXECUTABLE ROOTCINT_EXECUTABLE GENREFLEX_EXECUTABLE
		ROOT_VERSION ROOT_INCLUDE_DIR ROOT_LIBRARIES ROOT_LIBRARY_DIR)

mark_as_advanced(FORCE ROOT_LIBRARIES ROOT_GUI_LIBRARIES)

#----------------------------------------------------------------------------
# function ROOT_GENERATE_DICTIONARY( dictionary   
#                                    header1 header2 ... 
#                                    LINKDEF linkdef1 ... 
#                                    OPTIONS opt1...)
function(ROOT_GENERATE_DICTIONARY dictionary)
  include(CMakeParseArguments)
  CMAKE_PARSE_ARGUMENTS(ARG "" "" "LINKDEF;OPTIONS" "" ${ARGN})
  #---Get the list of include directories------------------
  get_directory_property(incdirs INCLUDE_DIRECTORIES)
  set(includedirs) 
  foreach( d ${incdirs})    
  	set(includedirs ${includedirs} -I${d})
  endforeach()
  #---Get LinkDef.h file------------------------------------
  set(linkdefs)
  foreach( f ${ARG_LINKDEF})
		find_file(linkFile ${f} PATHS ${incdirs} ${CMAKE_CURRENT_SOURCE_DIR})
		if(NOT linkFile) 
			message(FATAL_ERROR "Link file \"${f}\" not found!")
		endif()
      set(linkdefs ${linkdefs} ${linkFile})
		unset(linkFile CACHE)
  endforeach()
  #---Get the list of header files-------------------------
  set(headerfiles)
  foreach(fp ${ARG_UNPARSED_ARGUMENTS})
		find_file(headerFile ${fp} PATHS ${incdirs} ${CMAKE_CURRENT_SOURCE_DIR})
		if(NOT headerFile) 
			message(FATAL_ERROR "Header file \"${fp}\" not found!")
		endif()
      set(headerfiles ${headerfiles} ${headerFile})
		unset(headerFile CACHE)
  endforeach()
  #---call rootcint------------------------------------------
  add_custom_command(OUTPUT ${dictionary}.cxx ${dictionary}.h
                     COMMAND ${ROOTCINT_EXECUTABLE} -cint -f  ${dictionary}.cxx 
                                          -c ${ARG_OPTIONS} ${includedirs} ${headerfiles} ${linkdefs} 
							DEPENDS ${headerfiles} ${linkdefs} VERBATIM)
endfunction()

#----------------------------------------------------------------------------
# function REFLEX_GENERATE_DICTIONARY(dictionary   
#                                     header1 header2 ... 
#                                     SELECTION selectionfile ... 
#                                     OPTIONS opt1...)
function(REFLEX_GENERATE_DICTIONARY dictionary)  
  include(CMakeParseArguments)
  CMAKE_PARSE_ARGUMENTS(ARG "" "" "SELECTION;OPTIONS" "" ${ARGN})
  #---Get the list of include directories------------------
  get_directory_property(incdirs INCLUDE_DIRECTORIES)
  set(includedirs) 
  foreach( d ${incdirs})    
  	set(includedirs ${includedirs} -I${d})
  endforeach()
  #---Get the list of header files-------------------------
  set(headerfiles)
  foreach(fp ${ARG_UNPARSED_ARGUMENTS})
		find_file(headerFile ${fp} PATHS ${incdirs})
      set(headerfiles ${headerfiles} ${headerFile})
		unset(headerFile CACHE)
  endforeach()
  #---Get Selection file------------------------------------
  if(IS_ABSOLUTE ${ARG_SELECTION})
    set(selectionfile ${ARG_SELECTION})
  else() 
    set(selectionfile ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_SELECTION})
  endif()
  #---Get preprocessor definitions--------------------------
  get_directory_property(defs COMPILE_DEFINITIONS)
  foreach( d ${defs})    
   set(definitions ${definitions} -D${d})
  endforeach()
  #---Nanes and others---------------------------------------
  set(gensrcdict ${dictionary}.cpp)
  if(MSVC)
    set(gccxmlopts "--gccxmlopt=\"--gccxml-compiler cl\"")
  else()
    set(gccxmlopts)
  endif()  
  #---Check GCCXML and get path-----------------------------
  find_package(GCCXML)
  
  if(GCCXML)
    get_filename_component(gccxmlpath ${GCCXML} PATH)
  else()
    message(WARNING "GCCXML not found. Install and setup your environment to find 'gccxml' executable")
  endif()
  #---Actual command----------------------------------------
  add_custom_command(OUTPUT ${gensrcdict} ${rootmapname}     
                     COMMAND ${GENREFLEX_EXECUTABLE} ${headerfiles} -o ${gensrcdict} ${gccxmlopts} ${rootmapopts} --select=${selectionfile}
                             --gccxmlpath=${gccxmlpath} ${ARG_OPTIONS} ${includedirs} ${definitions}
                     DEPENDS ${headerfiles} ${selectionfile})
endfunction()
  
