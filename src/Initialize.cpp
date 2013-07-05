/*! \file DeclareHistogram.cpp
 * 
 * C++ wrapper to upak's hd1d/hd2d functions
 * David Miller, Aug. 2009
 */

#include <iostream>
#include <stdexcept>

#include "DetectorDriver.hpp"
#include "Globals.hpp"


// DAMM initialization call
extern "C" void drrmake_();
// DAMM declaration wrap-up call
extern "C" void endrr_();

/*! This function defines the histograms to be used in the analysis */
extern "C" void drrsub_(unsigned int& iexist)
{
    try {
        drrmake_();

        /** The DetectorDriver constructor will load processors
         *  from the xml configuration file upon first call.
         *  The DeclarePlots function will instantiate the DetectorLibrary
         *  class which will read in the "map" of channels.
         *  Subsequently the raw histograms, the diagnostic histograms 
         *  and the processors and analyzers plots are declared.
         *
         *  Note that in the PixieStd the Init function of DetectorDriver
         *  is called upon first buffer. This include reading in the
         *  calibration and walk correction factors.
         */
        DetectorDriver::get()->DeclarePlots();

        endrr_(); 
    } catch (std::exception &e) {
        // Any exceptions will be intercepted here
        std::cout << "Exception caught at Initialize:" << std::endl;
        std::cout << "\t" << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}
