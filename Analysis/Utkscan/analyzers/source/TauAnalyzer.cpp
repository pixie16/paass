/*! \file TauAnalyzer.cpp
 * \brief Implements the determination of the decay constants for a trace
 * @author D. Miller
 */
#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>

#include <cmath>

#include "Globals.hpp"
#include "TauAnalyzer.hpp"

using namespace std;

TauAnalyzer::TauAnalyzer() {
    name = "tau";
    type = subtype = "";
}

TauAnalyzer::TauAnalyzer(const std::string &aType, const std::string &aSubtype) :
        TraceAnalyzer(), type(aType), subtype(aSubtype) {
    name = "tau";
}

void TauAnalyzer::Analyze(Trace &trace, const ChannelConfiguration &cfg) {
    // don't do analysis for piled-up traces
    ///@TODO renable this, not a huge priority since Tau analyzer isn't used
    /// much.
//    if (trace.HasValue("filterEnergy2")) {
//        return;
//    }
    // only do analysis for the proper type and subtype
    if (type != cfg.GetType() && subtype != cfg.GetSubtype())
        return;

    TraceAnalyzer::Analyze(trace, cfg);

    Trace::const_iterator itMax = max_element(trace.begin(), trace.end());
    Trace::const_iterator itMin = min_element(itMax, (Trace::const_iterator) trace.end());
    iterator_traits<Trace::const_iterator>::difference_type size = distance(itMax, itMin);

    // skip over the area near the extrema since it may be non-exponential there
    advance(itMax, size / 10);
    advance(itMin, -size / 10);

    size = distance(itMax, itMin);
    double n = (double) size;

    double sum1 = 0, sum2 = 0;
    double i = 0;
    for (Trace::const_iterator it = itMax; it != itMin; it++) {
        double j = i + 1.;
        sum1 += double(*it) * (j * n * n - 3 * j * j * n + 2 * j * j * j);
        sum2 += double(*it) * (i * n * n - 3 * i * i * n + 2 * i * i * i);
        i += 1.;
    }
    double tau = 1 / log(sum1 / sum2) * Globals::get()->GetClockInSeconds();
    trace.SetTau(tau);

    EndAnalyze();
}