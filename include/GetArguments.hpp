/** \file GetArguments.hpp
 * \brief A quick and dirty way to get the arguments pass to scan through
 * the respective Fortran calls
 *
 * \author David Miller
 * \date June 2012
 */

#ifndef __GETARGUMENTS_HPP_
#define __GETARGUMENTS_HPP_

#ifdef LINK_GFORTRAN
#define GETARG__GETARGS _gfortran_getarg_i4
#define IARGC__GETARGS _gfortran_iargc
#else
#define GETARG__GETARGS getarg_
#define IARGC__GETARGS iargc_
#endif

extern "C" char* GETARG__GETARGS(const int &, char *, int);
extern "C" int IARGC__GETARGS(void);

/** 
 * Get the number of arguments (argc)
 */
inline int GetNumberArguments(void) 
{
    return IARGC__GETARGS();
}

/**
 * Get a particular argument from the command line
 */
inline void GetArgument(int i, char *arg, int length)
{
    GETARG__GETARGS(i, arg, length);
}
#endif // _GETARGUMENTS_HPP_
