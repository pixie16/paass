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
        name("generic"), initDone(false), didProcess(false),
        histo(0, 0, "generic"){
    preprocesscalls = 0;
    processcalls = 0;
    postprocesscalls = 0;
    preprocesstime = 0;
    processtime = 0;
    postprocesstime = 0;
    currstep = STEP::UNKNOWN;
}

EventProcessor::EventProcessor(std::string proc_name) :
        name(proc_name), initDone(false), didProcess(false),
        histo(0, 0, "generic"){
    preprocesscalls = 0;
    processcalls = 0;
    postprocesscalls = 0;
    preprocesstime = 0;
    processtime = 0;
    postprocesstime = 0;
    currstep = STEP::UNKNOWN;
}


EventProcessor::EventProcessor(int offset, int range, std::string proc_name) :
        name(proc_name), initDone(false), didProcess(false),
        histo(offset, range, proc_name){
    preprocesscalls = 0;
    processcalls = 0;
    postprocesscalls = 0;
    preprocesstime = 0;
    processtime = 0;
    postprocesstime = 0;
    currstep = STEP::UNKNOWN;
}

EventProcessor::~EventProcessor() {
	std::cout << name 
		  << " Preprocess: ( "<< preprocesscalls << " calls, " << preprocesstime << " s )"
		  << " Process: ( "<< processcalls << " calls, " << processtime << " s )"
		  << " Postprocess: ( "<< postprocesscalls << " calls, " << postprocesstime << " s )"
		  << std::endl;
}

bool EventProcessor::HasEvent(void) const {
    for (map<string, const DetectorSummary *>::const_iterator it = sumMap.begin();
         it != sumMap.end(); it++) {
        if (it->second->GetMult() > 0) {
            return (true);
        }
    }
    return (false);
}

bool EventProcessor::Init(RawEvent &rawev) {
    vector<string> intersect;
    const set <string> &usedDets = DetectorLibrary::get()->GetUsedDetectors();

    set_intersection(associatedTypes.begin(), associatedTypes.end(),
                     usedDets.begin(), usedDets.end(),
                     back_inserter(intersect));

    if (intersect.empty())
        return (false);

    for (vector<string>::const_iterator it = intersect.begin();
         it != intersect.end(); it++) {
        sumMap.insert(make_pair(*it, rawev.GetSummary(*it)));
    }

    initDone = true;
    Messenger m;
    stringstream ss;
    ss << "processor " << name << " initialized operating on "
       << intersect.size() << " detector type(s).";
    m.detail(ss.str());

    return (true);
}

bool EventProcessor::PreProcess(RawEvent &event) {
    start_time = std::chrono::high_resolution_clock::now();
    currstep = STEP::PREPROCESS;
    ++preprocesscalls;
    if (!initDone)
        return (didProcess = false);
    return (didProcess = true);
}

bool EventProcessor::Process(RawEvent &event) {
    start_time = std::chrono::high_resolution_clock::now();
    currstep = STEP::PROCESS;
    ++processcalls;
    if (!initDone)
        return (didProcess = false);
    return (didProcess = true);
}

void EventProcessor::EndProcess(void) {
    stop_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> dur = stop_time - start_time;
    switch(currstep){
	    case STEP::PREPROCESS:
		    preprocesstime += dur.count();
		    break;
	    case STEP::PROCESS:
		    processtime += dur.count();
		    break;
	    case STEP::POSTPROCESS:
		    postprocesstime += dur.count();
		    break;
            default:
		    break;
    }
}
