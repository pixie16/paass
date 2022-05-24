/** @file CloverFragProcessor.cpp
 *  @brief Processor for Clovers at Fragmentation Facilities (Based HEAVILY on CloverProcessor.cpp)
 *  @authors T.T. King
 *  @date Nov 2018
*/

#include "CloverFragProcessor.hpp"

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
#include "Display.h"
#include "Exceptions.hpp"
#include "Messenger.hpp"
#include "RawEvent.hpp"
#include "pugixml.hpp"

using namespace std;
using namespace dammIds::cloverFrag;

namespace dammIds {
//! Namespace containing histogram definitions for the Clover. We are overriding the CloverProcessor space so the processors are exclusive
namespace cloverFrag {
const unsigned int MAX_FRAGCLOVERS = 13;  //!< for *_DETX spectra

const int CLOVER_LOW_OFFSET = 20;  ///!< offset for clover low versions

const int D_CLOVERFRAG_ENERGY = 0;            //!< Energy
const int D_CLOVERFRAG_ENERGY_CLOVERX = 1;    //!< Energy Full Clover
const int D_CLOVERFRAG_MULTI = 39;             //!<Multiplicity
const int D_CLOVERFRAG_ENERGY_DECAY = 40;     //!<Rough Decay Gated Energy
const int D_CLOVERFRAG_ENERGY_IMPLANT = 41;   //!<Rough IMPLANT Gated Energy
const int D_CLOVERFRAG_ENERGY_ANTIVETO = 42;  //!<Anti veto Gated Energy

}  // namespace cloverFrag
}  // namespace dammIds

CloverFragProcessor::CloverFragProcessor(double gammaThreshold, double vetoThresh, double ionTrigThresh, double betaThresh) : EventProcessor(OFFSET, RANGE, "CloverFragProcessor") {
    associatedTypes.insert("clover");

    gammaThresh_ = gammaThreshold;
    vetoThresh_ = vetoThresh;
    ionTrigThresh_ = ionTrigThresh;
    betaThresh_ = betaThresh;
}

/** Declare plots including many for decay/implant/neutron gated analysis  */
void CloverFragProcessor::DeclarePlots(void) {
    const int energyBins1 = SD;

    //build the clovers from the channel list
    auto cloverLocs_ = DetectorLibrary::get()->GetLocations("clover", "clover_high");
    CloverBuilder(cloverLocs_);

    DeclareHistogram1D(D_CLOVERFRAG_ENERGY, energyBins1, "Ungated Clover_High Singles");
    DeclareHistogram1D(D_CLOVERFRAG_MULTI, S5, "Clover Multiplicity");
    DeclareHistogram1D(D_CLOVERFRAG_ENERGY_DECAY, energyBins1, "Rough Decay Gated Clover_High Singles");
    DeclareHistogram1D(D_CLOVERFRAG_ENERGY_IMPLANT, energyBins1, "Rough Implant Gated Clover_High Singles");
    DeclareHistogram1D(D_CLOVERFRAG_ENERGY_ANTIVETO, energyBins1, "Anti-Gated on Veto Clover_High Singles");

    DeclareHistogram1D(D_CLOVERFRAG_ENERGY + CLOVER_LOW_OFFSET, energyBins1, "Ungated Clover_LOW Singles");
    DeclareHistogram1D(D_CLOVERFRAG_MULTI + CLOVER_LOW_OFFSET, S5, "Clover Multiplicity");
    DeclareHistogram1D(D_CLOVERFRAG_ENERGY_DECAY + CLOVER_LOW_OFFSET, energyBins1, "Rough Decay Gated Clover_LOW Singles");
    DeclareHistogram1D(D_CLOVERFRAG_ENERGY_IMPLANT + CLOVER_LOW_OFFSET, energyBins1, "Rough Implant Gated Clover_LOW Singles");
    DeclareHistogram1D(D_CLOVERFRAG_ENERGY_ANTIVETO + CLOVER_LOW_OFFSET, energyBins1, "Anti-Gated on Veto Clover_LOW Singles");

    // for each clover
    for (unsigned int i = 0; i < numClovers; i++) {
        stringstream ss;
        ss << "Clover " << i << " gamma";
        DeclareHistogram1D(D_CLOVERFRAG_ENERGY_CLOVERX + i, energyBins1, ss.str().c_str());
        DeclareHistogram1D(D_CLOVERFRAG_ENERGY_CLOVERX + CLOVER_LOW_OFFSET + i, energyBins1, ss.str().c_str());
    }
}

bool CloverFragProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    return true;
}

