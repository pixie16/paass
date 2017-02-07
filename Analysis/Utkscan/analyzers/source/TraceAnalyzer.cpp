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
#include <algorithm>
#include <iostream>
#include <string>

#include <cmath>

#include <unistd.h>

#include "DammPlotIds.hpp"
#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

using namespace std;
using namespace dammIds::trace;

Plots TraceAnalyzer::histo(OFFSET, RANGE, "Analyzers");

int TraceAnalyzer::numTracesAnalyzed = 0; //!< number of analyzed traces

void TraceAnalyzer::Plot(const vector<unsigned int> &trc,
                         const int &id) {
    for (unsigned int i=0; i < trc.size(); i++) {
        histo.Plot(id, i, 1, (int)trc.at(i));
    }
}

void TraceAnalyzer::Plot(const vector<unsigned int> &trc, int id, int row) {
    for (unsigned int i=0; i < trc.size(); i++) {
        histo.Plot(id, i, row, (int)trc.at(i));
    }
}

void TraceAnalyzer::ScalePlot(const vector<unsigned int> &trc, int id, double
scale) {
    for (unsigned int i=0; i < trc.size(); i++) {
        histo.Plot(id, i, 1, abs((int)trc.at(i)) / scale);
    }
}

void TraceAnalyzer::ScalePlot(const vector<unsigned int> &trc, int id, int
row, double scale) {
    for (unsigned int i=0; i < trc.size(); i++) {
        histo.Plot(id, i, row, abs((int)trc.at(i)) / scale);
    }
}

void TraceAnalyzer::OffsetPlot(const vector<unsigned int> &trc, int id,
                               double offset) {
    for (unsigned int i=0; i < trc.size(); i++) {
        histo.Plot(id, i, 1, max(0., (int)trc.at(i) - offset));
    }
}

void TraceAnalyzer::OffsetPlot(const vector<unsigned int> &trc, int id, int
row, double offset) {
    for (unsigned int i=0; i < trc.size(); i++) {
        histo.Plot(id, i, row, max(0., (int)trc.at(i) - offset));
    }
}

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
