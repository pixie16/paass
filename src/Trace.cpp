/** \file Trace.cpp
 * \brief Implement how to do our usual tricks with traces
 */

#include <algorithm>
#include <iostream>
#include <cmath>
#include <numeric>

#include "Trace.hpp"

#include "DammPlotIds.hpp"

using namespace std;
using namespace dammIds::trace;

namespace dammIds {
    namespace trace {
    }
} // trace namespace

const Trace emptyTrace; ///< an empty trace for const references to point to

/*
 * Plots are static, class-wide variable, so every trace instance has
 * an access to the same histogram range
 */
Plots Trace::histo(OFFSET, RANGE);

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

    double sum = accumulate(begin() + lo, begin() + hi, 0.0);
    double mean = sum / numBins;
    double sq_sum = inner_product(begin() + lo, begin() + hi,
                                  begin() + lo, 0.0);
    double std_dev = sqrt(sq_sum / numBins - mean * mean);

    SetValue("baseline", mean);
    SetValue("sigmaBaseline", std_dev);

    baselineLow  = lo;
    baselineHigh = hi;

    return mean;
}

double Trace::DoDiscrimination(unsigned int lo, unsigned int numBins)
{
    unsigned int high = lo+numBins;
    
    unsigned int max = GetValue("maxpos");
    double discrim = 0, baseline = GetValue("baseline");
    
    //reference the sum to the maximum of the trace
    high += max;
    lo += max;
    
    if(size() < high)
	return U_DELIMITER;
    
    for(unsigned int i = lo; i < high; i++)
	discrim += at(i)-baseline;
    
    InsertValue("discrim", discrim);
    
    return(discrim);
}

double Trace::DoQDC(unsigned int lo, unsigned int numBins) 
{
    unsigned int high = lo+numBins;
    
    if(size() < high)
	return(NAN);
    
    double baseline = GetValue("baseline");
    double qdc = 0, fullQdc = 0;
    
    for(unsigned int i = lo; i <= high; i++) {
	qdc += at(i)-baseline;
	waveform.push_back(at(i)-baseline);
    }
    
    for(unsigned int i = 0; i < size(); i++)
	fullQdc += at(i)-baseline;
    
    InsertValue("fullQdc", fullQdc);
    InsertValue("tqdc", qdc);
    return(qdc);
}

unsigned int Trace::FindMaxInfo(void)
{
    unsigned int hi = constants.GetConstant("traceDelay") /
	(pixie::adcClockInSeconds*1e9);
    unsigned int lo = hi - (constants.GetConstant("trapezoidalWalk") / 
			    (pixie::adcClockInSeconds*1e9) ) - 3;
    
    if(size() < hi)
        return U_DELIMITER;
    
    Trace::const_iterator itTrace = max_element(begin()+lo, end()-(size()-hi));
    
    int maxPos = int(itTrace-begin());
    
    if(maxPos + constants.GetConstant("waveformHigh") > size())
	return U_DELIMITER;
    
    if(*itTrace >= 4095) {
	InsertValue("saturation", 1);
	return(-1);
    }
    
    DoBaseline(0,maxPos-constants.GetConstant("waveformLow"));
    
    InsertValue("maxpos", maxPos);
    InsertValue("maxval", *itTrace-GetValue("baseline"));
    
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
