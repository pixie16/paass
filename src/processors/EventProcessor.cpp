/*! \file EventProcessor.cpp
 * \brief Implementation of a generic event processor
 * \author David Miller
 * \date August 2009
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <sstream>
#include <vector>

#include <unistd.h>
#include <sys/times.h>

#include "DetectorLibrary.hpp"
#include "EventProcessor.hpp"
#include "RawEvent.hpp"
#include "Messenger.hpp"

using namespace std;

EventProcessor::EventProcessor() :
  name("generic"), initDone(false), didProcess(false), histo(0, 0, "generic"),
  userTime(0.), systemTime(0.) {
    clocksPerSecond = sysconf(_SC_CLK_TCK);
}

EventProcessor::EventProcessor(int offset, int range, std::string proc_name) :
  name(proc_name), initDone(false), didProcess(false),
  histo(offset, range, proc_name), userTime(0.), systemTime(0.) {
    clocksPerSecond = sysconf(_SC_CLK_TCK);
}

EventProcessor::~EventProcessor() {
    if (initDone) {
        cout << "processor " << name << " : "
            << userTime << " user time, "
            << systemTime << " system time" << endl;
    }
}

bool EventProcessor::HasEvent(void) const {
    for (map<string, const DetectorSummary*>::const_iterator it = sumMap.begin();
	 it != sumMap.end(); it++) {
        if (it->second->GetMult() > 0) {
            return true;
        }
    }
    return false;
}

bool EventProcessor::Init(RawEvent& rawev) {
    vector<string> intersect;
    const set<string> &usedDets = DetectorLibrary::get()->GetUsedDetectors();

    set_intersection(associatedTypes.begin(), associatedTypes.end(),
                     usedDets.begin(), usedDets.end(),
                     back_inserter(intersect) );

    if (intersect.empty()) {
        return false;
    }

    for (vector<string>::const_iterator it = intersect.begin();
	 it != intersect.end(); it++) {
        sumMap.insert( make_pair(*it, rawev.GetSummary(*it)) );
    }

    initDone = true;
    Messenger m;
    stringstream ss;
    ss << "processor " << name << " initialized operating on "
       << intersect.size() << " detector type(s).";
    m.detail(ss.str());

    return true;
}

bool EventProcessor::PreProcess(RawEvent &event) {
    if (!initDone)
        return (didProcess = false);
    return (didProcess = true);
}

bool EventProcessor::Process(RawEvent &event) {
    if (!initDone)
        return (didProcess = false);

    times(&tmsBegin);

    EndProcess();
    return (didProcess = true);
}

void EventProcessor::EndProcess(void) {
    tms tmsEnd;

    times(&tmsEnd);

    userTime += (tmsEnd.tms_utime - tmsBegin.tms_utime) / clocksPerSecond;
    systemTime += (tmsEnd.tms_stime - tmsBegin.tms_stime) / clocksPerSecond;

    //! Reset the beginning time so multiple calls of EndProcess from
    //! derived classes work properly
    times(&tmsBegin);
}


