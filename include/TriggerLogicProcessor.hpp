/** \file TriggerLogicProcessor.hpp
 *
 * class to handle logic signals and show "trigger" points
 * derived originally from MTC processor
 */

#ifndef __TRIGGERLOGICPROCESSOR_HPP_
#define __TRIGGERLOGICPROCESSOR_HPP_

#include "LogicProcessor.hpp"

class TriggerLogicProcessor : public LogicProcessor {
 private:
    int plotSize;
 public:
    TriggerLogicProcessor();
    virtual void DeclarePlots(void);
    virtual bool Process(RawEvent &event);
};

#endif // __TRIGGERLOGICPROCESSOR_HPP_
