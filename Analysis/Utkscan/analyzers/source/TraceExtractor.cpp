/** \file TraceExtractor.cpp
 *  \brief Extract traces for a specific type and subtype
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

TraceExtractor::TraceExtractor(const std::string& aType,
                               const std::string &aSubtype,
                               const std::string &aTag) :
        TraceAnalyzer(OFFSET, RANGE, "Trace Extractor"), type(aType),
        subtype(aSubtype), tag(aTag) {
}

void TraceExtractor::DeclarePlots(void)
{
    const int traceBins = dammIds::analyzers::traceBins;
    DeclareHistogram2D(DD_TRACE, traceBins, S7, "Trace Extractor");
}

void TraceExtractor::Analyze(
        Trace &trace, const std::string &aType, const std::string &aSubtype,
        const std::map<std::string,int> &tags) {
    static unsigned int numPlottedTraces = 0;
    static unsigned int numTraces = S8;

    if (type ==  aType && subtype == aSubtype && 
	(tag == "" || tags.find(tag) != tags.end()) &&
	numPlottedTraces < numTraces){
        TraceAnalyzer::Analyze(trace, type, subtype, tags);
        OffsetPlot(trace, DD_TRACE, numPlottedTraces, 0.0);
        numPlottedTraces++;
        EndAnalyze(trace);
    }
}
