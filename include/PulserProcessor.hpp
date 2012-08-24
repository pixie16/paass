/** \file PulserProcessor.hpp
 * \brief Class for handling Pulser Signals
 */

#ifndef __PULSERPROCESSOR_HPP_
#define __PULSERPROCESSOR_HPP_

#include "EventProcessor.hpp"
#include "TimingInformation.hpp"

class PulserProcessor : public EventProcessor, 
			public TimingInformation
{
 public:
    PulserProcessor(); // no virtual c'tors
    virtual void DeclarePlots(void);
    virtual bool Process(RawEvent &event);
 protected:
    TimingDataMap pulserMap;
 private:
    virtual bool RetrieveData(RawEvent &event);
    virtual void AnalyzeData(void);
}; //Class PulserProcessor
#endif // __PULSERPROCESSOR_HPP_
