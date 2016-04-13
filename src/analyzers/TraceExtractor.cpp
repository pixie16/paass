/** \file TraceExtractor.cpp
 *  \brief Extract traces for a specific type and subtype
 */
#include <sstream>

#include "Trace.hpp"
#include "TraceExtractor.hpp"
#include "DammPlotIds.hpp"

using std::string;

const unsigned int TraceExtractor::numTraces = dammIds::trace::extractor::maxSingleTraces;

TraceExtractor::TraceExtractor(const std::string& aType,
                               const std::string &aSubtype) :
    type(aType), subtype(aSubtype) {
    name = "Extractor";
}

void TraceExtractor::DeclarePlots(void)
{
    const int traceBins = dammIds::trace::traceBins;
    using namespace dammIds::trace::extractor;
    Trace sample_trace = Trace();
    sample_trace.DeclareHistogram2D(D_TRACE, traceBins, S7, "Trace Extractor");
}

void TraceExtractor::Analyze(Trace &trace, const std::string &aType,
                             const std::string &aSubtype) {
    using namespace dammIds::trace::extractor;
    static unsigned int numPlottedTraces = 0;

    if (type ==  aType && subtype == aSubtype && numPlottedTraces < numTraces){
        TraceAnalyzer::Analyze(trace, type, subtype);
        trace.OffsetPlot(D_TRACE, numPlottedTraces, 0.0);
        numPlottedTraces++;
        EndAnalyze(trace);
    }
}
