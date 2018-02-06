/** \file TtuTimingProcessor.hpp
 * \brief Class for handling TtuTiming Signals
 * \author S. V. Paulauskas
 * \date 10 July 2009
 */

#ifndef __TTUTIMINGPROCESSOR_HPP_
#define __TTUTIMINGPROCESSOR_HPP_

#include "EventProcessor.hpp"
#include "HighResTimingData.hpp"

//! A class to handle time differences between two pulsers
class TtuTimingProcessor : public EventProcessor {
 public:
    /** Default Constructor */
    TtuTimingProcessor();
    /** Default Destructor */
    ~TtuTimingProcessor(){};
    /** Declares the plots for the processor */
    virtual void DeclarePlots(void);
    /** Performs the main processsing, which may depend on other processors
    * \param [in] event : the event to process
    * \return true if processing was successful */
    virtual bool Process(RawEvent &event);
private:
    TimingMap pulserMap_; //!< map of the pulser events
};
#endif // __TTUTIMINGPROCESSOR_HPP_
