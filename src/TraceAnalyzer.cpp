/**    \file TraceAnalyzer.cpp
 *     \brief defines the Trace class.
 *
 *     Implements a quick online trapezoidal filtering mechanism
 *     for the identification of double pulses
 *
 *     - SNL - 7-2-07 - created
 *     - SNL - 2-4-08 - Add plotting spectra
 */

#include <iostream>
#include <string>

#include "damm_plotids.h"
#include "TraceAnalyzer.h"

using std::cout;
using std::endl;
using std::string;

/**
 * Initialize the trace analysis class.  Set the row numbers
 * for spectra 850 to zero
 */
bool TraceAnalyzer::Init(void)
{   
    numTracesAnalyzed = 0;

    return true;
}

/**
 * Set default filter parameters
 */
TraceAnalyzer::TraceAnalyzer() : 
    userTime(0.), systemTime(0.)
{
    clocksPerSecond = sysconf(_SC_CLK_TCK);
}

/** Output time processing traces */
TraceAnalyzer::~TraceAnalyzer() 
{
    cout << "Trace processor : " 
	 << userTime << " user time, "
	 << systemTime << " system time" << endl;
}

/**
 * Function to quickly analyze a trace online.
 */
void TraceAnalyzer::Analyze(const Trace &trace,
			    const string &detType, const string &detSubtype)
{
    tms tmsBegin;

    times(&tmsBegin); // begin timing process
    
    TracePlot(trace);
  
    tms tmsEnd;
    times(&tmsEnd);

    userTime += (tmsEnd.tms_utime - tmsBegin.tms_utime) / clocksPerSecond;
    systemTime += (tmsEnd.tms_stime - tmsBegin.tms_stime) / clocksPerSecond; 

    return;
}

/** declare the damm plots */
void TraceAnalyzer::DeclarePlots() const
{
    using namespace dammIds::trace;

    const int traceBins = SB;
    const int numTraces = S5;

    DeclareHistogram2D(DD_TRACE, traceBins, numTraces, "traces data");
}

/**
 * From the trace analysis, plot the damm spectra
 *   including the first few traces analyzed
 */
void TraceAnalyzer::TracePlot(const Trace &trace)			      
{
    using namespace dammIds::trace;
    
    // plot trace and associated filters and derived energies
    for(unsigned int i = 0; i < trace.size(); i++){
      plot(DD_TRACE, i, numTracesAnalyzed, trace[i]);
    }   
}
