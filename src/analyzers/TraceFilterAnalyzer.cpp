/** \file  TraceFilterAnalyzer.cpp
 *  \brief Implements the analysis of traces using trapezoidal filters
 * \author S. V. Paulauskas, original D. Miller
 * \date January 2011
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RandomPool.hpp"
#include "TraceFilter.hpp"
#include "TraceFilterAnalyzer.hpp"

using namespace std;
using namespace dammIds::trace::tracefilterer;

TraceFilterAnalyzer::TraceFilterAnalyzer(const TrapFilterParameters &t,
                                         const TrapFilterParameters &e) :
    TraceAnalyzer() {
    trigPars_ = t;
    enPars_ = e;
    name = "TraceFilterAnalyzer";
}

void TraceFilterAnalyzer::DeclarePlots(void) {
    const int energyBins = SE;
    const int energyBins2 = SB;
    const int traceBins = dammIds::trace::traceBins;

    //! Declare plots within the trace object
    Trace sample_trace = Trace();
    unsigned short numTraces = Globals::get()->numTraces();

    sample_trace.DeclareHistogram2D(DD_TRACE, traceBins, numTraces,
                                    "traces data TracePlotter");
    sample_trace.DeclareHistogram2D(DD_FILTER1, traceBins, numTraces,
                                    "fast filter");
    sample_trace.DeclareHistogram2D(DD_FILTER2, traceBins, numTraces,
                                    "energy filter");
    sample_trace.DeclareHistogram2D(DD_REJECTED_TRACE, traceBins, numTraces,
                                    "rejected traces");

    sample_trace.DeclareHistogram1D(D_ENERGY1, energyBins, "E1 from trace");

    sample_trace.DeclareHistogram2D(DD_ENERGY__BOARD_FILTER,
                                 energyBins2, energyBins2,
                                "Board raw energy vs filter energy (/10)");
    sample_trace.DeclareHistogram1D(D_RATIO_BOARD_FILTER,
                energyBins2, "Ratio raw energy to filter (%)");
}

void TraceFilterAnalyzer::Analyze(Trace &trace, const std::string &type,
                                  const std::string &subtype,
                                  const std::map<std::string,int> &tagmap) {
    TraceAnalyzer::Analyze(trace, type, subtype,tagmap);

    TrapFilterParameters  trigPars(0.104,0.0,10);
    TrapFilterParameters  enPars(0.128,0.048,0.01);

    TraceFilter filter(Globals::get()->filterClockInSeconds(), trigPars,
                       enPars);

    filter.SetVerbose(true);
    filter.CalcFilters(&trace);
    vector<double> trig = filter.GetTriggerFilter();
    vector<double> esums = filter.GetEnergySums();
    double trcEn = filter.GetEnergy();

    //     // start at sample 5 because first samples are occasionally corrupted
    //     trace.DoBaseline(5, baselineBins);
    //     if ( trace.GetValue("sigmaBaseline") > deviationCut ||
    //         abs(trailingBaseline - trace.GetValue("baseline")) < deviationCut) {
    //         // perhaps check trailing baseline deviation
    //         // from a simple linear fit
    //         static int rejectedTraces = 0;
    //         unsigned short numTraces = Globals::get()->numTraces();
    //         if (rejectedTraces < numTraces)
    //             trace.Plot(DD_REJECTED_TRACE, rejectedTraces++);
    //         EndAnalyze(); // update timing
    //         return;
    //     }

    //     fastFilter.clear();
    //     energyFilter.clear();

    //     // determine trace filters, these are trapezoidal filters characterized
    //     //   by a risetime and a gaptime and a range of the filter
    //     trace.TrapezoidalFilter(fastFilter, fastParms);
    //     trace.TrapezoidalFilter(energyFilter, energyParms);

    //     if (useThirdFilter) {
    //         thirdFilter.clear();
    //         trace.TrapezoidalFilter(thirdFilter, thirdParms);
    //     }

    //     fastFilter.ScalePlot(DD_FILTER1, numTracesAnalyzed,
    //                 fastParms.GetRiseSamples() );
    //     energyFilter.ScalePlot(DD_FILTER2, numTracesAnalyzed,
    //                 energyParms.GetRiseSamples() );
    //     if (useThirdFilter) {
    //         thirdFilter.ScalePlot(DD_FILTER3, numTracesAnalyzed,
    //                 thirdParms.GetRiseSamples() );
    //     }
    //     trace.plot(D_ENERGY1, 0.0);
    // }
    EndAnalyze(trace);
}
