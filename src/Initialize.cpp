/*! \file DeclareHistogram.cpp
 * 
 * C++ wrapper to upak's hd1d/hd2d functions
 * David Miller, Aug. 2009
 */

#include "PathHolder.hpp"
#include "MapFile.hpp"
#include "DetectorDriver.hpp"

#include <iostream>

// DAMM initialization call
extern "C" void drrmake_();
// DAMM declaration wrap-up call
extern "C" void endrr_();

/*! This function defines the histograms to be used in the analysis */
extern "C" void drrsub_(unsigned int& iexist)
{
    try {
        PathHolder* conf_path = new PathHolder("config.txt");
        delete conf_path;
        MapFile theMapFile = MapFile();
    } catch (exception &e) {
        // Any exception in opening files (config.txt and map2.txt)
        // will be intercepted here
        cout << "Exception caught at Initialize:" << endl;
        cout << "\t" << e.what() << endl;
        exit(EXIT_FAILURE);
    }
    drrmake_();
    DetectorDriver::get()->DeclarePlots(theMapFile);
    endrr_(); 
}
