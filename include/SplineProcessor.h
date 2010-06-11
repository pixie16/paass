/** \file SplineProcessor.h
 *
 * Definition for an event processor which examines pulses to determine
 *   timing between events using spline interpolation
 */
#ifndef __SPLINEPROCESSOR_H_
#define __SPLINEPROCESSOR_H_

#include <vector>

#include "EventProcessor.h"

//! Not the most descriptive definition for a global structure
struct data
{
    size_t n;
    double* y;
    double* sigma;
};

class SplineProcessor : public EventProcessor 
{
 private:
    /// bins used to calculate baseline statistics
    static const unsigned int numBinsBaseline = 15;
    /// sampling point at which the baseline measurements start mattering
    static const unsigned int baselineOffset = 24;

    int counter; //< counter for labelling traces 
 protected:
    ~SplineProcessor();
 public:
    SplineProcessor();

    virtual bool Init(DetectorDriver &driver);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void) const;
};

#endif // __SPLINEPROCESSOR_H_
