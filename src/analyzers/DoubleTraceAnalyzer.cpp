/** \file DoubleTraceAnalyzer.cpp
 * \brief Identifies double traces.
 *
 * Implements a quick online trapezoidal filtering mechanism
 * for the identification of double pulses
 *
 * \author S. N. Liddick
 * \date 02 July 2007
 *
 * <STRONG>Modified : </STRONG> SNL - 2-4-08 - Add plotting spectra
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <numeric>

#include <cstdlib>

#include "DammPlotIds.hpp"
#include "RandomPool.hpp"
#include "Trace.hpp"
#include "DoubleTraceAnalyzer.hpp"
#include "Messenger.hpp"
#include "Globals.hpp"

using namespace std;

int DoubleTraceAnalyzer::numDoubleTraces = 0;
/**
 * Set default values for time and energy
 */
DoubleTraceAnalyzer::DoubleTraceAnalyzer(double energyScaleFactor,
                                         short fast_rise, short fast_gap,
                                         short fast_threshold,
                                         short energy_rise, short energy_gap,
                                         short slow_rise, short slow_gap,
                                         short slow_threshold) :
    TraceFilterer(energyScaleFactor,
                  fast_rise, fast_gap, fast_threshold,
                  energy_rise, energy_gap,
                  slow_rise, slow_gap, slow_threshold) {
}

void DoubleTraceAnalyzer::DeclarePlots() {
    using namespace dammIds::trace::doubletraceanalyzer;

    TraceFilterer::DeclarePlots();

    const int energyBins = SE;
    const int energyBins2 = SA;
    const int timeBins = SA;
    const int traceBins = dammIds::trace::traceBins;

    Trace sample_trace = Trace();
    unsigned short numTraces = Globals::get()->numTraces();

    sample_trace.DeclareHistogram1D(D_ENERGY2, energyBins, "E2 from traces");

    sample_trace.DeclareHistogram2D(DD_DOUBLE_TRACE, traceBins, numTraces,
                                    "Double traces");
    sample_trace.DeclareHistogram2D(DD_ENERGY2__TDIFF, energyBins2, timeBins,
                                    "E2 vs DT");
    sample_trace.DeclareHistogram2D(DD_ENERGY2__ENERGY1, energyBins2,
                                    energyBins2, "E2 vs E1");

    sample_trace.DeclareHistogram2D(DD_TRIPLE_TRACE, traceBins,
                                    numTraces, "Interesting triple traces");
    sample_trace.DeclareHistogram2D(DD_TRIPLE_TRACE_FILTER1, traceBins,
                                numTraces, "Interesting traces (fast filter)");
    sample_trace.DeclareHistogram2D(DD_TRIPLE_TRACE_FILTER2, traceBins,
                            numTraces, "Interesting traces (energy filter)");
    sample_trace.DeclareHistogram2D(DD_TRIPLE_TRACE_FILTER3, traceBins,
                                numTraces, "Interesting traces (3rd filter)");
}

/**
 *   Detect a second crossing of the fast filter corresponding to a piled-up
 *     trace and deduce its energy
 */
void DoubleTraceAnalyzer::Analyze(Trace &trace, const std::string &type,
                                  const std::string &subtype) {
    if (subtype == "top" || subtype == "bottom")
        return;

    Messenger m;

    TraceFilterer::Analyze(trace, type, subtype);
    // class to see when the fast filter falls below threshold
    static binder2nd< less<Trace::value_type> > recrossesThreshold
	(less<Trace::value_type>(), fastThreshold);

    if ( pulse.isFound && level >= 10 ) {
        Trace::iterator iThr = fastFilter.begin() + pulse.time;
        Trace::iterator iHigh = fastFilter.end();

        vector<PulseInfo> pulseVec;
        pulseVec.push_back(pulse);
        const size_t pulseLimit = 50; // maximum number of pulses to find

        while (iThr < iHigh) {
            advance(iThr, fastParms.GetGapSamples());
            iThr = find_if(iThr, iHigh, recrossesThreshold);
            advance(iThr, fastParms.GetRiseSamples());

            FindPulse(iThr, iHigh);
            if (pulse.isFound) {
                pulseVec.push_back(pulse);
                iThr = fastFilter.begin() + pulse.time;
            } else break;
            if (pulseVec.size() > pulseLimit) {

                stringstream ss;
                ss << "Too many pulses, limit = "
                   << pulseLimit << ", breaking out.";
                m.warning(ss.str());

                EndAnalyze();
                return;
            }
        }

        trace.SetValue("numPulses", (int)pulseVec.size());

        if ( pulseVec.size() > 1 ) {
            using namespace dammIds::trace::doubletraceanalyzer;
            for (Trace::size_type i=1; i < pulseVec.size(); i++) {
                stringstream str;
                str << "filterEnergy" << i+1;
                trace.SetValue(str.str(), pulseVec[i].energy);
                str.str("");
                str << "filterTime" << i+1;
                trace.SetValue(str.str(), (int)pulseVec[i].time);
            }

            trace.Plot(DD_DOUBLE_TRACE, numDoubleTraces);
            if (pulseVec.size() > 2) {
                static int numTripleTraces = 0;

                stringstream ss;
                ss << "Found triple trace " << numTripleTraces
                   << ", num pulses = " << pulseVec.size()
                   << ", sigma baseline = " << trace.GetValue("sigmaBaseline");
                m.run_message(ss.str());

                trace.Plot(DD_TRIPLE_TRACE, numTripleTraces);
                fastFilter.ScalePlot(DD_TRIPLE_TRACE_FILTER1, numTripleTraces,
                                    fastParms.GetRiseSamples());
                energyFilter.ScalePlot(DD_TRIPLE_TRACE_FILTER2, numTripleTraces,
                                    energyParms.GetRiseSamples());
                if (useThirdFilter)
                    thirdFilter.ScalePlot(DD_TRIPLE_TRACE_FILTER3,
                                numTripleTraces, thirdParms.GetRiseSamples());
                    numTripleTraces++;
            }

            trace.plot(D_ENERGY2, pulseVec[1].energy);
            trace.plot(DD_ENERGY2__TDIFF,
                pulseVec[1].energy, pulseVec[1].time - pulseVec[0].time);
            trace.plot(DD_ENERGY2__ENERGY1,
                pulseVec[1].energy, pulseVec[0].energy);

            numDoubleTraces++;
        } // if found double trace
    } // sufficient analysis level

    EndAnalyze(trace);
}
