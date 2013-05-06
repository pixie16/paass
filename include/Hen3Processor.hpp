/** \file Hen3Processor.h
 *
 * Processor for 3Hen detectors
 */

#ifndef __HEN3PROCESSOR_H_
#define __HEN3PROCESSOR_H_ 1

#include "EventProcessor.hpp"

class Hen3Processor : public EventProcessor
{
public:
    Hen3Processor(); 
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);
protected:
    EventData BestBetaForNeutron(double nTime);
    static double const cycleTimePlotResolution_ = 1e-3;
    static double const diffTimePlotResolution_ = 1e-6;
};

#endif // __HEN3PROCESSOR_H_
