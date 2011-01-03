/** \file WaveformProcessor.h
 * 
 * Class for handling position-sensitive mcp signals
 */

#ifndef __WAVEFORMPROCESSOR_H_
#define __WAVEFORMPROCESSOR_H_

#include "EventProcessor.h"

class WaveformProcessor : public EventProcessor
{
 public:
    WaveformProcessor(); // no virtual c'tors
    virtual void DeclarePlots(void) const;
    virtual bool Process(RawEvent &event);

    struct FitData {
	size_t n;
	double * y;
	double * sigma;
    };

 private:
    int counter, TrcCtr, counter_1;
};
#endif // __WAVEFORMPROCESSOR_H_
