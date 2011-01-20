/** \file ScintProcessor.cpp
 *
 * implementation for scintillator processor
 */

#include <vector>

#include <cmath>

#include "damm_plotids.h"
#include "DetectorDriver.h"
#include "RawEvent.h"
#include "ScintProcessor.h"
#include "Trace.h"

using namespace std;
using namespace dammIds::scint;

ScintProcessor::ScintProcessor() : EventProcessor()
{
    name = "scint";
    //? change to associated type "scint:neutr"
    associatedTypes.insert("scint"); // associate with the scint type
}

void ScintProcessor::DeclarePlots(void) const
{
    {
	using namespace neutr::betaGated;
	DeclareHistogram1D(D_ENERGY_DETX + 0, SE, "beta gated scint1 sig1");
	DeclareHistogram1D(D_ENERGY_DETX + 1, SE, "beta gated scint1 sig2");
	DeclareHistogram1D(D_ENERGY_DETX + 2, SE, "beta gated 3Hen");
    }
    {
	using namespace neutr::gammaGated;
	DeclareHistogram1D(D_ENERGY_DETX + 0, SE, "gamma gated scint1 sig1");
	DeclareHistogram1D(D_ENERGY_DETX + 1, SE, "gamma gated scint1 sig2");
	DeclareHistogram1D(D_ENERGY_DETX + 2, SE, "gamma gated 3Hen");
    }
    
    {
	using namespace neutr::betaGammaGated;
	DeclareHistogram1D(D_ENERGY_DETX + 0, SE, "beta-gamma gated scint1 sig1");
	DeclareHistogram1D(D_ENERGY_DETX + 1, SE, "beta-gamma gated scint1 sig2");
	DeclareHistogram1D(D_ENERGY_DETX + 2, SE, "beta-gamma gated 3Hen");
    }

//for the beta detectors from LeRIBSS
    DeclareHistogram2D(DD_TQDCBETA, SC, S3, "Start vs. Trace QDC", 1, SE, 0, SE-1, S3, 0, S3-1);
    DeclareHistogram2D(DD_MAXBETA, SC, S3, "Start vs. Maximum");

    //To handle Liquid Scintillators
    DeclareHistogram2D(DD_TQDCLIQUID, SC, S3, "Liquid vs. Trace QDC", 1, SE, 0, SE-1, S3, 0, S3-1);
    DeclareHistogram2D(DD_MAXLIQUID, SC, S3, "Liquid vs. Maximum");
    DeclareHistogram1D(D_DISCRIM, SD, "N-Gamma Discrimination");
    DeclareHistogram2D(DD_NGVSE, SE, SE,"N-G Discrim vs Energy");
    DeclareHistogram2D(DD_TOFLIQUID, SE, S3,"N-G Discrim vs Energy");
    DeclareHistogram2D(DD_TRCLIQUID, S7, S7, "LIQUID TRACES");
}

