/** \file MtcProcessor.h
 *
 * class to handle mtc signals
 */

#ifndef __MTCPROCESSOR_H_
#define __MTCPROCESSOR_H_

#include "EventProcessor.h"

class MtcProcessor : public EventProcessor {
 public:
    MtcProcessor();
    virtual void DeclarePlots(void) const;
    virtual bool Process(RawEvent &event);
 private:
    double prevTime0; //< time of last leading edge
    double prevTime1; //< time of last trailing edge
};

#endif // __MTCPROCESSOR_H_
