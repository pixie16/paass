/** @file RootDevProcessor.cpp
 *  @brief Basic ROOT output. Fills a generic struc in the same tree layout as the other processors. It has NO damm output
 *  @authors T.T. King
 *  @date 03/30/2019
*/


#include "DetectorDriver.hpp"
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

    static const auto &Events = event.GetSummary("RD", true)->GetList();

    for (auto it = Events.begin(); it != Events.end(); it++) {
        RDstruct.energy = (*it)->GetCalibratedEnergy();
        RDstruct.rawEnergy = (*it)->GetEnergy();
        RDstruct.timeSansCfd = (*it)->GetTimeSansCfd() ;
        RDstruct.time = (*it)->GetTime() ;
        RDstruct.timeSansCfdInNs = (*it)->GetTimeSansCfdInNs() ;
        RDstruct.timeInNs = (*it)->GetTimeInNs() ;
        RDstruct.cfdForcedBit = (*it)->GetCfdForcedTriggerBit();
        RDstruct.cfdFraction = (*it)->GetCfdFractionalTime();
        RDstruct.cfdSourceBit = (*it)->GetCfdTriggerSourceBit();
        RDstruct.detNum = (*it)->GetChanID().GetLocation();
        RDstruct.channelID= (*it)->GetID();
        RDstruct.crateNum = (*it)->GetCrateNumber();
        RDstruct.modNum = (*it)->GetModuleNumber();
        RDstruct.chanNum = (*it)->GetChannelNumber();
        RDstruct.subtype = (*it)->GetChanID().GetSubtype();
        RDstruct.group = (*it)->GetChanID().GetGroup();
        RDstruct.pileup = (*it)->IsPileup();
        RDstruct.saturation = (*it)->IsSaturated();
        RDstruct.revision = (*it)->GetChanID().GetRevision();
        RDstruct.tickToNS = (*it)->GetChanID().GetTickToNS();
        RDstruct.adcTickToNS = (*it)->GetChanID().GetAdcTickToNS();

        if ((*it)->GetTrace().size() > 0) {
            RDstruct.hasValidTimingAnalysis = (*it)->GetTrace().HasValidTimingAnalysis();
            RDstruct.hasValidWaveformAnalysis = (*it)->GetTrace().HasValidWaveformAnalysis();
            RDstruct.baseline = (*it)->GetTrace().GetBaselineInfo().first;
            RDstruct.stdBaseline = (*it)->GetTrace().GetBaselineInfo().second;
            RDstruct.trace = (*it)->GetTrace();
            RDstruct.maxPos = (*it)->GetTrace().GetMaxInfo().first;
            RDstruct.maxVal = (*it)->GetTrace().GetMaxInfo().second;
            RDstruct.extMaxVal = (*it)->GetTrace().GetExtrapolatedMaxInfo().second;
            RDstruct.tqdc = (*it)->GetTrace().GetQdc();
            RDstruct.highResTime = (*it)->GetHighResTimeInNs();
            RDstruct.phase = (*it)->GetTrace().GetPhase() * (*it)->GetChanID().GetAdcTickToNS();
        }
        if (!(*it)->GetQdc().empty()) {
            RDstruct.qdcSums = (*it)->GetQdc();
        }
        pixie_tree_event_->rootdev_vec_.emplace_back(RDstruct);
        RDstruct = processor_struct::ROOTDEV_DEFAULT_STRUCT;
    }

    EndProcess();
    return true;
}
