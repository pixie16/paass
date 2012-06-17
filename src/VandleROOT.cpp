/**************************************
Contains the information for the ROOT
functionality in the VANDLE Processor

Written: S. V. Paulauskas 09 May 2011

**************************************/
#include <string>

#include "VandleRoot.hpp"

#include <TTree.h>

using std::string;

//********** AddBranch **********
bool VandleROOT::AddBranch(TTree *tree)
{
    if (tree) {
	string branchDef = "multiplicity/i:dummy/i:aveBaseline[10]/D:discrimination[10]/D:highResTime[10]/D:maxpos[10]/D:maxval[10]/D:phase[10]/D:stdDevBaseline[10]/D:tqdc[10]/D:location[10]/i";
	
	TBranch *vandleBranch  = tree->Branch("VandleSmallRight", &smallRight, branchDef.c_str()); 
	TBranch *vandleBranch1 = tree->Branch("VandleSmallLeft",  &smallLeft, branchDef.c_str());
	TBranch *vandleBranch2  = tree->Branch("VandleBigRight", &bigRight, branchDef.c_str()); 
	TBranch *vandleBranch3 = tree->Branch("VandleBigLeft",  &bigLeft, branchDef.c_str());
	
	return ( vandleBranch != NULL );
    } 
    return false;
}


//********** FillBranch **********
void VandleROOT::FillBranch(void)
{
    FillRoot(smallMap, "small");
    FillRoot(bigMap, "big");
    
    if (!HasEvent())
	smallRight = smallLeft = bigRight = bigLeft = DataRoot();
}


//********** FillRoot **********
void VandleROOT::FillRoot(const TimingDataMap &endMap, const string &barType)
{
    smallRight = smallLeft = bigRight = bigLeft = DataRoot();
    DataRoot *data;
    
    for(TimingDataMap::const_iterator itTemp = endMap.begin(); 
	itTemp != endMap.end(); itTemp++) {
	
	if(barType == "big") {
	    if( (*itTemp).first.second == "right") {
		data = &bigRight;
	    } else if ((*itTemp).first.second == "left") {
		data = &bigLeft;
	    } else continue;
	} else { 
	    if( (*itTemp).first.second == "right") {
		data = &smallRight;
	    } else if ( (*itTemp).first.second == "left") {
		data = &smallLeft;
	    } else continue;
	}
	
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
	
    } // end for(DataMap::
} 
