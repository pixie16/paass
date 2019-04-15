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

WaveformAnalyzer::WaveformAnalyzer(const std::set<std::string> &ignoredTypes)
    : TraceAnalyzer() {
    name = "WaveformAnalyzer";
    ignoredTypes_ = ignoredTypes;
}

void WaveformAnalyzer::Analyze(Trace &trace, const ChannelConfiguration &cfg) {
    TraceAnalyzer::Analyze(trace, cfg);
    if (trace.IsSaturated() || trace.empty() || ignoredTypes_.find(cfg.GetType()) != ignoredTypes_.end() || ignoredTypes_.find(cfg.GetType() + ":" + cfg.GetSubtype()) != ignoredTypes_.end() || ignoredTypes_.find(cfg.GetType() + ":" + cfg.GetSubtype() + ":" + cfg.GetGroup()) != ignoredTypes_.end()) {
        trace.SetHasValidAnalysis(false);
        EndAnalyze();
        return;
    }

    pair<unsigned int, unsigned int> range = cfg.GetWaveformBoundsInSamples();

    //First we calculate the position of the maximum.
    pair<unsigned int, double> max;
    try {
        max = TraceFunctions::FindMaximum(trace, cfg.GetTraceDelayInSamples());
    } catch (range_error &ex) {
        trace.SetHasValidAnalysis(false);
        cout << "WaveformAnalyzer::Analyze - " << ex.what() << endl;
        EndAnalyze();
        return;
    }

    //If the position of the maximum doesn't give us enough bins on the
    // baseline to calculate the average baseline then we're going to set
    // some of the variables to be used later to zero and end the analysis of
    // the waveform now.
    if ((int)(max.first - range.first) < TraceFunctions::minimum_baseline_length) {
#ifdef VERBOSE
        cout << "WaveformAnalyzer::Analyze - The low bound for the trace overlaps with the minimum bins for the"
        "baseline." << endl;
#endif
        trace.SetHasValidAnalysis(false);
        EndAnalyze();
        return;
    }

    try {
        //Next we calculate the baseline and its standard deviation
        pair<double, double> baseline = TraceFunctions::CalculateBaseline(trace, make_pair(0, max.first - range.first));

        //For well behaved traces the standard deviation of the baseline
        // shouldn't ever be more than 1-3 ADC units. However, for traces
        // that are not captured properly, we can get really crazy values
        // here the SiPM often saw values as high as 20. We will put in a
        // hard limit of 50 as a cutoff since anything with a standard
        // deviation of this high will never be something we want to analyze.
        static const double extremeBaselineVariation = 50;
        if (baseline.second >= extremeBaselineVariation) {
            trace.SetHasValidAnalysis(false);
            EndAnalyze();
            return;
        }

        //Subtract the baseline from the maximum value.
        max.second -= baseline.first;

        vector<double> traceNoBaseline;
        for (unsigned int i = 0; i < trace.size(); i++)
            traceNoBaseline.push_back(trace[i] - baseline.first);

        //Finally, we calculate the QDC in the waveform range and subtract
        // the baseline from it.
        pair<unsigned int, unsigned int> waveformRange(max.first - range.first, max.first + range.second);
        double qdc = TraceFunctions::CalculateQdc(traceNoBaseline, waveformRange);

        //Now we are going to set all the different values into the trace.
        trace.SetQdc(qdc);
        trace.SetBaseline(baseline);
        trace.SetMax(max);
        trace.SetExtrapolatedMax(make_pair(max.first,
                                           TraceFunctions::ExtrapolateMaximum(trace, max).first - baseline.first));
        trace.SetTraceSansBaseline(traceNoBaseline);
        trace.SetWaveformRange(waveformRange);
        trace.SetHasValidAnalysis(true);
    } catch (range_error &ex) {
        trace.SetHasValidAnalysis(false);
        cout << "WaveformAnalyzer::Analyze - " << ex.what() << endl;
        EndAnalyze();
        return;
    }
}