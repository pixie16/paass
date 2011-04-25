/** \file LogicProcessor.h
 *
 * class to handle logic signals 
 * derived originally from MTC processor
 */

#ifndef __LOGICPROCESSOR_H_
#define __LOGICPROCESSOR_H_

#include <vector>

#include "EventProcessor.h"

class LogicProcessor : public EventProcessor {
 protected:
    std::vector<double> lastStartTime; //< time of last leading edge
    std::vector<double> lastStopTime;  //< time of last trailing edge
    std::vector<bool>   logicStatus;   //< current level of the logic signal
 public:
    LogicProcessor();
    virtual void DeclarePlots(void) const;
    virtual bool Process(RawEvent &event);
    virtual bool LogicStatus(size_t loc) {
      return logicStatus.at(loc);
    };
};

#endif // __LOGICPROCESSOR_H_
