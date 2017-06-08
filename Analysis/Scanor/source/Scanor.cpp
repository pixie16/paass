///@file ScanorInterface.cpp
///@authors C. R. Thornsberry, S. V. Paulauskas
///@date September 16, 2016

#include "Scanor.hpp"
#include "ScanorInterface.hpp"

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
extern "C" void hissub_(unsigned short *sbuf[], unsigned short *nhw) {
    ScanorInterface::get()->Hissub(sbuf, nhw);
}
