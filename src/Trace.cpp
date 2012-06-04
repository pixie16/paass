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

#include "damm_plotids.h"

using namespace std;

namespace dammIds {
    namespace trace {
    }
} // trace namespace
    
const Trace emptyTrace; ///< an empty trace for const references to point to

/**
 * Defines how to implement a trapezoidal filter characterized by two
 * moving sum windows of width risetime separated by a length gaptime.
 * Filter is calculated from channels lo to hi.
 */
void Trace::TrapezoidalFilter(Trace &filter, 
			      const TrapezoidalFilterParameters &parms,
			      unsigned int lo, unsigned int hi) const
{
    // don't let the filter work outside of its reasonable range
    lo = max(lo, (unsigned int)parms.GetSize());

    filter.assign(lo, 0);
    
    //! check if we're going to do something bad here
    for (unsigned int i = lo; i < hi; i++) {
	int leftSum = accumulate(begin() + i - parms.GetSize(),
				 begin() + i - parms.GetRiseSamples() - parms.GetGapSamples(), 0);
	int rightSum = accumulate(begin() + i - parms.GetRiseSamples(), begin() + i, 0);
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
    SetValue("baseline", stats.GetMean());
    SetValue("sigmaBaseline", stats.GetStdDev());

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

void Trace::Plot(int id)
{
    for (size_type i=0; i < size(); i++) {
	histo.Plot(id, i, 1, at(i));
    }
}

void Trace::Plot(int id, int row)
{
    for (size_type i=0; i < size(); i++) {
	histo.Plot(id, i, row, at(i));
    }
}

void Trace::ScalePlot(int id, double scale)
{
    for (size_type i=0; i < size(); i++) {
	histo.Plot(id, i, 1, abs(at(i)) / scale);
    }
}

void Trace::ScalePlot(int id, int row, double scale)
{
    for (size_type i=0; i < size(); i++) {
	histo.Plot(id, i, row, abs(at(i)) / scale);
    }
}

void Trace::OffsetPlot(int id, double offset)
{
    for (size_type i=0; i < size(); i++) {
	histo.Plot(id, i, 1, max(0., at(i) - offset));
    }
}

void Trace::OffsetPlot(int id, int row, double offset)
{
    for (size_type i=0; i < size(); i++) {
	histo.Plot(id, i, row, max(0., at(i) - offset));
    }
}
