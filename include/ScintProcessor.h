/** \file ScintProcessor.h
 *
 * Processor for scintillator detectors
 */

#ifndef __SCINTPROCESSOR_H_
#define __SCINTPROCESSOR_H_

#include "EventProcessor.h"

class ScintProcessor : public EventProcessor 
{
 public:
    ScintProcessor(); // no virtual c'tors
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void) const;
    // nice and simple raw derived class
};

#endif // __SCINTPROCSSEOR_H_
