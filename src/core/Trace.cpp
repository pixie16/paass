/** \file Trace.cpp
 * \brief Implement how to do our usual tricks with traces
 */
#include <algorithm>
#include <iostream>
#include <cmath>
#include <numeric>

#include "Trace.hpp"

using namespace std;

namespace dammIds {
    namespace trace {
    }
}

const Trace emptyTrace; ///< an empty trace for const references to point to
Plots Trace::histo(dammIds::trace::OFFSET, dammIds::trace::RANGE, "traces");

double Trace::DoBaseline(unsigned int lo, unsigned int numBins) {
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

    return(mean);
}

double Trace::DoDiscrimination(unsigned int lo, unsigned int numBins) {
    unsigned int high = lo+numBins;

    if(size() < high)
        return pixie::U_DELIMITER;

    int discrim = 0, max = GetValue("maxpos");
    double baseline = GetValue("baseline");

    if(size() < high)
	return pixie::U_DELIMITER;

    for(unsigned int i = max+lo; i <= max+high; i++)
	discrim += at(i)-baseline;

    InsertValue("discrim", discrim);

    return(discrim);
}

double Trace::DoQDC(unsigned int lo, unsigned int numBins) {
    unsigned int high = lo+numBins;

    if(size() < high)
	return pixie::U_DELIMITER;

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

unsigned int Trace::FindMaxInfo(unsigned int lo, unsigned int hi,
                                unsigned int numBins) {
    unsigned int high = Globals::get()->traceDelay() /
        (Globals::get()->adcClockInSeconds()*1e9);
    unsigned int low = high - (Globals::get()->trapezoidalWalk() /
			    (Globals::get()->adcClockInSeconds()*1e9)) - 3;

    if(size() < high)
        return pixie::U_DELIMITER;

    Trace::const_iterator itTrace = max_element(begin()+low, end()-(size()-high));
    int maxPos = int(itTrace-begin());

    if(maxPos + hi > size())
	return pixie::U_DELIMITER;

    if(*itTrace >= 4095) {
        InsertValue("saturation", 1);
        return(-1);
    }

    DoBaseline(0, maxPos-lo);
    InsertValue("maxpos", maxPos);
    InsertValue("maxval", *itTrace-GetValue("baseline"));

    return (itTrace-begin());
}

void Trace::Plot(int id) {
    for (size_type i=0; i < size(); i++) {
        histo.Plot(id, i, 1, at(i));
    }
}

void Trace::Plot(int id, int row) {
    for (size_type i=0; i < size(); i++) {
        histo.Plot(id, i, row, at(i));
    }
}

void Trace::ScalePlot(int id, double scale) {
    for (size_type i=0; i < size(); i++) {
        histo.Plot(id, i, 1, abs(at(i)) / scale);
    }
}

void Trace::ScalePlot(int id, int row, double scale) {
    for (size_type i=0; i < size(); i++) {
        histo.Plot(id, i, row, abs(at(i)) / scale);
    }
}

void Trace::OffsetPlot(int id, double offset) {
    for (size_type i=0; i < size(); i++) {
        histo.Plot(id, i, 1, max(0., at(i) - offset));
    }
}

void Trace::OffsetPlot(int id, int row, double offset) {
    for (size_type i=0; i < size(); i++) {
        histo.Plot(id, i, row, max(0., at(i) - offset));
    }
}
