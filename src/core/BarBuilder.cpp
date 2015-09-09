/*! \file BarBuilder.cpp
 *  \brief A class that builds bar type detectors
 *  \author S. V. Paulauskas
 *  \date December 15, 2014
*/
#include <iostream>
#include <vector>

#include "BarBuilder.hpp"
#include "TimingMapBuilder.hpp"

using namespace std;

BarBuilder::BarBuilder(const std::vector<ChanEvent*> &vec) {
    list_ = vec;
    BuildBars();
}

void BarBuilder::BuildBars(void) {
    ClearMaps();
    FillMaps();

    for(TimingMap::const_iterator it = lefts_.begin(); it != lefts_.end(); it++) {
        TimingDefs::TimingIdentifier key = (*it).first;
        TimingMap::const_iterator mate = rights_.find(key);

        if(mate == rights_.end())
            continue;

        TimingCalibration cal = TimingCalibrator::get()->GetCalibration(key);

        bars_.insert(make_pair(key, BarDetector((*it).second,
                        (*mate).second, cal, key.second)));
    }
}

unsigned int BarBuilder::CalcBarNumber(const unsigned int &loc) {
    return(loc/2);
}

void BarBuilder::ClearMaps(void){
    bars_.clear();
    lefts_.clear();
    rights_.clear();
}

void BarBuilder::FillMaps(void) {
    for(vector<ChanEvent*>::const_iterator it = list_.begin();
    it != list_.end(); it++) {
        Identifier id = (*it)->GetChanID();
        TimingDefs::TimingIdentifier key(CalcBarNumber(id.GetLocation()),
                                         id.GetSubtype());

        HighResTimingData data((*it));

        if(!data.GetIsValidData())
            continue;

        if(id.HasTag("left") || id.HasTag("up") || id.HasTag("top"))
            lefts_.insert(make_pair(key,data));
        if(id.HasTag("right") || id.HasTag("down") || id.HasTag("bottom"))
            rights_.insert(make_pair(key,data));
    }
}
