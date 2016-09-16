#include <iostream>

#include "Unpacker.hpp"

Unpacker pixieUnpacker;

extern "C" void startup_()
{
	// Do some startup stuff.
}

// Catch the exit call from scanor and clean up c++ objects CRT
extern "C" void cleanup_()
{
	// Do some cleanup.
	std::cout << "\nCleaning up..\n";
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
extern "C" void hissub_(unsigned short *sbuf[], unsigned short *nhw)
{
	// Assign sbuf variable to local variable for use in function.
	unsigned int *buf= (unsigned int*)sbuf;

	// Process the data.
	pixieUnpacker.ReadSpill(buf, nhw[0] / 2);
}

//! DAMM initialization call
extern "C" void drrmake_();

//! DAMM declaration wrap-up call
extern "C" void endrr_();

/** Do banana gating using ban files args are the Banana number in the ban file,
 * the x-value to test, and the y-value to test.
 * \return true if the x,y pair is inside the banana gate */
extern "C" bool bantesti_(const int &, const int &, const int &);

/** create a DAMM 1D histogram
 * args are damm id, half-words per channel, param length, hist length,
 * low x-range, high x-range, and title
 */
extern "C" void hd1d_(const int &, const int &, const int &, const int &,
		      const int &, const int &, const char *, int);

/** create a DAMM 2D histogram
 * args are damm id, half-words per channel, x-param length, x-hist length
 * low x-range, high x-range, y-param length, y-hist length, low y-range
 * high y-range, and title
 */
extern "C" void hd2d_(const int &, const int &, const int &, const int &,
		      const int &, const int &, const int &, const int &,
		      const int &, const int &, const char *, int);

/*! Defines the main interface with the SCANOR library, the program essentially
 * starts here.
 * \param [in] iexist : unused paramter from SCANOR call
 */
extern "C" void drrsub_(uint32_t& iexist) {
    try {
        drrmake_();

		// Initialize some histograms.
		// At least one so the damn thing will run :P
		hd1d_(8000, 1, 1024, 512, 0, 10, "Run DAMM you!", 14);

        endrr_();
    } catch (std::exception &e) {
        // Any exceptions will be intercepted here
        std::cout << "Exception caught at Initialize:" << std::endl;
        std::cout << "\t" << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}
