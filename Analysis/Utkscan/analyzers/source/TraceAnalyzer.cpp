/** \file TraceAnalyzer.cpp
 * \brief defines the Trace class.
 *
 * Implements a quick online trapezoidal filtering mechanism
 * for the identification of double pulses
 *
 * \author S. Liddick, D. Miller, S. V. Paulauskas
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

int TraceAnalyzer::numTracesAnalyzed = -1; //!< number of analyzed traces

TraceAnalyzer::TraceAnalyzer() : histo(0, 0, "generic"), userTime(0.), systemTime(0.) {
    clocksPerSecond = sysconf(_SC_CLK_TCK);
}

TraceAnalyzer::TraceAnalyzer(const unsigned int &offset, const unsigned int &range, const std::string &name) :
        histo(offset, range, name), userTime(0.), systemTime(0.) {
    clocksPerSecond = sysconf(_SC_CLK_TCK);
}

TraceAnalyzer::~TraceAnalyzer() {
    cout << name << " analyzer : " << userTime << " user time, " << systemTime << " system time" << endl;
}

void TraceAnalyzer::Plot(const vector<unsigned int> &trc, const int &id) {
    for (unsigned int i = 0; i < trc.size(); i++)
        histo.Plot(id, i, 1, (int) trc.at(i));
}

void TraceAnalyzer::Plot(const vector<unsigned int> &trc, int id, int row) {
    for (unsigned int i = 0; i < trc.size(); i++)
        histo.Plot(id, i, row, (int) trc.at(i));
}

void TraceAnalyzer::ScalePlot(const vector<unsigned int> &trc, int id, double scale) {
    for (unsigned int i = 0; i < trc.size(); i++)
        histo.Plot(id, i, 1, abs((int) trc.at(i)) / scale);
}

void TraceAnalyzer::ScalePlot(const vector<unsigned int> &trc, int id, int row, double scale) {
    for (unsigned int i = 0; i < trc.size(); i++)
        histo.Plot(id, i, row, abs((int) trc.at(i)) / scale);
}

void TraceAnalyzer::OffsetPlot(const vector<unsigned int> &trc, int id, double offset) {
    for (unsigned int i = 0; i < trc.size(); i++)
        histo.Plot(id, i, 1, max(0., (int) trc.at(i) - offset));
}

void TraceAnalyzer::OffsetPlot(const vector<unsigned int> &trc, int id, int row, double offset) {
    for (unsigned int i = 0; i < trc.size(); i++)
        histo.Plot(id, i, row, max(0., (int) trc.at(i) - offset));
}

void TraceAnalyzer::Analyze(Trace &trace, const ChannelConfiguration &cfg) {
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

bool TraceAnalyzer::IsIgnored(const std::set<std::string> &list, const ChannelConfiguration &id){ bool retVal;
    if (list.find(id.GetType()) != list.end()) {
        retVal = true;
    } else if (list.find(id.GetType() + ":" + id.GetSubtype()) != list.end()) {
        retVal = true;
    } else if (list.find(id.GetType() + ":" + id.GetSubtype() + ":" + id.GetGroup()) != list.end()) {
        retVal = true;
    } else {
        retVal = false;
    }
    return retVal;
}