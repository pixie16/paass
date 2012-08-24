/** \file DeclareHistogram.cpp
 * \brief C++ wrapper to upak's hd1d/hd2d functions
 * \author David Miller
 * \date Aug. 2009
 */

#include "DetectorDriver.hpp"

// DAMM initialization call
extern "C" void drrmake_();
// DAMM declaration wrap-up call
extern "C" void endrr_();

/*! This function defines the histograms to be used in the analysis */
extern "C" void drrsub_(unsigned int& iexist)
{
    extern DetectorDriver driver;

    drrmake_(); // initialize things
    driver.DeclarePlots();
    endrr_(); // wrap things up
}
