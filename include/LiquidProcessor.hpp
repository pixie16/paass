/** \file LiquidProcessor.hpp
 *
 * Processor for liquid scintillator detectors
 */

#ifndef __LIQUIDPROCESSOR_HPP_
#define __LIQUIDPROCESSOR_HPP_

#include "EventProcessor.hpp"
#include "HighResTimingData.hpp"
#include "Trace.hpp"

//! Processor for Liquid Scintillators - Deprecated
class LiquidProcessor : public EventProcessor {
public:
    /** Default Constructor */
    LiquidProcessor();
    /** Default Destructor */
    ~LiquidProcessor(){};
    /** Performs the preprocessing, which cannot depend on other processors
    * \param [in] event : the event to process
    * \return true if preprocessing was successful */
    virtual bool PreProcess(RawEvent &event);
    /** Process an event
    * \param [in] event : the event to process
    * \return true if the processing was successful */
    virtual bool Process(RawEvent &event);
    /** Declare plots for processor */
    virtual void DeclarePlots(void);
private:
    HighResTimingData timeInfo;//!< Instance of HighResTimingData to hold info
};
#endif // __LIQUIDPROCSSEOR_HPP_
