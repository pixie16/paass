/*! \file DeclareHistogram.cpp
 * 
 * C++ wrapper to upak's hd1d/hd2d functions
 * David Miller, Aug. 2009
 */

#include <string>

#include <cstring>

#include "DetectorDriver.hpp"
#include "DammPlotIds.hpp"

using namespace std;

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
