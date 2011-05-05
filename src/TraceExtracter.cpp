/** \file TraceExtracter.cpp
 *  \brief Extract traces for a specific type and subtype
 */

#include <string>

#include "Trace.h"
#include "TraceExtracter.h"

#include "damm_plotids.h"

using std::string;

const int TraceExtracter::traceBins = SC;
const int TraceExtracter::numTraces = 99;

TraceExtracter::TraceExtracter(const std::string& aType, const std::string &aSubtype) : 
  TraceAnalyzer(), type(aType), subtype(aSubtype)
{
    name = "Extracter";
}

TraceExtracter::~TraceExtracter()
{
    // do nothing
}

/** Declare the damm plots */
void TraceExtracter::DeclarePlots(void) const
{
    using namespace dammIds::trace;
    for (int i=0; i < numTraces; i++)
	DeclareHistogram1D(D_TRACE + i, traceBins, "traces data");
}

/** Plot the damm spectra of the first few traces analyzed with (level >= 1) */
void TraceExtracter::Analyze(Trace &trace,
			     const string &aType, const string &aSubtype)
{   
    using namespace dammIds::trace;

    if (type ==  aType && subtype == aSubtype && numTracesAnalyzed < numTraces) {       
        TraceAnalyzer::Analyze(trace, type, subtype);
	trace.Plot(D_TRACE + numTracesAnalyzed);
    }
    EndAnalyze(trace);
}
