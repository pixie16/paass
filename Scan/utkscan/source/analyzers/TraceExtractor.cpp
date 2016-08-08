/** \file TraceExtractor.cpp
 *  \brief Extract traces for a specific type and subtype
 */
#include <iostream>
#include <sstream>

#include "Trace.hpp"
#include "TraceExtractor.hpp"
#include "DammPlotIds.hpp"

using std::string;

using namespace std;

const unsigned int TraceExtractor::numTraces = dammIds::trace::extractor::maxSingleTraces;

TraceExtractor::TraceExtractor(const std::string& aType,
                               const std::string &aSubtype,
			       const std::string &aTag) :
    type(aType), subtype(aSubtype), tag(aTag){
    name = "TraceExtractor";
}

void TraceExtractor::DeclarePlots(void)
{
    const int traceBins = dammIds::trace::traceBins;
    using namespace dammIds::trace::extractor;
    Trace sample_trace = Trace();
    sample_trace.DeclareHistogram2D(D_TRACE, traceBins, S7, "Trace Extractor");
}

void TraceExtractor::Analyze(Trace &trace, const std::string &aType,
                             const std::string &aSubtype, 
			     const std::map<std::string,int> &tags) {
    using namespace dammIds::trace::extractor;
    static unsigned int numPlottedTraces = 0;

    if (type ==  aType && subtype == aSubtype && 
	(tag == "" || tags.find(tag) != tags.end()) &&
	numPlottedTraces < numTraces){
        TraceAnalyzer::Analyze(trace, type, subtype, tags);
        trace.OffsetPlot(D_TRACE, numPlottedTraces, 0.0);
        numPlottedTraces++;
        EndAnalyze(trace);
    }
}
