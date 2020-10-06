///@file GeProcessor.cpp
///@brief Implementation for germanium processor
///@author D. Miller, K. Miernik, S. V. Paulauskas
///@date August 2009
#include "GeProcessor.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "DetectorLibrary.hpp"
#include "Exceptions.hpp"
#include "Messenger.hpp"

namespace dammIds {
namespace ge {
const int DD_ENERGY = 0;  //!< Energy
}
}  // namespace dammIds

using namespace std;
using namespace dammIds::ge;

GeProcessor::GeProcessor() : EventProcessor(OFFSET, RANGE, "GeProcessor") {
    associatedTypes.insert("ge");  // associate with germanium detectors
}

void GeProcessor::DeclarePlots(void) {
    DeclareHistogram2D(DD_ENERGY, SE, S6, "Calibrated Ge Singles");
}

bool GeProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    static const vector<ChanEvent *> &geEvents =
        event.GetSummary("ge", true)->GetList();

    for (vector<ChanEvent *>::const_iterator ge = geEvents.begin();
         ge != geEvents.end(); ge++) {
        if ((*ge)->IsSaturated() || (*ge)->IsPileup())
            continue;

        plot(DD_ENERGY, (*ge)->GetCalibratedEnergy(),
             (*ge)->GetChanID().GetLocation());

        if (DetectorDriver::get()->GetSysRootOutput()) {
            int module_freq = (*ge)->GetChanID().GetModFreq();
            //We are reusing the Clover Stuct definition. This means these well go into the clover_vec_
            GEstruct.rawEnergy = (*ge)->GetEnergy();
            GEstruct.energy = (*ge)->GetCalibratedEnergy();
            GEstruct.time = (*ge)->GetTimeSansCfd() * Globals::get()->GetClockInSeconds(module_freq) * 1e9;  //store ns
            GEstruct.detNum = (*ge)->GetChanID().GetLocation();
            GEstruct.cloverNum = -1; //We Define the Single Channel Ge detectors to be cloverNum -1 so that we can use both clovers and single channels at the same time. 
            pixie_tree_event_->clover_vec_.emplace_back(GEstruct);
            GEstruct = processor_struct::CLOVER_DEFAULT_STRUCT;  //reset to initalized values (see PaassRootStruct.hpp
        }
    }
    return true;
}