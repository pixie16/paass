/** \file set2root.hpp
  * \brief A program to convert a pixie16 binary .set file into a root file.
  *
  * This program reads a pixie16 dsp variable file (.var) and a binary .set
  * file and outputs a .root file containing the names of the pixie16
  * parameters and their values.
  *
  * \author C. R. Thornsberry, S. V. Paulauskas
  * \date May 18th, 2016
  */
#ifndef SET2ROOT_HPP
#define SET2ROOT_HPP

#include <vector>
#include <string.h>

#ifdef USE_ROOT_OUTPUT
class TFile;
#endif

class parameter {
public:
    std::vector<unsigned int> values;

    parameter() : name(), offset() {}

    parameter(const std::string &name_, const unsigned int &offset_) : name(name_), offset(offset_) {}

    std::string getName() { return name; }

    unsigned int getOffset() { return offset; }

#ifdef USE_ROOT_OUTPUT
    bool write(TFile *f_, const std::string &dir_="");
#endif

    std::string print();

private:
    std::string name; /// The name of the pixie16 parameter.
    unsigned int offset; /// The offset of this parameter in the .set file (in words).
};

#endif
