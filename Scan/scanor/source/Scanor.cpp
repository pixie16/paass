///@file ScanorInterface.cpp
///@authors C. R. Thornsberry, S. V. Paulauskas
///@date September 16, 2016

#include <iostream>
#include <sstream>
#include <vector>

#include <cstdint>
#include <cstring>

#include "Scanor.hpp"
#include "ScanorInterface.hpp"

#define TOTALREAD 1000000
#define EXTERNAL_FIFO_LENGTH 131072
#define U_DELIMITER 0xFFFFFFFF

// Vector for storing command line arguments.
std::vector<std::string> fort_args;

int fortargc = 0;
char **fortargv = NULL;

// Get command line arguments from scanor.
extern "C" void addcmdarg_(char *arg_){
	std::string temparg = std::string(arg_);
	fort_args.push_back(temparg.substr(0, temparg.find_first_of(' '))); // Strip trailing whitespace.
}

// Generate an array of c-strings to mimic argc and argv.
// Kind of messy, but it works well. We can clean this up later.
extern "C" void finalizeargs_(){
	if(fortargv) return; // Only do this once.
	
	fortargc = (int)fort_args.size();
	fortargv = new char*[fortargc];
	
	size_t arglen;
	for(int i = 0; i < fortargc; i++){
		arglen = fort_args.at(i).length();
		fortargv[i] = new char[arglen+1];
		memcpy(fortargv[i], fort_args.at(i).data(), arglen);
		fortargv[i][arglen] = '\0';
	}
}

/** \fn extern "C" void hissub_(unsigned short *ibuf[],unsigned short *nhw)
 * \brief interface between scan and C++
 *
 * In a typical experiment, Pixie16 reads data from all modules when one module
 * has hit the maximum number of events which is programmed during experimental
 * setup.  This spill of data is then broken into smaller chunks for
 * transmitting across the network.  The hissub_ function takes the chunks
 * and reconstructs the spill.
 *
 * Summarizing the terminology:
 *  - Spill  - a readout of all Pixie16 modules
 *  - Buffer - the data from a specific Pixie16 module
 *  - Chunk  - the packet transferred from Pixie16 to the acquisition
 *
 * The hissub_ function is passed a pointer to an array with data (ibuf) and
 * the number of half words (nhw) contained in it.  This function is used with
 * the new Pixie16 readout (which is the default).  If the old Pixie16 readout
 * is used, the code should be recompiled without the newreadout flag in which
 * case this particular function is not used.
 * \param [in] ibuf : the array with the data
 * \param [in] nhw : the number of half words contained in the data buffer
*/
extern "C" void hissub_(unsigned short *sbuf[],unsigned short *nhw) {
    ScanorInterface::get()->Hissub(sbuf,nhw);
}

/*! Defines the main interface with the SCANOR library, the program essentially
 * starts here.
 * \param [in] iexist : unused paramter from SCANOR call
 */
extern "C" void drrsub_(uint32_t& iexist) {
    drrmake_();
    ScanorInterface::get()->Drrsub(iexist);
    endrr_();
}
