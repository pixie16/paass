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
/** Define this version of GETARG__GETARGS if gfortran */
#define GETARG__GETARGS _gfortran_getarg_i4
/** Define this version of IARGC__GETARGS if gfortran */
#define IARGC__GETARGS _gfortran_iargc
#else
/** Define this version of GETARG__GETARGS if g77 */
#define GETARG__GETARGS getarg_
/** Define this version of IARGC__GETARGS if f77 */
#define IARGC__GETARGS iargc_
#endif

/** Define the fortran function to get the argc
 * \return the number of args */
extern "C" int IARGC__GETARGS(void);
/** Define the fortran function to get the arguments
 * \returns a pointer to the beginning of the arg */
extern "C" char* GETARG__GETARGS(const int &, char *, int);

/** \return C++ Wrapper to get the number of arguments (argc) */
inline int GetNumberArguments(void) {
    return IARGC__GETARGS();
}

/** C++ Wrapper to get a particular argument from the command line
 * \param [in] i the index of the argument to get
 * \param [in] arg : where to put the argument
 * \param [in] length : the length of the arg */
inline void GetArgument(int i, char *arg, int length) {
    GETARG__GETARGS(i, arg, length);
}
#endif // _GETARGUMENTS_HPP_
