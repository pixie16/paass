/** \file LogicProcessor.h
 *
 * class to handle logic signals 
 * derived originally from MTC processor
 */

#ifndef __LOGICPROCESSOR_H_
#define __LOGICPROCESSOR_H_

#include "EventProcessor.h"

class LogicProcessor : public EventProcessor {
 public:
    LogicProcessor();
    virtual void DeclarePlots(void) const;
    virtual bool Process(RawEvent &event);
 protected:
    vector<double> lastStartTime; //< time of last leading edge
    vector<double> lastStopTime;  //< time of last trailing edge
};

#endif // __LOGICPROCESSOR_H_
