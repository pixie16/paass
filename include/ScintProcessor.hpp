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

class ScintProcessor : public EventProcessor,
		       public TimingInformation
{
public:
    ScintProcessor(); // no virtual c'tors
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);
    // nice and simple raw derived class
private:
   virtual void LiquidAnalysis(RawEvent &event);
   unsigned int counter;
};

#endif // __SCINTPROCSSEOR_HPP_