bool CloverFragProcessor::Process(RawEvent &event) {
    using namespace dammIds::cloverFrag;

    if (!EventProcessor::Process(event))
        return false;

    //vector <ChanEvent*>
    static const auto &cloverEvents = event.GetSummary("clover", true)->GetList();
    static const auto &vetoEvents = event.GetSummary("pspmt:RIT", true)->GetList();
    static const auto &ionTriggerEvents = event.GetSummary("pspmt:FIT", true)->GetList();
    static const auto &pspmtBetaEvents = event.GetSummary("pspmt:dynode_high", true)->GetList();

    // loop through gater events for either light ion veto (behind) or ion trigger (in front)
    // the Ion trigger allows for beam diagnositcs though beam produced isomers
    // light ion veto will trigger with punch throughs (C, P+, alphas etc)
    //pspmt beta signal

    bool hasVeto = false;
    bool hasIonTrig = false;
    bool hasPspmtBeta = false;

    for (auto itVeto = vetoEvents.begin(); itVeto != vetoEvents.end(); itVeto++) {
        if ((*itVeto)->GetCalibratedEnergy() >= vetoThresh_)
            hasVeto = true;
    };

    for (auto itIon = ionTriggerEvents.begin(); itIon != ionTriggerEvents.end(); itIon++) {
        if ((*itIon)->GetCalibratedEnergy() >= ionTrigThresh_)
            hasIonTrig = true;
    };

    for (auto itPbeta = pspmtBetaEvents.begin(); itPbeta != pspmtBetaEvents.end(); itPbeta++) {
        if ((*itPbeta)->GetCalibratedEnergy() >= betaThresh_)
            hasPspmtBeta = true;
    };
    // end gater loops

    // start loop over clover events (start with multiplicity plot)
    plot(D_CLOVERFRAG_MULTI, cloverEvents.size());

    for (auto itClover = cloverEvents.begin(); itClover != cloverEvents.end(); itClover++) {
        double gEnergy = (*itClover)->GetCalibratedEnergy();
        int cloverNum = leafToClover[(*itClover)->GetChanID().GetLocation()];
        bool cloverHighGain = false;
        if ((*itClover)->GetChanID().GetSubtype() == "clover_high"){
                cloverHighGain = true;
            }

        if (gEnergy < gammaThresh_)
            continue;

        //we call it a "decay" if we dont have a veto or ionTrigger and we have a pspmt:dynode_high
        bool isDecay = false;
        if (!hasIonTrig && !hasVeto && hasPspmtBeta) {
            isDecay = true;
        }
        if (cloverHighGain) {
            plot(D_CLOVERFRAG_ENERGY, gEnergy);
            plot(D_CLOVERFRAG_ENERGY_CLOVERX + cloverNum, gEnergy);
            if (isDecay) {
                plot(D_CLOVERFRAG_ENERGY_DECAY, gEnergy);
            }
            if (hasIonTrig) {
                plot(D_CLOVERFRAG_ENERGY_IMPLANT, gEnergy);
            }
            if (!hasVeto) {
                plot(D_CLOVERFRAG_ENERGY_ANTIVETO, gEnergy);
            }
        } else {
            plot(D_CLOVERFRAG_ENERGY + CLOVER_LOW_OFFSET, gEnergy);
            plot(D_CLOVERFRAG_ENERGY_CLOVERX + CLOVER_LOW_OFFSET + cloverNum, gEnergy);
            if (isDecay) {
                plot(D_CLOVERFRAG_ENERGY_DECAY + CLOVER_LOW_OFFSET, gEnergy);
            }
            if (hasIonTrig) {
                plot(D_CLOVERFRAG_ENERGY_IMPLANT + CLOVER_LOW_OFFSET, gEnergy);
            }
            if (!hasVeto) {
                plot(D_CLOVERFRAG_ENERGY_ANTIVETO + CLOVER_LOW_OFFSET, gEnergy);
            }
        }

        if (DetectorDriver::get()->GetSysRootOutput()) {
            //Fill root struct and push back on to vector3
            Cstruct.cloverHigh = cloverHighGain;
            Cstruct.rawEnergy = (*itClover)->GetEnergy();
            Cstruct.energy = gEnergy;
            Cstruct.time = (*itClover)->GetTimeSansCfd() * Globals::get()->GetClockInSeconds() * 1e9;  //store ns
            Cstruct.detNum = (*itClover)->GetChanID().GetLocation();
            Cstruct.cloverNum = cloverNum;
            pixie_tree_event_->clover_vec_.emplace_back(Cstruct);
            Cstruct = processor_struct::CLOVER_DEFAULT_STRUCT;  //reset to initalized values (see PaassRootStruct.hpp
        }
    }
    EndProcess();
    return true;
}

void CloverFragProcessor::CloverBuilder(const std::set<int> &cloverLocs) {
    /*  clover specific routine, determine the number of clover detector
       channels and divide by num of chans per clover to find the total number of clovers
       ORNL clovers are 4
    */
    auto cloverLocations = &cloverLocs;
    // could set it now but we'll iterate through the locations to set this
    unsigned int cloverChans = 0;

    for (set<int>::const_iterator it = cloverLocations->begin(); it != cloverLocations->end(); it++) {
        leafToClover[*it] = int(cloverChans / chansPerClover);
        cloverChans++;
    }

    if (cloverChans % chansPerClover != 0) {
        stringstream ss;
        ss << " There does not appear to be the proper number of"
           << " channels per clover.";
        throw GeneralException(ss.str());
    }
    if (cloverChans != 0) {
        numClovers = cloverChans / chansPerClover;
        Messenger m;
        m.start("Building clovers");

        stringstream ss;
        ss << "A total of " << cloverChans
           << " clover channels were detected: ";
        int lastClover = numeric_limits<int>::min();
        for (map<int, int>::const_iterator it = leafToClover.begin();
             it != leafToClover.end(); it++) {
            if (it->second != lastClover) {
                m.detail(ss.str());
                ss.str("");
                lastClover = it->second;
                ss << "Clover " << lastClover << " : ";
            } else {
                ss << ", ";
            }
            ss << setw(2) << it->first;
        }
        m.detail(ss.str());

        if (numClovers > dammIds::cloverFrag::MAX_FRAGCLOVERS) {
            m.fail();
            stringstream ss;
            ss << "Number of detected clovers is greater than defined"
               << " MAX_CLOVERS = " << dammIds::cloverFrag::MAX_FRAGCLOVERS << "."
               << " See CloverProcessor.hpp for details.";
            throw GeneralException(ss.str());
        }
        m.done();
    }
}
