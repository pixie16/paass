/** \file TraceAnalyzer.cpp
 * \brief defines the Trace class.
 *
 * Implements a quick online trapezoidal filtering mechanism
 * for the identification of double pulses
 *
 * \author S. Liddick 
 * \date 7-2-07
 * <strong>Modified : </strong> SNL - 2-4-08 - Add plotting spectra
 */

#include <iostream>
#include <string>

#include <unistd.h>

#include "DammPlotIds.hpp"
#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

using std::cout;
using std::endl;
using std::string;

/**
 * Initialize the trace analysis class.  Set the row numbers
 * for spectra 850 to zero
 */
bool TraceAnalyzer::Init(void)
{   
    return true;
}

/**
 * Set default filter parameters
 */

using namespace dammIds::trace;

TraceAnalyzer::TraceAnalyzer() : 
    userTime(0.), systemTime(0.),
    histo(OFFSET, RANGE) 
{
    name = "Trace";
    // start at -1 so that when incremented on first trace analysis,
    //   row 0 is respectively filled in the trace spectrum of inheritees 
    numTracesAnalyzed = -1;    
    clocksPerSecond = sysconf(_SC_CLK_TCK);
}

TraceAnalyzer::TraceAnalyzer(int offset, int range) : 
    userTime(0.), systemTime(0.),
    histo(offset, range) 
{
    name = "Trace";
    // start at -1 so that when incremented on first trace analysis,
    //   row 0 is respectively filled in the trace spectrum of inheritees 
    numTracesAnalyzed = -1;    
    clocksPerSecond = sysconf(_SC_CLK_TCK);
}

/** Output time processing traces */
TraceAnalyzer::~TraceAnalyzer() 
{
    cout << name << " analyzer : " 
	 << userTime << " user time, "
	 << systemTime << " system time" << endl;
}

/**
 * Function to quickly analyze a trace online.
 */
void TraceAnalyzer::Analyze(Trace &trace,
			    const string &detType, const string &detSubtype)
{
    times(&tmsBegin); // begin timing process
    numTracesAnalyzed++;
    EndAnalyze(trace);
    return;
}

/**
 * End the analysis and record the analyzer level in the trace
 */
void TraceAnalyzer::EndAnalyze(Trace &trace)
{
    trace.SetValue("analyzedLevel", level);
    EndAnalyze();
}

/**
 * Finish analysis updating the analyzer timing information
 */
void TraceAnalyzer::EndAnalyze(void)
{    
    tms tmsEnd;
    times(&tmsEnd);

    userTime += (tmsEnd.tms_utime - tmsBegin.tms_utime) / clocksPerSecond;
    systemTime += (tmsEnd.tms_stime - tmsBegin.tms_stime) / clocksPerSecond;

    // reset the beginning time so multiple calls of EndAnalyze from
    //   derived classes work properly
    times(&tmsBegin);
}

/** declare the damm plots */
void TraceAnalyzer::DeclarePlots()
{
    // do nothing
}
