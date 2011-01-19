/**
 *   \file Trace.cpp
 *   
 *   Implement how to do our usual tricks with traces
 */

#include <algorithm>
#include <iostream>
#include <numeric>

#include "StatsAccumulator.h"
#include "Trace.h"

using namespace std;

/**
 * Defines how to implement a trapezoidal filter characterized by two
 * moving sum windows of width risetime separated by a length gaptime.
 * Filter is calculated from channels lo to hi.
 */
void Trace::TrapezoidalFilter(Trace &filter, 
			      const TrapezoidalFilterParameters &parms,
			      unsigned int lo, unsigned int hi) const
{
    filter.assign(lo, 0);
    
    //! check if we're going to do something bad here

    for (unsigned int i = lo; i < hi; i++) {
	int leftSum = accumulate(begin() + i - parms.GetSize(),
				 begin() + i - parms.GetRiseSamples() - parms.GetGapSamples(), 0);
	int rightSum = accumulate(begin() + i - parms.GetGapSamples(), begin() + i, 0);
	filter.push_back(rightSum - leftSum);
    }
}

double Trace::DoBaseline(unsigned int lo, unsigned int numBins)
{
    if (size() < lo + numBins) {
	cerr << "Bad range in baseline calculation." << endl;
	return NAN;
    }

    unsigned int hi = lo + numBins;

    if (baselineLow == lo && baselineHigh == hi)
	return GetValue("baseline");

    StatsAccumulator stats = accumulate(begin() + lo, begin() + hi, StatsAccumulator());
    InsertValue("baseline", stats.GetMean());
    InsertValue("sigmaBaseline", stats.GetStdDev());

    baselineLow  = lo;
    baselineHigh = hi;

    return stats.GetMean();
}

unsigned int Trace::FindMaxInfo(unsigned int lo, unsigned int numBins)
{
    if(size() < lo + numBins) {
	return U_DELIMITER;
    }

    unsigned int hi = lo + numBins;
    Trace::const_iterator itTrace = max_element(begin() + lo, end() - hi);
    
    // !These are getting set to the doubleTraceData map, not sure why - SVP
    InsertValue("maxpos", int(itTrace-begin()));
    InsertValue("maxval", int(*itTrace));

    return (itTrace-begin());
}
