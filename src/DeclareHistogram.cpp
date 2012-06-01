/*! \file DeclareHistogram.cpp
 * 
 * C++ wrapper to upak's hd1d/hd2d functions
 * David Miller, Aug. 2009
 */

#include <string>

#include <cstring>

#include "DetectorDriver.h"
#include "damm_plotids.h"

using namespace std;

// DAMM initialization call
extern "C" void drrmake_();
// DAMM declaration wrap-up call
extern "C" void endrr_();

/*! This function defines the histograms to be used in the analysis */
extern "C" void drrsub_(unsigned int& iexist)
{
    using namespace dammIds::diagnostic;

    extern DetectorDriver driver;

    extern Plots rawHisto;
    drrmake_(); // initialize things

    driver.DeclarePlots();

    rawHisto.DeclareHistogram1D(D_HIT_SPECTRUM, S7, "channel hit spectrum");
    rawHisto.DeclareHistogram1D(D_SUBEVENT_GAP, SE, "time btwn chan-in event,10ns bin");
    rawHisto.DeclareHistogram1D(D_EVENT_LENGTH, SE, "time length of event, 10 ns bin");
    rawHisto.DeclareHistogram1D(D_EVENT_GAP, SE, "time between events, 10 ns bin");
    rawHisto.DeclareHistogram1D(D_EVENT_MULTIPLICITY, S7, "number of channels in event");
    rawHisto.DeclareHistogram1D(D_BUFFER_END_TIME, SE, "length of buffer, 1 ms bin");
    rawHisto.DeclareHistogram2D(DD_RUNTIME_SEC, SE, S6, "run time - s");
    rawHisto.DeclareHistogram2D(DD_DEAD_TIME_CUMUL, SE, S6, "dead time - cumul");
    rawHisto.DeclareHistogram2D(DD_BUFFER_START_TIME, SE, S6, "dead time - 0.1%");
    rawHisto.DeclareHistogram2D(DD_RUNTIME_MSEC, SE, S7, "run time - ms");
    rawHisto.DeclareHistogram1D(D_NUMBER_OF_EVENTS, S4, "event counter");
    
    endrr_(); // wrap things up
}
