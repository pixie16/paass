/** \file set2root.cpp
  * \brief A program to convert a pixie16 binary .set file into a root file.
  *
  * This program reads a pixie16 dsp variable file (.var) and a binary .set
  * file and outputs a .root file containing the names of the pixie16
  * parameters and their values.
  *
  * \author C. R. Thornsberry, S. V. Paulauskas
  * \date May 18th, 2016
  */
#include <iostream>
#include <sstream>
#include <fstream>

#ifdef USE_ROOT_OUTPUT
#include "TNamed.h"
#include "TFile.h"
#endif

#include "HelperFunctions.hpp"
#include "set2root.hpp"

#define FILTER_CLOCK 8E-3 // Filter clock (in us)
#define ADC_CLOCK 4E-3 // ADC clock (in us)
#define READ_SIZE 1280 // Size of each module in the .set file.

#ifdef USE_ROOT_OUTPUT
bool parameter::write(TFile *f_, const std::string &dir_/*=""*/){
    if(!f_ || !f_->IsOpen() || values.empty()){ return false; }

    if(values.size() == 1){
        std::stringstream stream;
        stream << values.front();
        TNamed named(name.c_str(), stream.str().c_str());
        named.Write();
    }
    else{
        unsigned int count = 0;
        for(std::vector<unsigned int>::iterator iter = values.begin(); iter != values.end(); iter++){
            std::stringstream stream;
            stream << (*iter);

            std::stringstream chName;
            chName << name << "[";
            if(count < 10)
                chName << "0";
            chName << count << "]";

            TNamed named(chName.str().c_str(), stream.str().c_str());
            named.Write();
            count++;
        }
    }

    return true;
}
#endif

std::string parameter::print() {
    if (values.empty()) { return ""; }

    std::stringstream stream;
    if (values.size() == 1) {
        stream << name << "\t" << values.front() << "\n";
    } else {
        unsigned int count = 0;
        for (std::vector<unsigned int>::iterator iter = values.begin();
             iter != values.end(); iter++) {
            stream << name << "[";
            if (count < 10)
                stream << "0";
            stream << count << "]" << "\t";
            if (name != "PreampTau")
                stream << (*iter);
            else
                stream << IeeeStandards::IeeeFloatingToDecimal((*iter));
            stream << "\n";
            count++;
        }
    }

    return stream.str();
}

/** Find a parameter with a given file offset. This function returns
  * NULL if a match was not found, and a returns a pointer to a
  * parameter object upon success.
  *  param[in] params   : Vector filled with parameter objects.
  *  param[in] offset_  : Set file offset to attempt to match.
  */
parameter *
findParameter(std::vector<parameter> &params, const unsigned int &offset_) {
    for (std::vector<parameter>::iterator iter = params.begin();
         iter != params.end(); iter++) {
        if (iter->getOffset() == offset_) { return &(*iter); }
    }
    return NULL;
}

/** Read a pixie16 .var dsp file and store the parameter names and
  * offsets in a vector of parameter objects. This function returns
  * false if an error occured, and true upon success.
  *  param[out] params        : Vector to be filled with parameter objects.
  *  param[in] var_filename_  : Filename of the .var file to read.
  */
bool readVarFile(std::vector<parameter> &params, const char *var_filename_) {
    std::ifstream input(var_filename_);
    if (!input.good()) {
        return false;
    }

    params.clear();

    unsigned int value;
    std::string hexstr;
    std::string name;
    while (true) {
        input >> hexstr >> name;
        if (input.eof()) { break; }

        if (hexstr.find("0x") != std::string::npos)
            hexstr = hexstr.substr(hexstr.find("0x") + 2);

        std::stringstream stream;
        stream << std::hex << hexstr;
        stream >> value;

        params.push_back(parameter(name, value));
    }

    input.close();

    return true;
}

/** Read a pixie16 .set file and store the integer values in a vector
  * of parameter objects. This function returns -1 if an error
  * occured, and the number of entries read upon success.
  *  param[in] params        : Vector filled with parameter objects.
  *  param[in] set_filename_ : Filename of the .set file to read.
  *  param[in] offset_       : The number of words to skip at the beginning of the file.
  *  param[in] len_          : The number of words to read form the file.
  */
int readSetFile(std::vector<parameter> &params, const char *set_filename_,
                const size_t &offset_, const size_t &len_ = READ_SIZE) {
    if (params.empty()) { return -1; }

    std::ifstream input(set_filename_);
    if (!input.good()) {
        return -1;
    }

    input.seekg(offset_ * 4);
    if (input.eof()) {
        input.close();
        return -1;
    }

    unsigned int firstValue = params.front().getOffset();
    int output = 0;
    parameter *current_param = NULL;
    unsigned int word;

    for (size_t i = 0; i < len_; i++) {
        input.read((char *) &word, 4);
        if (input.eof()) { break; }
        output++;

        parameter *param = findParameter(params, firstValue + i);
        if (param) {
            current_param = param;
        }

        if (current_param) {
            current_param->values.push_back(word);
        }
    }

    input.close();

    return output;
}

