/** \file TwoChanTimingProcessor.hpp
 * \brief Class to perform simple two channel timing experiments
 * \author S. V. Paulauskas
 * \date July 10, 2009
 */

#ifndef __TWOCHANTIMINGPROCESSOR_HPP_
#define __TWOCHANTIMINGPROCESSOR_HPP_

#include "EventProcessor.hpp"

//! A class that handles a simple timing experiment
class TwoChanTimingProcessor : public EventProcessor {
public:
    /** Default Constructor */
    TwoChanTimingProcessor();

    /** Default Destructor */
    ~TwoChanTimingProcessor();


    /** Performs the main processsing, which may depend on other processors
    * \param [in] event : the event to process
    * \return true if processing was successful */
    bool Process(RawEvent &event);
};

#endif // __TWOCHANTIMINGPROCESSOR_HPP_
