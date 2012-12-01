/** \file TraceExtractor.cpp
 *  \brief Extract traces for a specific type and subtype
 */

#include <string>

#include "DammPlotIds.hpp"
#include "Trace.hpp"
#include "TraceExtractor.hpp"

using std::string;
using namespace dammIds::trace;

const int TraceExtractor::traceBins = SC;
const int TraceExtractor::numTraces = 99;

TraceExtractor::TraceExtractor(const std::string& aType, const std::string &aSubtype) : 
  TraceAnalyzer(extractor::OFFSET, extractor::RANGE), type(aType), subtype(aSubtype)
{
    name = "Extractor";
}

TraceExtractor::~TraceExtractor()
{
    // do nothing
}

/** Declare the damm plots */
void TraceExtractor::DeclarePlots(void)
{
    using namespace dammIds::trace;
    for (int i=0; i < numTraces; i++)
	DeclareHistogram1D(extractor::D_TRACE + i, traceBins, "traces data");
}

/** Plot the damm spectra of the first few traces analyzed with (level >= 1) */
void TraceExtractor::Analyze(Trace &trace,
			     const string &aType, const string &aSubtype)
{   
    using namespace dammIds::trace;

    if (type ==  aType && subtype == aSubtype && numTracesAnalyzed < numTraces) {	
	TraceAnalyzer::Analyze(trace, type, subtype);	
	trace.OffsetPlot(extractor::D_TRACE + numTracesAnalyzed, trace.DoBaseline(1,20) );
	EndAnalyze(trace);
    }
}
