/** \file PulserProcessor.h
 * 
 * Class for handling Pulser Signals
 */

#ifndef __PULSERPROCESSOR_H_
#define __PULSERPROCESSOR_H_

#include <map>
#include <string>

#include "EventProcessor.h"
#include "RawEvent.h"
#include "Trace.h"

class PulserProcessor : public EventProcessor
{
 public:
    PulserProcessor(); // no virtual c'tors
    virtual void DeclarePlots(void);
    virtual bool Process(RawEvent &event);

 private:
    struct PulserData
    {
	PulserData(std::string type ="");
	PulserData(ChanEvent *chan);
	
	unsigned int location;
	double maxValue;
	double phase;
	double trcQDC;
	double stdDevBaseline;
	double aveBaseline;
	double highResTime;
	int    maxPos;
	Trace  trace;
    };
    
    virtual bool RetrieveData(RawEvent &event);
    virtual void AnalyzeData(void);
    virtual bool GoodDataCheck(const PulserData& DataCheck);

    std::map<std::string, struct PulserData> pulserMap;
}; //Class PulserProcessor
#endif // __PULSERPROCESSOR_H_