#ifdef USE_ROOT_OUTPUT
void closeFile(TFile *f_){
    if(!f_){ return; }
    f_->Close();
}
#else

void closeFile(std::ofstream &f_) {
    f_.close();
}

#endif

void help(char *prog_name_) {
    std::cout << "  SYNTAX: " << prog_name_
              << " <varFile> <setFile> <startMod> <stopMod> [output]\n";
}

int main(int argc, char *argv[]) {
    if (argc > 1 &&
        (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        help(argv[0]);
        return 0;
    } else if (argc < 5) {
        std::cout << " Invalid number of arguments to " << argv[0]
                  << ". Expected 4, received " << argc - 1 << ".\n";
        help(argv[0]);
        return 1;
    }

    int start_mod = atoi(argv[3]);
    int stop_mod = atoi(argv[4]);
    if (start_mod < -1) {
        std::cout << " ERROR! Invalid start module specification (" << start_mod
                  << ").\n";
        return -1;
    } else if (stop_mod < -1) {
        std::cout << " ERROR! Invalid stop module specification (" << stop_mod
                  << ").\n";
        return -1;
    }

    // Set the input filenames.
    std::string varFilename = std::string(argv[1]);
    std::string setFilename = std::string(argv[2]);

    // Get the output filename.
    std::string outFilename;
    if (argc > 5) { outFilename = std::string(argv[5]); }
    else {
#ifdef USE_ROOT_OUTPUT
        outFilename = "params.root";
#else
        outFilename = "params.dat";
#endif
    }

    // Open the output file.
#ifdef USE_ROOT_OUTPUT
    TFile *f = new TFile(outFilename.c_str(), "RECREATE");
    if(!f->IsOpen()){
        std::cout << " ERROR! Failed to open output root file '" << outFilename << "'!\n";
        return 1;
    }
    TNamed named1("varFile", varFilename.c_str());
    TNamed named2("setFile", setFilename.c_str());
    named1.Write();
    named2.Write();
#else
    std::ofstream f(outFilename.c_str());
    if (!f.good()) {
        std::cout << " ERROR! Failed to open output file '" << outFilename
                  << "'!\n";
        return 1;
    }
    f << "varFile = " << varFilename << "\n";
    f << "setFile = " << setFilename << "\n";
#endif

    // Read the .var file.
    std::vector<parameter> params;
    if (readVarFile(params, varFilename.c_str())) {
        std::cout << " Successfully read " << params.size()
                  << " entries from .var file.\n";
    } else {
        std::cout << " ERROR! Failed to read dsp .var file '" << varFilename
                  << "'!\n";
        closeFile(f);
        return 1;
    }

    // Read the .set file.
    int readEntries;
    for (int i = start_mod; i <= stop_mod; i++) {
        for (std::vector<parameter>::iterator iter = params.begin();
             iter != params.end(); iter++) {
            iter->values.clear();
        }

        readEntries = readSetFile(params, setFilename.c_str(), i * READ_SIZE);
        if (readEntries > 0) {
            std::cout << " Successfully read " << readEntries
                      << " words from .set file for module " << i << ".\n";
        } else if (readEntries == 0) {
            std::cout << " ERROR! Read zero entries from .set file for module "
                      << i << "!\n";
            continue;
        } else {
            std::cout << " ERROR! Failed to read from .set file '"
                      << setFilename << "'!\n";
            closeFile(f);
            return 1;
        }

        std::stringstream stream;
        stream << "MODULE";
        if (i < 10)
            stream << "0";
        stream << i;

#ifdef USE_ROOT_OUTPUT
        f->mkdir(stream.str().c_str());
#else
        f
                << "###############################################################################\n";
        f << "##                                 " << stream.str()
          << "                                  ##\n";
        f
                << "###############################################################################\n";
#endif

        // Write values to the output file.
        for (std::vector<parameter>::iterator iter = params.begin();
             iter != params.end(); iter++) {
#ifdef USE_ROOT_OUTPUT
            if(iter->values.size() > 1){
                std::string dirname = stream.str()+"/"+iter->getName();
                f->mkdir(dirname.c_str());
                f->cd(dirname.c_str());
            }
            else{ f->cd(stream.str().c_str()); }
            iter->write(f);
#else
            f << iter->print();
#endif
        }
    }

    closeFile(f);
    std::cout << "  Done! Wrote output file '" << outFilename << "'.\n";

    return 0;
}
