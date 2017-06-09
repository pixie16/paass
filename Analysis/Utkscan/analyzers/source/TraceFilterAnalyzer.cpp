/** \file  TraceFilterAnalyzer.cpp
 *  \brief Implements the analysis of traces using trapezoidal filters
 * \author D. Miller, S. V. Paulauskas
 * \date January 2011
 */
#include <sstream>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RandomInterface.hpp"
#include "TraceFilter.hpp"
#include "TraceFilterAnalyzer.hpp"

using namespace std;
using namespace dammIds::analyzers::tracefilter;

namespace dammIds {
    namespace analyzers {
        namespace tracefilter {
            const unsigned int D_RETVALS = 0; //!< Return values for TraceFilter class
            const unsigned int DD_TRIGGER_FILTER = 1;//!< Trigger Filter
            const unsigned int DD_REJECTED_TRACE = 2;//!< Traces with some kind of error
            const unsigned int DD_PILEUP = 3; //!< Piled up traces
        }
    }
}

TraceFilterAnalyzer::TraceFilterAnalyzer(const bool &analyzePileup) :
        TraceAnalyzer(OFFSET, RANGE, "TraceFilterAnalyzer") {
    analyzePileup_ = analyzePileup;
    name = "TraceFilterAnalyzer";
}

void TraceFilterAnalyzer::DeclarePlots(void) {
    const int traceBins = dammIds::analyzers::traceBins;
    DeclareHistogram1D(D_RETVALS, S3, "Retvals for Filtering");
    DeclareHistogram2D(DD_TRIGGER_FILTER, traceBins, S7, "Trigger Filter");
    DeclareHistogram2D(DD_REJECTED_TRACE, traceBins, S7, "Rejected Traces");
    DeclareHistogram2D(DD_PILEUP, traceBins, S7, "Rejected Traces");
}

void TraceFilterAnalyzer::Analyze(Trace &trace, const ChannelConfiguration &cfg) {
    TraceAnalyzer::Analyze(trace, cfg);
    Globals *globs = Globals::get();
    static int numTrigFilters = 0;
    static int numRejected = 0;
    static int numPileup = 0;
    static unsigned short numTraces = S7;

    //Want to put filter clock units of ns/Sample
    TraceFilter filter(globs->GetFilterClockInSeconds() * 1e9, cfg.GetTriggerFilterParameters(),
                       cfg.GetEnergyFilterParameters(), analyzePileup_);
    unsigned int retval = filter.CalcFilters(&trace);

    //if retval != 0 there was a problem and we should look at the trace
    if (retval != 0) {
        plot(D_RETVALS, retval);
        if (numRejected < numTraces)
            plot(DD_REJECTED_TRACE, numRejected++);
    }

    trace.SetTriggerFilter(filter.GetTriggerFilter());
    trace.SetTriggerPositions(filter.GetTriggers());
    trace.SetFilteredEnergies(filter.GetEnergies());
    trace.SetEnergySums(filter.GetEnergySums());
    trace.SetFilteredBaseline(filter.GetBaseline());

    //plot traces that were flagged as pileups
    if (filter.GetHasPileup() && numPileup < numTraces)
        plot(DD_PILEUP, numPileup++);

    //500 is an arbitrary offset since DAMM cannot display negative numbers.
    vector<double> tfilt = filter.GetTriggerFilter();
    for (vector<double>::iterator it = tfilt.begin(); it != tfilt.end(); it++)
        plot(DD_TRIGGER_FILTER, (int) (it - tfilt.begin()), numTrigFilters, (*it) + 500);
    numTrigFilters++;

    EndAnalyze(trace);
}
