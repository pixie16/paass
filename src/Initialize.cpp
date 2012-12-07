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
    PathHolder* conf_path = new PathHolder("config.txt");
    delete conf_path;
    MapFile theMapFile = MapFile();
    drrmake_();
    DetectorDriver::get()->DeclarePlots(theMapFile);
    endrr_(); 
}
