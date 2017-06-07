///@file DefaultConfigurationValues.cpp
///@brief Contains some default values for use in the scan codes in the event that a user forgets something in the
/// configuration file
///@author S. V. Paulauskas
///@date June 3, 2017

#ifndef PAASS_DEFAULTCONFIGURATIONVALUES_HPP
#define PAASS_DEFAULTCONFIGURATIONVALUES_HPP

namespace DefaultConfig {
    ///These are used when reading /Configuration/Map/Module/Channel/Trace
    static const unsigned int waveformLow = 5;
    static const unsigned int waveformHigh = 10;
    static const unsigned int discrimStart = 3;
    static const double baselineThreshold = 3.;

    ///These are used when reading /Configuration/Map/Module/Channel/Trace/Filter/Trigger or .../Filter
    static const unsigned int filterL = 100;
    static const unsigned int filterG = 100;
    static const unsigned int filterT = 1;

    ///These are used when reading /Configuration/Map/Module/Channel/Fit
    static const double fitBeta = 0.254373;
    static const double fitGamma = 0.208072;

    ///These are used when reading /Configuration/Map/Module/Channel/Cfd
    static const double cfdF = 0.8;
    static const double cfdD = 1.;
    static const double cfdL = 0.;
}

#endif //PAASS_DEFAULTCONFIGURATIONVALUES_HPP
