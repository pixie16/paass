/** \file  TraceFilterer.cpp
 *  \brief Implements the analysis of traces using trapezoidal filters
 *
 *  This trace plots the filtered traces as well as energy and timing info
 *  I (SVP) Will be overhauling this class with a new implementation of the
 *  filters at some point.
 * \author David Miller
 * \date January 2011
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>

#include "DammPlotIds.hpp"
#include "RandomPool.hpp"
#include "Trace.hpp"
#include "TraceFilterer.hpp"
#include "Globals.hpp"

using namespace std;
using namespace dammIds::trace;

TraceFilterer::TraceFilterer(double energyScaleFactor,
                             short fast_rise, short fast_gap,
                             short fast_threshold,
                             short energy_rise, short energy_gap,
                             short slow_rise, short slow_gap,
                             short slow_threshold) :
    fastParms(fast_gap, fast_rise),
    energyParms(energy_gap, energy_rise),
    thirdParms(slow_gap, slow_rise) {
    //? this uses some legacy values for third parms, are they appropriate
    name = "Filterer";
    useThirdFilter = false;
    energyScaleFactor_ = energyScaleFactor;

    // scale thresholds by the length of integration (i.e. rise time)
    fastThreshold = fast_threshold * fastParms.GetRiseSamples();
    slowThreshold = slow_threshold * thirdParms.GetRiseSamples();
}

bool TraceFilterer::Init(const std::string &filterFileName) {
    const int maxTraceLength = 6400;

    fastFilter.reserve(maxTraceLength);
    energyFilter.reserve(maxTraceLength);
    thirdFilter.reserve(maxTraceLength);
    return true;
}

void TraceFilterer::DeclarePlots(void) {
    const int energyBins = SE;
    const int energyBins2 = SB;
    const int traceBins = dammIds::trace::traceBins;

    using namespace dammIds::trace::tracefilterer;

    //! Declare plots within the trace object
    Trace sample_trace = Trace();
    unsigned short numTraces = Globals::get()->numTraces();

    sample_trace.DeclareHistogram2D(DD_TRACE, traceBins, numTraces,
                                    "traces data TracePlotter");
    sample_trace.DeclareHistogram2D(DD_FILTER1, traceBins, numTraces,
                                    "fast filter");
    sample_trace.DeclareHistogram2D(DD_FILTER2, traceBins, numTraces,
                                    "energy filter");
    if (useThirdFilter) {
        sample_trace.DeclareHistogram2D(DD_FILTER3, traceBins, numTraces,
                                        "3rd filter");
    }
    sample_trace.DeclareHistogram2D(DD_REJECTED_TRACE, traceBins, numTraces,
                                    "rejected traces");

    sample_trace.DeclareHistogram1D(D_ENERGY1, energyBins, "E1 from trace");

    sample_trace.DeclareHistogram2D(DD_ENERGY__BOARD_FILTER,
                                 energyBins2, energyBins2,
                                "Board raw energy vs filter energy (/10)");
    sample_trace.DeclareHistogram1D(D_RATIO_BOARD_FILTER,
                energyBins2, "Ratio raw energy to filter (%)");
}

void TraceFilterer::Analyze(Trace &trace, const std::string &type,
                            const std::string &subtype) {
    using namespace dammIds::trace::tracefilterer;

    if (level >= 5) {
        const size_t baselineBins = 30;
        const double deviationCut = fastThreshold / 4. /
                                    fastParms.GetRiseSamples();

        double trailingBaseline  = trace.DoBaseline(
                                trace.size() - baselineBins - 1, baselineBins);

        // start at sample 5 because first samples are occasionally corrupted
        trace.DoBaseline(5, baselineBins);
        if ( trace.GetValue("sigmaBaseline") > deviationCut ||
            abs(trailingBaseline - trace.GetValue("baseline")) < deviationCut) {
            // perhaps check trailing baseline deviation
            // from a simple linear fit
            static int rejectedTraces = 0;
            unsigned short numTraces = Globals::get()->numTraces();
            if (rejectedTraces < numTraces)
                trace.Plot(DD_REJECTED_TRACE, rejectedTraces++);
            EndAnalyze(); // update timing
            return;
        }

        fastFilter.clear();
        energyFilter.clear();

        // determine trace filters, these are trapezoidal filters characterized
        //   by a risetime and a gaptime and a range of the filter
        trace.TrapezoidalFilter(fastFilter, fastParms);
        trace.TrapezoidalFilter(energyFilter, energyParms);

        if (useThirdFilter) {
            thirdFilter.clear();
            trace.TrapezoidalFilter(thirdFilter, thirdParms);
        }
        FindPulse(fastFilter.begin(), fastFilter.end());

        if (pulse.isFound) {
            trace.SetValue("filterTime", (int)pulse.time);
            trace.SetValue("filterEnergy", pulse.energy);
        }

        fastFilter.ScalePlot(DD_FILTER1, numTracesAnalyzed,
                    fastParms.GetRiseSamples() );
        energyFilter.ScalePlot(DD_FILTER2, numTracesAnalyzed,
                    energyParms.GetRiseSamples() );
        if (useThirdFilter) {
            thirdFilter.ScalePlot(DD_FILTER3, numTracesAnalyzed,
                    thirdParms.GetRiseSamples() );
        }
        trace.plot(D_ENERGY1, pulse.energy);
    }
    EndAnalyze(trace);
}

const TraceFilterer::PulseInfo& TraceFilterer::FindPulse(Trace::iterator begin,
                                                         Trace::iterator end) {
    static binder2nd< greater<Trace::value_type> > crossesThreshold
	(greater<Trace::value_type>(), fastThreshold);

    Trace::size_type sample;
    Trace::size_type presample;
    pulse.isFound = false;

    while (begin < end) {
        begin = find_if(begin, end, crossesThreshold);
        if (begin == end) {
            break;
        }

        pulse.time = begin - fastFilter.begin();
        pulse.isFound = true;
        presample = pulse.time - fastParms.GetRiseSamples();

        // sample the slow filter in the middle of its size
        if (useThirdFilter) {
            sample = pulse.time + (thirdParms.GetSize() - fastParms.GetSize()) / 2;
            if (sample >= thirdFilter.size() ||
            thirdFilter[sample] < slowThreshold) {
                begin++;
                continue;
            }
        }
        //? some investigation needed here for good resolution
        // add a presample location
        // sample = pulse.time + (energyParms.GetSize() - fastParms.GetSize()) / 2;
        sample = pulse.time + 40;

        RandomPool* randoms = RandomPool::get();
        if (sample < energyFilter.size()) {
            pulse.energy = energyFilter[sample] + randoms->Get();
            // subtract an energy filter baseline
            if (presample >= 0) {
                pulse.energy -= energyFilter[presample];
            }
            // scale to the integration time
            pulse.energy /= energyParms.GetRiseSamples();
            pulse.energy *= energyScaleFactor_;
        } else
            pulse.energy = NAN;

        break;
    }

    return pulse;
}
