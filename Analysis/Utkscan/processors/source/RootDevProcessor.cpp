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

#include "StringManipulationFunctions.hpp"

#include "DetectorDriver.hpp"
#include "DetectorLibrary.hpp"
#include "HelperFunctions.hpp"
#include "RawEvent.hpp"
#include "RootDevProcessor.hpp"

using namespace std;

RootDevProcessor::RootDevProcessor(std::string additionaltypes) : EventProcessor("RootDevProcessor") {
    //associatedTypes.insert("RD");
    typestoread = std::vector<std::string>();
    typestoread.push_back("RD");
    //parse additionaltypes here
    if( not additionaltypes.empty()){
    	auto typelist = StringManipulation::TokenizeString(additionaltypes,",");
    	for( size_t ii = 0; ii < typelist.size(); ++ii ){
		    typestoread.push_back(typelist.at(ii));
    	}
    }
    for( size_t ii = 0; ii < typestoread.size(); ++ii ){
	    associatedTypes.insert(typestoread.at(ii));
    }
    Rev = Globals::get()->GetPixieRevision();
}

bool RootDevProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    EndProcess();
    return true;
}

bool RootDevProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    for( size_t ii = 0; ii < typestoread.size(); ++ii ){
	    const auto &Events = event.GetSummary(typestoread.at(ii), true)->GetList();

	    for (auto it = Events.begin(); it != Events.end(); it++) {
		    RDstruct.energy = (*it)->GetCalibratedEnergy();
		    RDstruct.rawEnergy = (*it)->GetEnergy();
		    if (Rev == "F") {
			    RDstruct.timeSansCfd = (*it)->GetTimeSansCfd() * Globals::get()->GetClockInSeconds((*it)->GetChanID().GetModFreq()) * 1e9;
			    RDstruct.time = (*it)->GetTime() * Globals::get()->GetAdcClockInSeconds((*it)->GetChanID().GetModFreq()) * 1e9;
			    RDstruct.cfdForcedBit = (*it)->GetCfdForcedTriggerBit();
			    RDstruct.cfdFraction = (*it)->GetCfdFractionalTime();
			    RDstruct.cfdSourceBit = (*it)->GetCfdTriggerSourceBit();
		    } else {
			    RDstruct.timeSansCfd = (*it)->GetTimeSansCfd() * Globals::get()->GetClockInSeconds() * 1e9;
			    RDstruct.time = (*it)->GetTime() * Globals::get()->GetAdcClockInSeconds() * 1e9;
		    }
		    RDstruct.detNum = (*it)->GetChanID().GetLocation();
		    RDstruct.crateNum = (*it)->GetCrateNumber();
		    RDstruct.modNum = (*it)->GetModuleNumber();
		    RDstruct.chanNum = (*it)->GetChannelNumber();
		    RDstruct.type = (*it)->GetChanID().GetType();
		    RDstruct.subtype = (*it)->GetChanID().GetSubtype();
		    RDstruct.group = (*it)->GetChanID().GetGroup();
		    auto tagset = (*it)->GetChanID().GetTags();
		    for( auto& tag : tagset )
			    RDstruct.tag.push_back(tag);
		    RDstruct.pileup = (*it)->IsPileup();
		    RDstruct.saturation = (*it)->IsSaturated();

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
			    if (Rev == "F") {
				    RDstruct.phase = (*it)->GetTrace().GetPhase() * Globals::get()->GetAdcClockInSeconds((*it)->GetChanID().GetModFreq()) * 1e9;
			    } else {
				    RDstruct.phase = (*it)->GetTrace().GetPhase() * Globals::get()->GetAdcClockInSeconds() * 1e9;
			    }
		    }
		    if (!(*it)->GetQdc().empty()) {
			    RDstruct.qdcSums = (*it)->GetQdc();
		    }
		    pixie_tree_event_->rootdev_vec_.emplace_back(RDstruct);
		    RDstruct = processor_struct::ROOTDEV_DEFAULT_STRUCT;
	    }
    }

    EndProcess();
    return true;
}
