/** \file ScintROOT.cpp
 *\brief Outputs Scint info to ROOT - NOT UNDER DEVELOPMENT!!
 *
 *Writes certain information from the ScintProcessor into a ROOT tree.
 *This functionality is currently not supported, and may well give garbage.
 *
 *\author S. V. Paulauskas  
 *\date 10 May 2011
 */
#include <string>

#include "ScintROOT.hpp"

#include <TTree.h>

using std::string;

void ScintROOT::FillRoot(const TimingDataMap &detMap)
{
    for(TimingDataMap::const_iterator itTemp = detMap.begin(); 
	itTemp != detMap.end(); itTemp++) {
	beta = liquid = DataRoot();
	
	DataRoot *data;

	if( (*itTemp).first.second == "beta") {
	    data = &beta;
	    const TimingData &tempData = (*itTemp).second;
	    
	    data->location[data->multiplicity] = (*itTemp).first.first;
	    data->maxval[data->multiplicity] = tempData.maxval;
	    data->tqdc[data->multiplicity] = tempData.tqdc;
	    data->aveBaseline[data->multiplicity] = tempData.aveBaseline;
	    data->highResTime[data->multiplicity] = tempData.highResTime;
	    data->maxpos[data->multiplicity] = tempData.maxpos;
	    data->phase[data->multiplicity] = tempData.phase;
	    data->stdDevBaseline[data->multiplicity] = tempData.stdDevBaseline;
	    data->multiplicity++;
	} else if( (*itTemp).first.second == "liquid") {
	    data = &liquid;
	    
	    const TimingData &tempData = (*itTemp).second;
	    
	    data->location[data->multiplicity] = (*itTemp).first.first;
	    data->maxval[data->multiplicity] = tempData.maxval;
	    data->tqdc[data->multiplicity] = tempData.tqdc;
	    data->aveBaseline[data->multiplicity] = tempData.aveBaseline;
	    data->highResTime[data->multiplicity] = tempData.highResTime;
	    data->maxpos[data->multiplicity] = tempData.maxpos;
	    data->phase[data->multiplicity] = tempData.phase;
	    data->stdDevBaseline[data->multiplicity] = tempData.stdDevBaseline;
	    data->discrimination[data->multiplicity] = tempData.discrimination;
	    data->multiplicity++;
	}
	else
	    beta = liquid = DataRoot();
    }// end for(DataMap::
}

bool ScintROOT::AddBranch(TTree *tree)
{
    if (tree) {
	string branchDef = "multiplicity/i:dummy/i:aveBaseline[10]/D:discrimination[10]/D:highResTime[10]/D:maxpos[10]/D:maxval[10]/D:phase[10]/D:stdDevBaseline[10]/D:tqdc[10]/D:location[10]/i";
	TBranch *scintBranch = 
	    tree->Branch("Beta", &beta, branchDef.c_str());
	
	TBranch *scintBranch1 = 
	    tree->Branch("Liquid", &liquid, branchDef.c_str());
	
	return (scintBranch != NULL);
    } 
    return false;
}

void ScintROOT::FillBranch(void)
{
    FillRoot(liquidMap);
    FillRoot(betaMap);

    if (!HasEvent())
	beta = liquid = DataRoot();
}
