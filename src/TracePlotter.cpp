/** \file TracePlotter.cpp
 *  \brief A small clas which takes care of plotting the traces of channel events
 */

#include <string>

#include "Trace.h"
#include "TracePlotter.h"

#include "damm_plotids.h"

using std::string;

const int TracePlotter::traceBins = SC;
const int TracePlotter::numTraces = S5;

TracePlotter::TracePlotter() : TraceAnalyzer()
{
    name = "Plotter";
    // do nothing
}

TracePlotter::~TracePlotter()
{
    // do nothing
}

/** Declare the damm plots */
void TracePlotter::DeclarePlots(void) const
{
    using namespace dammIds::trace;

    TraceAnalyzer::DeclarePlots();
    DeclareHistogram2D(DD_TRACE, traceBins, numTraces, "traces data");
}

/** Plot the damm spectra of the first few traces analyzed with (level >= 1) */
void TracePlotter::Analyze(Trace &trace,
			   const string &type, const string &subtype)
{   
    using namespace dammIds::trace;

    TraceAnalyzer::Analyze(trace, type, subtype);
   
    if (level >= 1 && numTracesAnalyzed < numTraces) {       
	for (Trace::size_type i=0; i < trace.size(); i++) {
	    plot(DD_TRACE, i, numTracesAnalyzed, trace[i]);
	}
    }
    EndAnalyze(trace);
}
