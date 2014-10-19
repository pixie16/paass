/** \file TraceExtractor.cpp
 *  \brief Extract traces for a specific type and subtype
 */
#include <sstream>

#include "Trace.hpp"
#include "TraceExtractor.hpp"
#include "DammPlotIds.hpp"

using std::string;

const int TraceExtractor::numTraces = dammIds::trace::extractor::maxSingleTraces;

TraceExtractor::TraceExtractor(const std::string& aType, 
                               const std::string &aSubtype) : 
    type(aType), subtype(aSubtype) {
    name = "Extractor";
}

/** Declare the damm plots */
void TraceExtractor::DeclarePlots(void)
{
    const int traceBins = dammIds::trace::traceBins;
    using namespace dammIds::trace::extractor;
    Trace sample_trace = Trace();
    for (int i = 0; i < numTraces; ++i) {
        std::stringstream ss;
        ss << "TraceExtractor trace " << i;
        sample_trace.DeclareHistogram1D(D_TRACE + i, traceBins, 
                                        ss.str().c_str());
    }
}

/** Plot the damm spectra of the first few traces analyzed with (level >= 1) */
void TraceExtractor::Analyze(Trace &trace,
			     const string &aType, const string &aSubtype) {   
    using namespace dammIds::trace::extractor;

    if (type ==  aType && 
        subtype == aSubtype && 
        numTracesAnalyzed < numTraces) {	
        TraceAnalyzer::Analyze(trace, type, subtype);	
        trace.OffsetPlot(D_TRACE + numTracesAnalyzed, trace.DoBaseline(1,20) );
        EndAnalyze(trace);
    }
}
