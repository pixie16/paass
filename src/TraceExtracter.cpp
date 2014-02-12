/** \file TraceExtracter.cpp
 *  \brief Extract traces for a specific type and subtype
 */

#include <string>
#include <sstream>

#include "Trace.hpp"
#include "TraceExtracter.hpp"
#include "DammPlotIds.hpp"

using std::string;
using namespace dammIds::trace;

const int TraceExtracter::traceBins = SC;
const int TraceExtracter::numTraces = dammIds::trace::maxSingleTraces;

TraceExtracter::TraceExtracter(const std::string& aType, 
                            const std::string &aSubtype) : 
                                type(aType), subtype(aSubtype)
{
    name = "Extracter";
}

TraceExtracter::~TraceExtracter()
{
    // do nothing
}

/** Declare the damm plots */
void TraceExtracter::DeclarePlots(void)
{
    Trace sample_trace = Trace();
    for (int i = 0; i < numTraces; ++i) {
        std::stringstream ss;
        ss << "TraceExtracter trace " << i;
        sample_trace.DeclareHistogram1D(D_TRACE + i, traceBins, 
                                        ss.str().c_str());
    }
}

/** Plot the damm spectra of the first few traces analyzed with (level >= 1) */
void TraceExtracter::Analyze(Trace &trace,
			     const string &aType, const string &aSubtype)
{   
    using namespace dammIds::trace;

    if (type ==  aType && 
        subtype == aSubtype && 
        numTracesAnalyzed < numTraces) {	
        TraceAnalyzer::Analyze(trace, type, subtype);	
        trace.OffsetPlot(D_TRACE + numTracesAnalyzed, trace.DoBaseline(1,20) );
        EndAnalyze(trace);
    }
}
