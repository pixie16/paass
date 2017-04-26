/** \file WaveformAnalyzer.cpp
 *\brief Preliminary waveform analysis
 *
 *Does preliminary waveform analysis on traces. The parameters set here
 *will be used for the high resolution timing algorithms to do their thing.
 *
 *\author S. V. Paulauskas
 *\date July 16, 2009
*/
#include <numeric>
#include <string>
#include <utility>

#include <cmath>

#include "HelperFunctions.hpp"
#include "WaveformAnalyzer.hpp"

using namespace std;

WaveformAnalyzer::WaveformAnalyzer() : TraceAnalyzer() {
    name = "WaveformAnalyzer";
    messenger_ = new Messenger();
}

void WaveformAnalyzer::Analyze(Trace &trace, const std::string &type,
                               const std::string &subtype,
                               const std::map<std::string, int> &tags) {
    TraceAnalyzer::Analyze(trace, type, subtype, tags);

    if (trace.IsSaturated() || trace.empty()) {
        EndAnalyze();
        return;
    }

    Globals *globals = Globals::get();

    pair<unsigned int, unsigned int> range =
            globals->GetWaveformRange(type + ":" + subtype);

    if (type == "beta" && subtype == "double" &&
        tags.find("timing") != tags.end())
        range = globals->GetWaveformRange(type + ":" + subtype + ":timing");

    try {
        //First we calculate the position of the maximum.
        pair<unsigned int, double> max =
                TraceFunctions::FindMaximum(trace, globals->GetTraceDelayInNs() /
                                                   (globals->GetAdcClockInSeconds() *
                                                    1.e9));

        //Next we calculate the baseline and its standard deviation
        pair<double, double> baseline =
                TraceFunctions::CalculateBaseline(trace,
                                                  make_pair(0, max.first -
                                                               range.first));

        //Subtract the baseline from the maximum value.
        max.second -= baseline.first;

        vector<double> traceNoBaseline;
        for (unsigned int i = 0; i < trace.size(); i++)
            traceNoBaseline.push_back(trace[i] - baseline.first);

        //Finally, we calculate the QDC in the waveform range and subtract
        // the baseline from it.
        pair<unsigned int, unsigned int> waveformRange(max.first - range.first,
                                                       max.first + range.second);
        double qdc =
                TraceFunctions::CalculateQdc(traceNoBaseline, waveformRange);

        //Now we are going to set all the different values into the trace.
        trace.SetQdc(qdc);
        trace.SetBaseline(baseline);
        trace.SetMax(max);
        trace.SetExtrapolatedMax(
                make_pair(max.first,
                          TraceFunctions::ExtrapolateMaximum(trace, max).first -
                                  baseline.first));
        trace.SetTraceSansBaseline(traceNoBaseline);
        trace.SetWaveformRange(waveformRange);
    } catch (range_error &ex) {
         string det = type +":" + subtype;
        cerr << "WaveformAnalyzer::Analyze - " << ex.what() << " for a " << det
             <<endl;

        EndAnalyze();
    }
}