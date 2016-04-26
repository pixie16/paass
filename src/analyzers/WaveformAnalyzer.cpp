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
    knownTypes_.push_back("labr3");
}

void WaveformAnalyzer::Analyze(Trace &trace,
                               const std::string &detType,
                               const std::string &detSubtype,
                               const std::map<std::string, int> & tagMap) {
    TraceAnalyzer::Analyze(trace, detType, detSubtype,tagMap);

    if(CheckIfUnknown(detType) || trace.HasValue("saturation")){
        EndAnalyze();
        return;
    }

    Globals *globals = Globals::get();

    pair<unsigned int, unsigned int> range = globals->waveformRange(detType+":"+detSubtype);

    if( detType == "beta" && detSubtype == "double" && tagMap.find("timing") != tagMap.end())
	range = globals->waveformRange(detType+":"+detSubtype+":timing");
    
    double qdc = trace.DoQDC(trace.FindMaxInfo(range.first, range.second)-range.first,
			     range.second+range.first);

    trace.InsertValue("qdcToMax", qdc/trace.GetValue("maxval"));

    if(detSubtype == "liquid")
        trace.DoDiscrimination(globals->discriminationStart(),
                range.second - globals->discriminationStart());

    EndAnalyze();
}

bool WaveformAnalyzer::CheckIfUnknown(const std::string &type) {
    if(find(knownTypes_.begin(), knownTypes_.end(), type) == knownTypes_.end())
        return true;
    return false;
}
