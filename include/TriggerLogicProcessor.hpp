/** \file TriggerLogicProcessor.hpp
 *
 * class to handle logic signals and show "trigger" points
 * derived originally from MTC processor
 */
#ifndef __TRIGGERLOGICPROCESSOR_HPP_
#define __TRIGGERLOGICPROCESSOR_HPP_

#include "LogicProcessor.hpp"

//! Trigger Logic Processing derived from LogicProcessor
class TriggerLogicProcessor : public LogicProcessor {
public:
    /** Default Constructor */
    TriggerLogicProcessor(void);

    /** Default destructor */
    ~TriggerLogicProcessor(){};

    /** Declare the plots for the processor */
    virtual void DeclarePlots(void);

    /** Process an event
    * \param [in] event : the event to process
    * \return True if the processing was successful */
    virtual bool Process(RawEvent &event);
private:
    int plotSize; //!< The size of the plots for the processor
};
#endif // __TRIGGERLOGICPROCESSOR_HPP_
