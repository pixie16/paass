/** \file TriggerLogicProcessor.h
 *
 * class to handle logic signals and show "trigger" points
 * derived originally from MTC processor
 */

#ifndef __TRIGGERLOGICPROCESSOR_H_
#define __TRIGGERLOGICPROCESSOR_H_

#include "LogicProcessor.h"

class TriggerLogicProcessor : public LogicProcessor {
 private:
    int plotSize;
 public:
    TriggerLogicProcessor();
    virtual void DeclarePlots(void) const;
    virtual bool Process(RawEvent &event);
};

#endif // __TRIGGERLOGICPROCESSOR_H_