bool ScintProcessor::Process(RawEvent &event)
{
    liquidMap.clear();
    betaMap.clear();
    
    if (!EventProcessor::Process(event))
	return false;
    
    static const vector<ChanEvent*> &scintNeutrEvents = 
	event.GetSummary("scint:neutr")->GetList();
    
    static const vector<ChanEvent*> &scintBetaEvents = 
	event.GetSummary("scint:beta")->GetList();
    
    static const vector<ChanEvent*> &scintLiquidEvents = 
	event.GetSummary("scint:liquid")->GetList();
    
    int betaMult = event.GetSummary("scint:beta")->GetMult();
    int gammaMult = event.GetSummary("ge:clover_high")->GetMult();
    
    for(vector<ChanEvent*>::const_iterator itBeta = scintBetaEvents.begin();
	itBeta != scintBetaEvents.end(); itBeta++) {
	
	unsigned int loc = (*itBeta)->GetChanID().GetLocation();

	map<int, struct ScintData>::iterator itTemp = 
	    betaMap.insert(make_pair(loc, ScintData((*itBeta)))).first;
	
	if (GoodDataCheck((*itTemp).second)) {
	    plot(DD_TQDCBETA, (*itTemp).second.tqdc, loc);
	    plot(DD_MAXBETA, (*itTemp).second.maxval, loc);
	}
    }
    
    for(vector<ChanEvent*>::const_iterator itLiquid = scintLiquidEvents.begin();
	itLiquid != scintBetaEvents.end(); itLiquid++) {
	
	unsigned int loc = (*itLiquid)->GetChanID().GetLocation();
	
	map<int, struct ScintData>::iterator itTemp = 
	    liquidMap.insert(make_pair(loc, ScintData((*itLiquid)))).first;
	
	if (GoodDataCheck((*itTemp).second)) {
	    plot(DD_TQDCLIQUID, (*itTemp).second.tqdc, loc);
	    plot(DD_MAXLIQUID, (*itTemp).second.maxval, loc);
	}	     
    }
    
    for (vector<ChanEvent*>::const_iterator it = scintNeutrEvents.begin();
	 it != scintNeutrEvents.end(); it++) {
	ChanEvent *chan = *it;
	
	int loc = chan->GetChanID().GetLocation();
	
	using namespace neutr;
	
	double neutronEnergy = chan->GetCalEnergy();
	
	//plot neutron spectrum gated by beta
	if (betaMult > 0) { 
	    plot(betaGated::D_ENERGY_DETX + loc, neutronEnergy);
	}
	//plot neutron spectrum gated by gamma
	if (gammaMult > 0) {
	    plot(gammaGated::D_ENERGY_DETX + loc, neutronEnergy);
	    if (betaMult > 0) {
		plot(betaGammaGated::D_ENERGY_DETX + loc, neutronEnergy);
	    }
	}
    } // end loop over scint neutr types
    
    
    for(map<int, ScintData>::iterator itLiquid = liquidMap.begin(); itLiquid != liquidMap.end(); itLiquid++) {
	/****N/Gamma discrimination ****/
	double discrim = 0; 
	int maxX = (*itLiquid).second.maxpos;
	int lowerLimit = 5;
	int upperLimit = 12;
	
	for(vector<int>::iterator i = (*itLiquid).second.trace.begin(); i != (*itLiquid).second.trace.end(); i++)
	    plot(DD_TRCLIQUID, int(i-(*itLiquid).second.trace.begin()), counter, *i);
	counter ++;
	
	if( GoodDataCheck((*itLiquid).second))
	{
	    for(unsigned int j = maxX+lowerLimit; (j < maxX+upperLimit) && (j < (*itLiquid).second.trace.size()); j++)  
		discrim += ((*itLiquid).second.trace.at(j)-(*itLiquid).second.aveBaseline);		    
	    
	    double discrim_norm = discrim/(*itLiquid).second.tqdc;	    
	    plot(D_DISCRIM,int(discrim_norm*100)+1000,1);
	    plot(DD_NGVSE, int(discrim), int((*itLiquid).second.tqdc));
	    
	    for(map<int, ScintData>::iterator itStart = betaMap.begin(); itStart != betaMap.end(); itStart++) { // operations w.r.t triggers 
		int liquidPlusStartLoc = (*itLiquid).first + (*itStart).first;
		const int resMult = 20;
		const int resOffSet = 2000;
		
		double TOF = (*itLiquid).second.highResTime - (*itStart).second.highResTime; //in 10ns
		
		if(GoodDataCheck((*itStart).second))
		    plot(DD_TOFLIQUID, TOF*resMult+resOffSet, liquidPlusStartLoc);
		
	    } //Loop over Beta Starts
	} // Good Liquid Check
    } //for(map<int, ScintData>::iterator itStart
    
    EndProcess(); // update the processing time
    return true;
}

bool ScintProcessor::GoodDataCheck(const ScintData& DataCheck) {
    if((DataCheck.maxval != -9999) && (DataCheck.phase !=-9999) && (DataCheck.tqdc !=-9999) && (DataCheck.highResTime != -9999))
	return(true);
    else
	return(false);
}

ScintProcessor::ScintData::ScintData(string type) {
    detSubtype     = type;
    maxval         = -9999;
    maxpos         = -9999;
    phase          = -9999;
    tqdc           = -9999;
    stdDevBaseline = -9999;
    aveBaseline    = -9999;
    highResTime    = -9999;
}

ScintProcessor::ScintData::ScintData(ChanEvent* chan) {
    detSubtype     = chan->GetChanID().GetSubtype();
    highResTime    = chan->GetHighResTime();
    tqdc           = trace.GetValue("tqdc");
    maxval         = trace.GetValue("maxval");
    maxpos         = trace.GetValue("maxpos");
    phase          = trace.GetValue("phase");
    stdDevBaseline = trace.GetValue("baseline");
    aveBaseline    = trace.GetValue("sigmaBaseline");
}
