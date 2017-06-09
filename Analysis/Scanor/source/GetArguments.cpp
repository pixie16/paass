///@file GetArguments.cpp
///@author D. Miller, S. V. Paulauskas
///@date September 26, 2016

#include <cstring>

#include <string>
#include <vector>

#include "GetArguments.hpp"

///C++ Wrapper to get the number of arguments (argc). We add
/// one to take into account the difference between FORTRAN 1 and C++ 0
/// counting.
int GetNumberArguments(void) {
    return IARGC__GETARGS() + 1;
}

///@brief C++ Wrapper to get a particular argument from the command line
std::string GetArgument(const int &i) {
    std::string tmp = "";
    char arg[MAX_FORTRAN_ARG_LENGTH];
    GETARG__GETARGS(i, arg, MAX_FORTRAN_ARG_LENGTH);
    tmp = arg;
    return tmp.substr(0, tmp.find_first_of(" "));
}

///\brief Returns an argv style array that can be used to pass to getopt and
/// other similar functions.
char **GetArguments(void) {
    static std::vector<std::string> args;
    for (int i = 0; i < GetNumberArguments(); i++)
        args.push_back(GetArgument(i));

    //Create this as static so that we do not go out of scope. (probably a
    // better way to do this but it'll work for now.
    static std::vector<char *> cstrings;
    for (unsigned int i = 0; i < args.size(); ++i)
        cstrings.push_back(const_cast<char *>(args.at(i).c_str()));
    return &cstrings[0];
}
