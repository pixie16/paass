/** \file WaaAnalyzer.cpp
 * \brief Obtains the phase of a waveform using a weighted average algorithm
 * \author S. V. Paulauskas
 * \date August 13, 2013
 */
#include <algorithm>
#include <iostream>
#include <vector>

#include "Globals.hpp"
#include "DammPlotIds.hpp"
#include "WaaAnalyzer.hpp"

using namespace std;
using namespace dammIds::trace::waveformanalyzer;

void WaaAnalyzer::DeclarePlots(void) {
    Trace sample_trace = Trace();
    sample_trace.DeclareHistogram2D(DD_TRACES, S7, S5, "traces data Waa");
}

WaaAnalyzer::WaaAnalyzer() {
    name = "WaaAnalyzer";
}

void WaaAnalyzer::Analyze(Trace &trace, const string &detType,
                          const string &detSubtype,
                          const std::map<std::string, int> & tagMap) {
    TraceAnalyzer::Analyze(trace, detType, detSubtype,tagMap);

    if(trace.HasValue("saturation") || trace.empty()) {
     	EndAnalyze();
     	return;
    }

    const unsigned int maxPos = (unsigned int)trace.GetValue("maxpos");
    const double baseline = trace.GetValue("baseline");

    double sum = 0, phi = 0;
    static int row=0;
    for(unsigned int i = 0; i < trace.size(); i++) {
     	sum += trace[i]-baseline;
        trace.plot(DD_TRACES, i, row, trace[i]);
    }
    row++;

    unsigned int low = 5, high = 5;
    sum = 0; phi = 0;
    for(unsigned int i = maxPos - low; i <= maxPos + high; i++)
	sum += trace[i]-baseline;
    for(unsigned int i = maxPos - low; i <= maxPos + high; i++)
     	phi += ((trace[i]-baseline)/sum)*i;
    trace.InsertValue("phase", phi);
    //cout << phi << " " << maxPos << " " << endl;
    EndAnalyze();
} //void WaaAnalyzer::Analyze
