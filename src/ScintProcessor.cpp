/** \file ScintProcessor.cpp
 *
 * implementation for scintillator processor
 */

#include <vector>

#include <cmath>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "RawEvent.hpp"
#include "ScintProcessor.hpp"
#include "Trace.hpp"

using namespace std;
using namespace dammIds::scint;

namespace dammIds {
    namespace scint {
        namespace neutr {
            namespace betaGated {
                const int D_ENERGY_DETX = 0; // for 3 detectors (1-3)
            }
            namespace gammaGated {
                const int D_ENERGY_DETX = 3; // for 3 detectors (1-3)
            }
            namespace betaGammaGated {
                const int D_ENERGY_DETX = 6; // for 3 detectors (1-3)
            }
        } // neutr namespace
        const int DD_TQDCBETA         = 9;
        const int DD_MAXBETA          = 10;
        const int DD_TQDCLIQUID       = 11;
        const int DD_MAXLIQUID        = 12;
        const int D_DISCRIM           = 13;
        const int DD_NGVSE            = 14;
        const int DD_TOFLIQUID        = 15;
        const int DD_TRCLIQUID        = 16;
    }
} 

ScintProcessor::ScintProcessor() : EventProcessor(OFFSET, RANGE)
{
    name = "scint";
    //? change to associated type "scint:neutr"
    associatedTypes.insert("scint"); // associate with the scint type
}

void ScintProcessor::DeclarePlots(void)
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
    histo.DeclareHistogram2D(DD_TQDCBETA, SC, S3, "Start vs. Trace QDC", 1, SE, 0, SE-1, S3, 0, S3-1);
    DeclareHistogram2D(DD_MAXBETA, SC, S3, "Start vs. Maximum");

    //To handle Liquid Scintillators
    histo.DeclareHistogram2D(DD_TQDCLIQUID, SC, S3, "Liquid vs. Trace QDC", 1, SE, 0, SE-1, S3, 0, S3-1);
    DeclareHistogram2D(DD_MAXLIQUID, SC, S3, "Liquid vs. Maximum");
    DeclareHistogram1D(D_DISCRIM, SD, "N-Gamma Discrimination");
    // DeclareHistogram2D(DD_NGVSE, SE, SE,"N-G Discrim vs Energy");
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
    int gammaMult = event.GetSummary("ge:clover_hi.hpp")->GetMult();
    
    if (betaMult > 0) {
	// here we correlate the decay for the plastic beta scintillators
	//   for the LeRIBSS setup, the corresponding implant is the stop of the tape movement
	EventInfo corEvent;
	corEvent.time = scintBetaEvents[0]->GetTime();
	corEvent.type = EventInfo::DECAY_EVENT;

	event.GetCorrelator().Correlate(corEvent, 1, 1);
    }
    for(vector<ChanEvent*>::const_iterator itBeta = scintBetaEvents.begin();
	itBeta != scintBetaEvents.end(); itBeta++) {
	
	unsigned int loc = (*itBeta)->GetChanID().GetLocation();
	ScintData data(*itBeta);

	betaMap.insert(make_pair(loc, data));
	
	if ( data.GoodDataCheck() ) {
	    plot(DD_TQDCBETA, data.tqdc, loc);
	    plot(DD_MAXBETA, data.maxval, loc);
	}
    }
    
    for(vector<ChanEvent*>::const_iterator itLiquid = scintLiquidEvents.begin();
	itLiquid != scintLiquidEvents.end(); itLiquid++) {
	
	unsigned int loc = (*itLiquid)->GetChanID().GetLocation();
	
	ScintData data(*itLiquid);
	liquidMap.insert(make_pair(loc, data));
	
	if ( data.GoodDataCheck() ) {
	    plot(DD_TQDCLIQUID, data.tqdc, loc);
	    plot(DD_MAXLIQUID, data.maxval, loc);
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
	unsigned int maxX = (unsigned int)(*itLiquid).second.maxpos;
	unsigned int lowerLimit = 5;
	unsigned int upperLimit = 12;
	
	const ScintData &data = (*itLiquid).second;

	for(Trace::const_iterator i = data.trace.begin(); i != data.trace.end(); i++)
	    plot(DD_TRCLIQUID, int(i-data.trace.begin()), counter, *i);
	counter++;
	
	if( data.GoodDataCheck() ) {
	    for(unsigned int j = maxX+lowerLimit; j < maxX+upperLimit && j < data.trace.size(); j++)
		discrim += data.trace.at(j)-data.aveBaseline;   
	    
	    double discrim_norm = discrim / data.tqdc;	    
	    plot(D_DISCRIM,int(discrim_norm*100)+1000,1);
	    plot(DD_NGVSE, int(discrim), int(data.tqdc));
	    
	    for(map<int, ScintData>::iterator itStart = betaMap.begin(); itStart != betaMap.end(); itStart++) { // operations w.r.t triggers 
		const ScintData &betaData = (*itStart).second;
		int liquidPlusStartLoc = (*itLiquid).first + (*itStart).first;
		const int resMult = 20;
		const int resOffSet = 2000;
		
		double TOF = data.highResTime - betaData.highResTime; //in seconds
		
		if(betaData.GoodDataCheck())
		    plot(DD_TOFLIQUID, TOF*resMult+resOffSet, liquidPlusStartLoc);		
	    } //Loop over Beta Starts
	} // Good Liquid Check
    } //for(map<int, ScintData>::iterator itStart
    
    EndProcess(); // update the processing time
    return true;
}

bool ScintProcessor::ScintData::GoodDataCheck(void) const 
{
    return (maxval != emptyValue && phase != emptyValue &&
	    tqdc != emptyValue && highResTime != emptyValue);
}

ScintProcessor::ScintData::ScintData(string type) : trace(emptyTrace)
{
    detSubtype     = type;
    maxval         = emptyValue;
    maxpos         = emptyValue;
    phase          = emptyValue;
    tqdc           = emptyValue;
    stdDevBaseline = emptyValue;
    aveBaseline    = emptyValue;
    highResTime    = emptyValue;
}

ScintProcessor::ScintData::ScintData(ChanEvent* chan) : trace(chan->GetTrace())
{
    detSubtype     = chan->GetChanID().GetSubtype();
    highResTime    = chan->GetHighResTime();
    tqdc           = trace.GetValue("tqdc");
    maxval         = trace.GetValue("maxval");
    maxpos         = trace.GetValue("maxpos");
    phase          = trace.GetValue("phase");
    stdDevBaseline = trace.GetValue("baseline");
    aveBaseline    = trace.GetValue("sigmaBaseline");
}
