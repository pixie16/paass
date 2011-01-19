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

// forward declaration
class TrapezoidalFilterParameters;

// use an alias in this file to make things a bit more readable
namespace {
    typedef class TrapezoidalFilterParameters TFP;
}

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
    void TrapezoidalFilter(Trace &filter, const TFP &parms,
			   unsigned int lo = 0) const {
	TrapezoidalFilter( filter, parms, lo, size() );
    };
    void TrapezoidalFilter(Trace &filter, const TFP &parms,
			   unsigned int lo, unsigned int hi) const;
    void InsertValue(std::string name, double value) {
	doubleTraceData.insert(make_pair(name,value));
    }
    void InsertValue(std::string name, int value) {
	intTraceData.insert(make_pair(name,value));	
    }
    void SetValue(std::string name, double value) {
	if (doubleTraceData.count(name) > 0) 
	    doubleTraceData[name] = value;
	else
	    InsertValue(name,value);
    }
    void SetValue(std::string name, int value) {
	if (intTraceData.count(name) > 0)
	    intTraceData[name] = value;
	else
	    InsertValue(name,value);
    }
    bool HasValue(std::string name) {
	return (doubleTraceData.count(name) > 0 ||
		intTraceData.count(name) > 0);
    }
    double GetValue(std::string name) const {
	if (doubleTraceData.count(name) > 0)
	    return (*doubleTraceData.find(name)).second;
	if (intTraceData.count(name) > 0)
	    return (*intTraceData.find(name)).second;
	return NAN;
    }
    double DoBaseline(unsigned int lo = 0, unsigned int numBins = numBinsBaseline);
    unsigned int FindMaxInfo(unsigned int lo = 0, unsigned int numBins = numBinsBaseline);
};

/** Parameters for your typical trapezoidal filter */
class TrapezoidalFilterParameters
{
 private:
    Trace::size_type gapSamples;
    Trace::size_type riseSamples;
    
    double tau;
 public:
    TrapezoidalFilterParameters(int gap, int rise, double t = NAN) :
	gapSamples(gap), riseSamples(rise), tau(t) {};
    TrapezoidalFilterParameters(const TFP &x) :
	gapSamples(x.gapSamples), riseSamples(x.riseSamples),
	tau(x.tau) {};
    const TFP& operator=(const TFP &right) {
	gapSamples = right.gapSamples;
	riseSamples = right.riseSamples;
	tau = right.tau;

	return (*this);
    }
    int GetGapSamples(void) const  {return gapSamples;}
    int GetRiseSamples(void) const {return riseSamples;}
    int GetSize(void) const        {return 2*riseSamples + gapSamples;}
    double GetTau(void) const      {return tau;}
};


#endif // __TRACE_H_
