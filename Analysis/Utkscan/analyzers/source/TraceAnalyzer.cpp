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

int TraceAnalyzer::numTracesAnalyzed = 0; //!< number of analyzed traces

using namespace dammIds::trace;

TraceAnalyzer::TraceAnalyzer() : userTime(0.), systemTime(0.) {
    name = "Trace";
    // start at -1 so that when incremented on first trace analysis,
    //   row 0 is respectively filled in the trace spectrum of inheritees
    numTracesAnalyzed = -1;
    clocksPerSecond = sysconf(_SC_CLK_TCK);
}

TraceAnalyzer::~TraceAnalyzer()
{
    cout << name << " analyzer : "
	 << userTime << " user time, "
	 << systemTime << " system time" << endl;
}

void TraceAnalyzer::Analyze(Trace &trace,
			    const std::string &detType, const std::string &detSubtype) {
    times(&tmsBegin);
    numTracesAnalyzed++;
    EndAnalyze(trace);
    return;
}


void TraceAnalyzer::Analyze(Trace &trace,
			    const std::string &detType, const std::string &detSubtype,
                            const std::map<std::string, int> & tagMap) {
    times(&tmsBegin);
    numTracesAnalyzed++;
    EndAnalyze(trace);
    return;
}

void TraceAnalyzer::EndAnalyze(Trace &trace) {
    trace.SetValue("analyzedLevel", level);
    EndAnalyze();
}

void TraceAnalyzer::EndAnalyze(void) {
    tms tmsEnd;
    times(&tmsEnd);

    userTime += (tmsEnd.tms_utime - tmsBegin.tms_utime) / clocksPerSecond;
    systemTime += (tmsEnd.tms_stime - tmsBegin.tms_stime) / clocksPerSecond;

    // reset the beginning time so multiple calls of EndAnalyze from
    //   derived classes work properly
    times(&tmsBegin);
}
