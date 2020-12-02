/*! \file NEXTBuilder.cpp
 *  \brief A class that builds bar type detectors
 *  \author S. V. Paulauskas
 *  \date December 15, 2014
*/
#include <iostream>
#include <vector>

#include "NEXTBuilder.hpp"
#include "NEXTDetector.hpp"
#include "TimingMapBuilder.hpp"

using namespace std;

void NEXTBuilder::BuildModules(void) {
    ClearMaps();
    FillMaps();
    double intg;
    double l_ft, l_fb, l_bt, l_bb;
    double r_ft, r_fb, r_bt, r_bb; 
 

    bool kLeft = true;
    bool kRight = true;
    
    HighResPositionData pLeft, pRight;    

    for (map<unsigned int, unsigned int>::const_iterator it = lefts_.begin(); it != lefts_.end(); it++) {
        map<unsigned int, unsigned int>::const_iterator mate = rights_.find(it->first);
        if (mate == rights_.end())
            continue;
        pLeft.ClearEvent(); pRight.ClearEvent();
        intg = 0.0;
        l_ft = 0; l_fb = 0; l_bt = 0; l_bb = 0;
        r_ft = 0; r_fb = 0; r_bt = 0; r_bb = 0; 

        for (vector<ChanEvent *>::const_iterator pt = pos_.begin(); pt != pos_.end(); pt++){
            ChannelConfiguration ia = (*pt)->GetChanID();
            std::string modID = ia.GetGroup();
            if (std::to_string(it->first) ==  modID ) {
            intg = (*pt)->GetEnergy();           
           
                if ((*pt)->GetChanID().HasTag("left")) {

                    pLeft.AddChanEvent(*pt);
                    if((*pt)->GetChanID().HasTag("FT") && l_ft == 0) l_ft = intg;
                    if((*pt)->GetChanID().HasTag("FB") && l_fb == 0) l_fb = intg;
                    if((*pt)->GetChanID().HasTag("BB") && l_bb == 0) l_bb = intg;
                    if((*pt)->GetChanID().HasTag("BT") && l_bt == 0) l_bt = intg; 
                }
                if ((*pt)->GetChanID().HasTag("right")) {

                    pRight.AddChanEvent(*pt);
                    
                    if((*pt)->GetChanID().HasTag("FT") && r_ft == 0) r_ft = intg;
                    if((*pt)->GetChanID().HasTag("FB") && r_fb == 0) r_fb = intg;
                    if((*pt)->GetChanID().HasTag("BB") && r_bb == 0) r_bb = intg;
                    if((*pt)->GetChanID().HasTag("BT") && r_bt == 0) r_bt = intg; 
                }
                intg = 0.0;
            }
        }
        
         if (l_ft>0 && l_fb>0 && l_bb>0 && l_bt>0)
            kLeft = true;
        else 
            kLeft = false;
        
        if (r_ft>0 && r_fb>0 && r_bb>0 && r_bt>0)
            kRight = true;
        else 
            kRight = false; 

        if (list_.at(it->second)->GetTrace().size() != 0 && list_.at(mate->second)->GetTrace().size() != 0
             && kLeft && kRight ) {
                 
            TimingDefs::TimingIdentifier key =
                    make_pair(it->first, list_.at(it->second)->GetChanID().GetSubtype());
            hrMods_.insert(make_pair(key, NEXTDetector(HighResTimingData(*(list_.at(it->second))), pLeft,
                                                       HighResTimingData(*(list_.at(mate->second))), pRight,
                                                       key)));
        } else {
            lrMods_.insert(make_pair(it->first,
                                      make_pair(0.5 * (list_.at(it->second)->GetWalkCorrectedTime() +
                                                        list_.at(mate->second)->GetWalkCorrectedTime()),
                                                sqrt(list_.at(it->second)->GetCalibratedEnergy() *
                                                             list_.at(mate->second)->GetCalibratedEnergy()))));
        }
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
}

void NEXTBuilder::FillMaps(void) {
    for (vector<ChanEvent *>::const_iterator it = list_.begin(); it != list_.end(); it++) {
        ChannelConfiguration id = (*it)->GetChanID();
        //unsigned int modNum = CalcModNumber(id.GetLocation());
        unsigned int modNum = std::atoi(id.GetGroup().c_str());
        unsigned int idx = (unsigned int) (it - list_.begin());
        if (id.HasTag("left") || id.HasTag("up") || id.HasTag("top"))
            lefts_.insert(make_pair(modNum, idx));
        if (id.HasTag("right") || id.HasTag("down") || id.HasTag("bottom"))
            rights_.insert(make_pair(modNum, idx));
    }
}
