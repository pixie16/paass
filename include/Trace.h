/* 
 * \file Trace.h
 *
 * A simple class to store the traces.
 * Used instead of a typedef so additional functionality can be added later.
 */

#ifndef __TRACE_H_
#define __TRACE_H_

#include <map>
#include <string>
#include <vector>

#include <cmath>

#include "param.h"

#ifndef NAN
#include <limits>
#define NAN (numeric_limits<float>::quiet_NaN())
#endif

/** Parameters for your typical trapezoidal filter */
class TrapezoidalFilterParameters
{
 private:
    int gapSamples;
    int riseSamples;
    
    double tau;
 public:
    TrapezoidalFilterParameters(int gap, int rise, double t) :
	gapSamples(gap), riseSamples(rise), tau(t) {};
    int GetGapSamples(void) const  {return gapSamples;}
    int GetRiseSamples(void) const {return riseSamples;}
    int GetSize(void) const        {return 2*riseSamples + gapSamples;}
    double GetTau(void) const      {return tau;}
};

/**
   Store the information for a trace
 */
class Trace : public std::vector<int>
{
 private:
    static const unsigned int numBinsBaseline = 15;
    unsigned int baselineLow; 
    unsigned int baselineHigh;

    std::map<std::string, double> doubleTraceData;
    std::map<std::string, int>    intTraceData;
 public:
    Trace() : std::vector<int>() {baselineLow = baselineHigh = U_DELIMITER; };
    // an automatic conversion
    Trace(const std::vector<int> &x) : std::vector<int>(x) {
	baselineLow = baselineHigh = U_DELIMITER;
    };
    void TrapezoidalFilter(Trace &filter, const TrapezoidalFilterParameters &parms,
			   unsigned int lo = 0) const {
	TrapezoidalFilter( filter, parms, lo, size() );
    };
    void TrapezoidalFilter(Trace &filter, const TrapezoidalFilterParameters &parms,
			   unsigned int lo, unsigned int hi) const;
    void InsertValue(std::string name, double value) {
	doubleTraceData.insert(make_pair(name,value));
    }
    void InsertValue(std::string name, int value) {
	intTraceData.insert(make_pair(name,value));	
    }
    bool HasValue(std::string name) {
	return (doubleTraceData.count(name) > 0 ||
		intTraceData.count(name) > 0);
    }
    double GetValue(std::string name) {
	if (doubleTraceData.count(name) > 0)
	    return doubleTraceData[name];
	if (intTraceData.count(name) > 0)
	    return intTraceData[name];
	return NAN;
    }
    double DoBaseline(unsigned int lo = 0, unsigned int numBins = numBinsBaseline);
    unsigned int FindMaxInfo(unsigned int lo = 0, unsigned int numBins = numBinsBaseline);
};


#endif // __TRACE_H_
