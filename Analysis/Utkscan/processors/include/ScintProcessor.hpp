/** \file ScintProcessor.hpp
 *
 * Processor for scintillator detectors
 *
 * KM 10/20/12:
 * Obsolete file, see ScintProcessor.cpp for details
 */
#ifndef __SCINTPROCESSOR_HPP_
#define __SCINTPROCESSOR_HPP_

#include "EventProcessor.hpp"
#include "Trace.hpp"

//! Obsoltete Class to handle scintillator events
class ScintProcessor : public EventProcessor {
public:
    /** Default Constructor */
    ScintProcessor();

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
    /** Analyze liquid scintillators in an event
    * \param [in] event : the event to process */
    virtual void LiquidAnalysis(RawEvent &event);

    unsigned int counter; //!< a counter for the class
};

#endif
