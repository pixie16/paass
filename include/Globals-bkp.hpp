/** \file Globals.hpp
 * \brief constant parameters used in pixie16 analysis
 */

#ifndef __GLOBALS_HPP_
#define __GLOBALS_HPP_

#include <string>

#include <cstdlib>
#include <stdint.h>

//? move these
const int MAX_PAR = 32000; //< maximum limit for calibrations

/* More verbose initialization */
namespace verbose {
    const bool MAP_INIT = false;
    const bool CALIBRATION_INIT = false;
    const bool CORRELATOR_INIT = false;
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
    /** 3Hen -> 50 us, LeRIBBS -> 3 us */
    const double eventInSeconds = 3e-6;
    /** The time width of an event in units of pixie16 clock ticks */
    const int eventWidth = eventInSeconds / pixie::clockInSeconds;

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
    const double gammaThreshold = 10.0;
    /** Subevent length for addback construction (in seconds). */
    const double subEventWindow = 100e-9;
    /** Low energy cut on addback, in keV. */
    const double addbackEnergyCut = 25;
    /** Prompt gamma-beta coincidence window (in seconds).*/
    const double gammaBetaLimit = 200e-9;

    /**
     * Beta scintilators
     */
    const double betaThreshold = 10.0;

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

#endif // __GLOBALS_HPP_
