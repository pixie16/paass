/** @file RootDevProcessor.cpp
 *  @brief Basic ROOT output. Fills a generic struc in the same tree layout as the other processors. It has NO damm output
 *  @authors T.T. King
 *  @date 03/30/2019
*/

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "DetectorDriver.hpp"
#include "DetectorLibrary.hpp"
#include "HelperFunctions.hpp"
#include "RawEvent.hpp"
#include "RootDevProcessor.hpp"

using namespace std;

RootDevProcessor::RootDevProcessor() : EventProcessor() {
    associatedTypes.insert("RD");
}

bool RootDevProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    return true;
}

bool RootDevProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    //vector <ChanEvent*>
    static const auto &Events = event.GetSummary("RD", true)->GetList();

    for (auto it = Events.begin(); it != Events.end(); it++) {
        RDstruct.energy = (*it)->GetCalibratedEnergy();
        RDstruct.rawEnergy = (*it)->GetEnergy();
        RDstruct.timeSansCfd = (*it)->GetTimeSansCfd() * Globals::get()->GetClockInSeconds((*it)->GetChanID().GetModFreq());
        RDstruct.time = (*it)->GetTime() * Globals::get()->GetAdcClockInSeconds((*it)->GetChanID().GetModFreq());
        RDstruct.detNum = (*it)->GetChanID().GetLocation();
        RDstruct.modNum = (*it)->GetModuleNumber();
        RDstruct.chanNum = (*it)->GetChannelNumber();
        RDstruct.subtype = (*it)->GetChanID().GetSubtype();
        RDstruct.group = (*it)->GetChanID().GetGroup();
        RDstruct.pileup = (*it)->IsPileup();
        RDstruct.saturation = (*it)->IsSaturated();

        if ((*it)->GetTrace().size() > 0) {
            RDstruct.trace = (*it)->GetTrace();
            RDstruct.maxPos = (*it)->GetTrace().GetMaxInfo().first;
            RDstruct.maxVal = (*it)->GetTrace().GetMaxInfo().second;
            RDstruct.tqdc = (*it)->GetTrace().GetQdc();
            RDstruct.highResTime = (*it)->GetHighResTimeInNs();
        }
        if (!(*it)->GetQdc().empty()) {
            RDstruct.qdcSums = (*it)->GetQdc();
        }
        pixie_tree_event_->root_dev_vec_.emplace_back(RDstruct);
        RDstruct = processor_struct::ROOTDEV_DEFAULT_STRUCT;
    }

    EndProcess();
    return true;
}
