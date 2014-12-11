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
}

void WaveformAnalyzer::Analyze(Trace &trace,
			       const std::string &detType,
			       const std::string &detSubtype) {
    TraceAnalyzer::Analyze(trace, detType, detSubtype);

    if(detType == "vandleSmall" || detType == "vandleBig"
       || detType == "liquid_scint" || detType == "pulser"
       || detType == "tvandle" || detType == "beta_scint") {

        if(trace.HasValue("saturation")) {
            EndAnalyze();
            return;
        }

        unsigned int waveformLow = Globals::get()->waveformLow();
        unsigned int waveformHigh = Globals::get()->waveformHigh();
        unsigned int startDiscrimination = Globals::get()->discriminationStart();
        unsigned int maxPos = trace.FindMaxInfo();

        double qdc = trace.DoQDC(maxPos-waveformLow,
                                 waveformHigh+waveformLow);

        trace.InsertValue("qdcToMax", qdc/trace.GetValue("maxval"));

        if(detSubtype == "liquid")
            trace.DoDiscrimination(startDiscrimination,
                    waveformHigh - startDiscrimination);
    } //if(detType
    EndAnalyze();
}
