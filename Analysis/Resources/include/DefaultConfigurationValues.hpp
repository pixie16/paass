///@file DefaultConfigurationValues.cpp
///@brief Contains some default values for use in the scan codes in the event that a user forgets something in the
/// configuration file
///@author S. V. Paulauskas
///@date June 3, 2017

#ifndef PAASS_DEFAULTCONFIGURATIONVALUES_HPP
#define PAASS_DEFAULTCONFIGURATIONVALUES_HPP

namespace DefaultConfig {
    static const unsigned int waveformLow = 5;
    static const unsigned int waveformHigh = 10;

    static const unsigned int filterL = 100;
    static const unsigned int filterG = 100;
    static const unsigned int filterT = 1;

    static const double fitBeta = 0.254373;
    static const double fitGamma = 0.208072;

    static const double cfdF = 0.8;
    static const double cfdD = 1.;
    static const double cfdL = 0.;

    static const unsigned int discrimStart = 3;

    static const double baselineThreshold = 3.;
}

#endif //PAASS_DEFAULTCONFIGURATIONVALUES_HPP
