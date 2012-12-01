/** \file TracePlotter.cpp
 *  \brief A small class which takes care of plotting the traces of channel events
 */

#include <string>
#include <iostream>

#include "Trace.hpp"
#include "TracePlotter.hpp"

#include "DammPlotIds.hpp"

using std::string;
using namespace dammIds::trace;

const int TracePlotter::traceBins = SC;
const int TracePlotter::numTraces = S5;

TracePlotter::TracePlotter() : TraceAnalyzer(plotter::OFFSET, plotter::RANGE)
{
    name = "Plotter";
    // do nothing
}

TracePlotter::TracePlotter(int offset, int range) : TraceAnalyzer(offset, range)
{
    name = "Plotter";
    // do nothing
}

TracePlotter::~TracePlotter()
{
    // do nothing
}

/** Declare the damm plots */
void TracePlotter::DeclarePlots(void)
{
    //TraceAnalyzer::DeclarePlots();
    DeclareHistogram2D(plotter::DD_TRACE, traceBins, numTraces, "traces data TracePlotter");
}

/** Plot the damm spectra of the first few traces analyzed with (level >= 1) */
void TracePlotter::Analyze(Trace &trace,
			   const string &type, const string &subtype)
{   
    TraceAnalyzer::Analyze(trace, type, subtype);
   
    if (level >= 1 && numTracesAnalyzed < numTraces) {       
        trace.Plot(plotter::DD_TRACE, numTracesAnalyzed);
    }
    EndAnalyze(trace);
}
