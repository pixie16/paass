/*! \file param.h 
  \brief constant parameters used in pixie16 analysis
  
*/

#ifndef __PARAM_H_
#define __PARAM_H_

#include <cstdlib>

#include <stdint.h>

const size_t MAX_STRIP = 40; //< number of DSSD strips

const int MAX_PAR = 32000; //< maximum limit for calibrations

namespace pixie {
    typedef uint32_t word_t; //< a pixie word
    typedef uint16_t halfword_t; //< a half pixie word

    const double clockInSeconds = 10e-9; //< one pixie clock is 10 ns
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

#endif // __PARAM_H_
