/*! \file NEXTBuilder.cpp
 *  \brief A class that builds bar type detectors
 *  \author S. V. Paulauskas
 *  \date December 15, 2014
*/
#include <iostream>
#include <vector>

#include "NEXTBuilder.hpp"
#include "TimingMapBuilder.hpp"

using namespace std;

void NEXTBuilder::BuildModules(void) {
    ClearMaps();
    FillMaps();

    for (map<unsigned int, unsigned int>::const_iterator it = lefts_.begin(); it != lefts_.end(); it++) {
        map<unsigned int, unsigned int>::const_iterator mate = rights_.find(it->first);
        if (mate == rights_.end())
            continue;

        for (vector<ChanEvent *>::const_iterator pt = pos_.begin(); pt != pos_.end(); pt++){
            ChannelConfiguration ia = (*pt)->GetChanID();
            std::string modID = ia.GetGroup();
            if (std::to_string(it->first) ==  modID ) {
                if (ia.HasTag("left"))  pos_lefts_.emplace((*pt));
                if (ia.HasTag("right")) pos_rights_.emplace((*it));
            }
        }

        if (list_.at(it->second)->GetTrace().size() != 0 && list_.at(mate->second)->GetTrace().size() != 0
             && pos_lefts_.size() >= 4 && pos_right_.size() >=4 ) {
            TimingDefs::TimingIdentifier key =
                    make_pair(it->first, list_.at(it->second)->GetChanID().GetSubtype());
            hrMods_.insert(make_pair(key, NEXTDetector(HighResTimingData(*(list_.at(it->second))), HighResPositionData(pos_lefts_),
                                                       HighResTimingData(*(list_.at(mate->second))), HighResPositionData(pos_rights_),
                                                       key)));
        } else {
            lrMods_.insert(make_pair(it->first,
                                      make_pair(0.5 * (list_.at(it->second)->GetWalkCorrectedTime() +
                                                        list_.at(mate->second)->GetWalkCorrectedTime()),
                                                sqrt(list_.at(it->second)->GetCalibratedEnergy() *
                                                             list_.at(mate->second)->GetCalibratedEnergy()))));
        }
        pos_lefts_.clear();
        pos_rights_.clear();
    }
}

unsigned int NEXTBuilder::CalcModNumber(const unsigned int &loc) {
    return loc / 2;
}

void NEXTBuilder::ClearMaps(void) {
    lrMods_.clear();
    hrMods_.clear();
    lefts_.clear();
    rights_.clear();
    pos_lefts_.clear();
    pos_rights_.clear();
}

void NEXTBuilder::FillMaps(void) {
    for (vector<ChanEvent *>::const_iterator it = list_.begin(); it != list_.end(); it++) {
        ChannelConfiguration id = (*it)->GetChanID();
        unsigned int modNum = CalcModNumber(id.GetLocation());
        unsigned int idx = (unsigned int) (it - list_.begin());
        if (id.HasTag("left") || id.HasTag("up") || id.HasTag("top"))
            lefts_.insert(make_pair(modNum, idx));
        if (id.HasTag("right") || id.HasTag("down") || id.HasTag("bottom"))
            rights_.insert(make_pair(modNum, idx));
    }
}
