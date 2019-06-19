/** \file LiquidScintProcessor.hpp
 *
 * Processor for liquid scintillator detectors
 * @authors K. Miernik, S. V. Paulauskas
 */
#ifndef __LIQUIDSCINTPROCESSOR_HPP_
#define __LIQUIDSCINTPROCESSOR_HPP_

#include "EventProcessor.hpp"
#include "ProcessorRootStruc.hpp"
#include "DetectorDriver.hpp"
#include "Trace.hpp"

//! Class to handle liquid scintillator
class LiquidScintProcessor : public EventProcessor {
public:
    /** Default Constructor */
    LiquidScintProcessor();

    /** Default Destructor */
    ~LiquidScintProcessor() {};

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
    unsigned int counter;//!< A counter for counting...
    processor_struct::VANDLES LSstruc;
};

#endif // __LIQUIDSCINTPROCSSEOR_HPP_
