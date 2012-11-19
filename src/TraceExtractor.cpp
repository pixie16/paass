/** \file TraceExtractor.cpp
 *  \brief Extract traces for a specific type and subtype
 */
#include <string>

#include "Trace.hpp"
#include "TraceExtractor.hpp"

#include "DammPlotIds.hpp"

using std::string;
using namespace dammIds::trace::extractor;

const int TraceExtractor::traceBins = SC;
const int TraceExtractor::numTraces = 99;


TraceExtractor::TraceExtractor(const string& aType, const string &aSubtype) : 
  TraceAnalyzer(OFFSET, RANGE), type(aType), subtype(aSubtype)
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
    for (int i=0; i < numTraces; i++)
	DeclareHistogram1D(D_TRACE + i, traceBins, "traces data");
}

/** Plot the damm spectra of the first few traces analyzed with (level >= 1) */
void TraceExtractor::Analyze(Trace &trace,
			     const string &aType, const string &aSubtype)
{   

    if (type ==  aType && subtype == aSubtype && numTracesAnalyzed < numTraces) {	
	TraceAnalyzer::Analyze(trace, type, subtype);	
	trace.OffsetPlot(D_TRACE + numTracesAnalyzed, trace.DoBaseline(1,20) );
	EndAnalyze(trace);
    }
}
