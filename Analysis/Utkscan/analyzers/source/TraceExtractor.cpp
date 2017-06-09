/** \file TraceExtractor.cpp
 *  \brief Extract traces for a specific type and subtype
 *  @authors D. Miller, S. V. Paulauskas
 */
#include <iostream>
#include <sstream>

#include "DammPlotIds.hpp"
#include "Trace.hpp"
#include "TraceExtractor.hpp"

using namespace std;
using namespace dammIds::analyzers::extractor;

namespace dammIds {
    namespace analyzers {
        namespace extractor {
            const unsigned int DD_TRACE = 0;
        }
    }
}

TraceExtractor::TraceExtractor(const std::string &type, const std::string &subtype, const std::string &tag) :
        TraceAnalyzer(OFFSET, RANGE, "Trace Extractor"), type_(type), subtype_(subtype), tag_(tag) {
}

void TraceExtractor::DeclarePlots(void) {
    const int traceBins = dammIds::analyzers::traceBins;
    DeclareHistogram2D(DD_TRACE, traceBins, S7, "Trace Extractor");
}

void TraceExtractor::Analyze(Trace &trace, const ChannelConfiguration &cfg) {
    static unsigned int numPlottedTraces = 0;
    static unsigned int numTraces = S8;

    if (type_ == cfg.GetType() && subtype_ == cfg.GetSubtype() && cfg.HasTag(tag_) && numPlottedTraces < numTraces) {
        TraceAnalyzer::Analyze(trace, cfg);
        OffsetPlot(trace, DD_TRACE, numPlottedTraces, 0.0);
        numPlottedTraces++;
        EndAnalyze(trace);
    }
}
