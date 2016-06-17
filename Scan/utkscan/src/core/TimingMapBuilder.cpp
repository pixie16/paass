/*! \file TimingMapBuilder.cpp
 *  \brief A Class to build TimingMaps
 *  \author S. V. Paulauskas
 *  \date December 16, 2014
*/
#include <iostream>
#include <vector>

#include "TimingMapBuilder.hpp"

using namespace std;

TimingMapBuilder::TimingMapBuilder(const std::vector<ChanEvent*> &evts) {
    FillMaps(evts);
}

void TimingMapBuilder::FillMaps(const std::vector<ChanEvent*> &evts) {
    map_.clear();
    for(vector<ChanEvent*>::const_iterator it = evts.begin();
    it != evts.end(); it++) {
        Identifier id = (*it)->GetChanID();
        TimingDefs::TimingIdentifier key(id.GetLocation(), id.GetSubtype());

        HighResTimingData data((*it));
        if(!data.GetIsValid())
            continue;
        map_.insert(make_pair(key,data));
    }
}
