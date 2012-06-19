/*! \file Globals.hpp
  \brief constant parameters used in pixie16 analysis
  
*/

#ifndef __GLOBALS_HPP_
#define __GLOBALS_HPP_

#include <string>

#include <cstdlib>
#include <stdint.h>

//? move these
const int MAX_PAR = 32000; //< maximum limit for calibrations

namespace pixie {
    typedef uint32_t word_t; //< a pixie word
    typedef uint16_t halfword_t; //< a half pixie word
    typedef uint32_t bufword_t; //< word in a pixie buffer

#ifdef REVF
    const double clockInSeconds = 8e-9; //< one pixie clock is 10 ns
    const double adcClockInSeconds = 4e-9; //< one ADC clock is 10 ns
    const double filterClockInSeconds = 8e-9; //< one filter clock is 10 ns
#else 
    const double clockInSeconds = 10e-9; //< one pixie clock is 10 ns
    const double adcClockInSeconds = 10e-9; //< one ADC clock is 10 ns
    const double filterClockInSeconds = 10e-9; //< one filter clock is 10 ns
#endif
   
    const size_t numberOfChannels = 16; //< number of channels in a module
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
