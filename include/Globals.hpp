/*! \file Globals.hpp
  \brief constant parameters used in pixie16 analysis
  
*/

#ifndef __GLOBALS_HPP_
#define __GLOBALS_HPP_

#include <vector>
#include <sstream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <stdint.h>
#include "Exceptions.hpp"

//? move these
const int MAX_PAR = 32000; //< maximum limit for calibrations

/* More verbose initialization */
namespace verbose {
    const bool MAP_INIT = false;
    const bool CALIBRATION_INIT = false;
};

namespace pixie {
    typedef uint32_t word_t; //< a pixie word
    typedef uint16_t halfword_t; //< a half pixie word
    typedef uint32_t bufword_t; //< word in a pixie buffer

#ifdef REVF
    const double clockInSeconds = 8e-9; //< one pixie clock is 8 ns
    const double adcClockInSeconds = 4e-9; //< one ADC clock is 4 ns
    const double filterClockInSeconds = 8e-9; //< one filter clock is 8 ns
#else 
    const double clockInSeconds = 10e-9; //< one pixie clock is 10 ns
    const double adcClockInSeconds = 10e-9; //< one ADC clock is 10 ns
    const double filterClockInSeconds = 10e-9; //< one filter clock is 10 ns
#endif
   
    const size_t numberOfChannels = 16; //< number of channels in a module

    /** The time widht of an event in seconds.*/
    /** 3Hen -> 100 us, LeRIBBS -> 3 us */
    const double eventInSeconds = 100e-6;
    /** The time width of an event in units of pixie16 clock ticks */
    const int eventWidth = eventInSeconds / pixie::clockInSeconds;

    /** Energies from pixie16 are contracted by this number.
     * Was 2.0 for older LeRIBBS
     * changed to 4.0 for LeRIBBS experiment (93Br)
     * Set to 1.0, looks like ADC range is 16K
     * */
    const double energyContraction = 1.0; 
};

/**
 * Put here all detectors specific constants
 * e.g (thresholds, time windows, etc.)
 * */
namespace detectors {
    /**
     * Ge clovers
     */
    /** Gamma energy low threshold. */
    const double gammaThreshold = 20.0;
    /* Mismatch ratio of low and high gain in Clovers */
    const double geLowRatio = 1.5;
    const double geHighRatio = 3.0;
    /** Subevent length for addback construction (in seconds). */
    const double subEventWindow = 100e-9;
    /** Low energy cut on addback, in keV. */
    const double addbackEnergyCut = 25;
    /** Prompt gamma-beta coincidence window (in seconds).*/
    const double gammaBetaLimit = 200e-9;
    /** Prompt gamma-gamma coincidence window (in seconds).*/
    const double gammaGammaLimit = 200e-9;

    /** Early/late gamma-gamma coincidence window (in seconds).*/
    const double earlyLowLimit = 1.0;
    const double earlyHighLimit = 1.5;

    /**
     * 3Hen
     */
    const double neutronLowLimit = 2500.0;
    const double neutronHighLimit = 3400.0;

};

/** buffer and module data are terminated with a "-1" value
 *   also used to indicate when a quantity is out of range or peculiar
 *   this should theoretically be the same as UINT_MAX in climits header
 */
const pixie::word_t U_DELIMITER = (pixie::word_t)-1;

namespace readbuff {
    const int STATS = -10;
    const int ERROR = -100;
}

const double emptyValue = -9999.; //< a default number to set values to
const std::string emptyString = ""; //< an empty string for blank references

const pixie::word_t clockVsn = 1000; ///< an arbitrary vsn used to pass clock data

const size_t maxConfigLineLength = 100;

/** Some common string operations */
namespace strings {
    /** Converts string to double or throws an exception if not 
        * succesful */
    inline double to_double (std::string s) { 
        std::istringstream iss(s);
        double value;
        if (!(iss >> value)) {
            std::stringstream ss;
            ss << "strings::to_double: Could not convert string '" 
               << s << "' to double" << std::endl;
            throw ConversionException(ss.str());
        }
        return value;
    }

    /** Converts string to int or throws an exception if not 
        * succesful */
    inline int to_int (std::string s) {
        std::istringstream iss(s);
        int value;
        if (!(iss >> value)) {
            std::stringstream ss;
            ss << "strings::to_int: Could not convert string '" 
               << s << "' to int" << std::endl;
            throw ConversionException(ss.str());
        }
        return value;
    }

    /** Converts string to bool (True, true, 1 and False, false, 0) are 
        * accepted; throws an exception if not succesful. Notice tolower
        * will work only with ascii, not with utf-8, but shouldn't be a 
        * problem for true and false words. */
    inline bool to_bool (std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (s == "true" || s == "1")
            return true;
        else if (s == "false" || s == "0")
            return false;
        else {
            std::stringstream ss;
            ss << "strings::to_bool: Could not convert string '" 
               << s << "' to bool" << std::endl;
            throw ConversionException(ss.str());
        }
    }

    /** Tokenizes the string, splitting it on given delimiter.
        * delimiters are removed from returned vector of tokens.*/
    inline std::vector<std::string> tokenize(std::string str, std::string delimiter) {
        std::string temp;
        std::vector<std::string> tokenized;
        while (str.find(delimiter) != std::string::npos) {
            size_t pos = str.find(delimiter);
            temp = str.substr(0, pos);
            str.erase(0, pos + 1);
            tokenized.push_back(temp);
        }
        tokenized.push_back(str);
        return tokenized;
    }
};

#endif // __GLOBALS_HPP_
