/// @file GetArguments.hpp

/// @brief A quick and dirty way to get the arguments pass to scan through
/// the respective Fortran calls
/// @author David Miller
/// @date June 2012

#ifndef __GETARGUMENTS_HPP_
#define __GETARGUMENTS_HPP_

#ifdef LINK_GFORTRAN
///@brief Define this version of GETARG__GETARGS if gfortran
#define GETARG__GETARGS _gfortran_getarg_i4
///@brief Define this version of IARGC__GETARGS if gfortran
#define IARGC__GETARGS _gfortran_iargc
#else
///@brief Define this version of GETARG__GETARGS if g77
#define GETARG__GETARGS getarg_
///@brief Define this version of IARGC__GETARGS if g77
#define IARGC__GETARGS iargc_
#endif

#define MAX_FORTRAN_ARG_LENGTH 1024

///@brief Define the fortran function to get the argc
///@return the number of args
extern "C" int IARGC__GETARGS(void);
///@brief Define the fortran function to get the arguments
///@returns a pointer to the beginning of the arg
extern "C" char *GETARG__GETARGS(const int &, char *, int);

///@return C++ Wrapper to get the number of arguments (argc). We add
/// one to take into account the difference between FORTRAN 1 and C++ 0
/// counting.
int GetNumberArguments(void);

///@brief C++ Wrapper to get a particular argument from the command line
///@param [in] i : the index of the argument to get
std::string GetArgument(const int &i);

///\brief Returns an argv style array that can be used to pass to getopt and
/// other similar functions.
char **GetArguments(void);

#endif // _GETARGUMENTS_HPP_
