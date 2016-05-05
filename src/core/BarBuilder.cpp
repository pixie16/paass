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

void BarBuilder::BuildBars(void) {
    ClearMaps();
    FillMaps();

    for(map<unsigned int, unsigned int>::const_iterator it = lefts_.begin();
        it != lefts_.end(); it++) {
	map<unsigned int, unsigned int>::const_iterator mate = rights_.find(it->first);
	if(mate == rights_.end())
	    continue;

	if(list_.at(it->second)->GetTrace().size() != 0 &&
	   list_.at(mate->second)->GetTrace().size() != 0) {
	    TimingDefs::TimingIdentifier key =
	     	make_pair(it->first, list_.at(it->second)->GetChanID().GetSubtype());
	    hrtBars_.insert(make_pair(key,
            BarDetector(HighResTimingData(list_.at(it->second)),
                        HighResTimingData(list_.at(mate->second)), key)));
	} else {
	    lrtBars_.insert(make_pair(it->first,
				      make_pair(0.5*(list_.at(it->second)->GetCorrectedTime()+
						     list_.at(mate->second)->GetCorrectedTime()),
						sqrt(list_.at(it->second)->GetCalEnergy()*
						     list_.at(mate->second)->GetCalEnergy()))));
	}
    }
}

unsigned int BarBuilder::CalcBarNumber(const unsigned int &loc) {
    return(loc/2);
}

void BarBuilder::ClearMaps(void){
    lrtBars_.clear();
    hrtBars_.clear();
    lefts_.clear();
    rights_.clear();
}

void BarBuilder::FillMaps(void) {
    for(vector<ChanEvent*>::const_iterator it = list_.begin();
    it != list_.end(); it++) {
	Identifier id = (*it)->GetChanID();
	unsigned int barNum = CalcBarNumber(id.GetLocation());
	unsigned int idx = (unsigned int)(it - list_.begin());
	if(id.HasTag("left") || id.HasTag("up") || id.HasTag("top"))
	    lefts_.insert(make_pair(barNum,idx));
	if(id.HasTag("right") || id.HasTag("down") || id.HasTag("bottom"))
	    rights_.insert(make_pair(barNum,idx));
    }
}
