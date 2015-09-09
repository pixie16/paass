/** \file WaveformAnalyzer.cpp
 *\brief Preliminary waveform analysis
 *
 *Does preliminary waveform analysis on traces. The parameters set here
 *will be used for the high resolution timing algorithms to do their thing.
 *
 *\author S. V. Paulauskas
 *\date 16 July 2009
*/
#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>

#include <cmath>

#include "WaveformAnalyzer.hpp"

using namespace std;

WaveformAnalyzer::WaveformAnalyzer() : TraceAnalyzer() {
    name = "Waveform";

    knownTypes_.push_back("vandle");
    knownTypes_.push_back("beta");
    knownTypes_.push_back("beta_scint");
    knownTypes_.push_back("liquid");
    knownTypes_.push_back("tvandle");
    knownTypes_.push_back("pulser");
}

void WaveformAnalyzer::Analyze(Trace &trace,
                               const std::string &detType,
                               const std::string &detSubtype) {
    TraceAnalyzer::Analyze(trace, detType, detSubtype);

    if(CheckIfUnknown(detType) || trace.HasValue("saturation")){
        EndAnalyze();
        return;
    }

    Globals *globals = Globals::get();

    pair<unsigned int, unsigned int> range = globals->waveformRange();
    if(detType == "beta" && detSubtype == "double")
        range = globals->siPmtWaveformRange();
    unsigned int startDiscrimination = globals->discriminationStart();
    unsigned int maxPos = trace.FindMaxInfo(range.first, range.second);

    double qdc = trace.DoQDC(maxPos-range.first,
                            range.second+range.first);

    trace.InsertValue("qdcToMax", qdc/trace.GetValue("maxval"));

    if(detSubtype == "liquid")
        trace.DoDiscrimination(startDiscrimination,
                range.second - startDiscrimination);

    EndAnalyze();
}

bool WaveformAnalyzer::CheckIfUnknown(const std::string &type) {
    if(find(knownTypes_.begin(), knownTypes_.end(), type) == knownTypes_.end())
        return true;
    return false;
}
