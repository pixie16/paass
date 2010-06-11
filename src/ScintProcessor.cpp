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

using namespace std;
using namespace dammIds::scint;

ScintProcessor::ScintProcessor() : EventProcessor()
{
    name = "scint";
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

}

bool ScintProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
	return false;
    
    // flag for the presence of a beta decay electron
    int betaMult  = 0;
    int gammaMult = 0;
    
    static const vector<ChanEvent*> &scintEvents = sumMap["scint"]->GetList();

    //loop of scint and identify signals belonging to beta detectors  
    for (vector<ChanEvent*>::const_iterator it = scintEvents.begin();
	 it != scintEvents.end(); it++) {
	if ( (*it)->GetChanID().GetSubtype() == "beta" ) {
	    betaMult++;
	}
    }

    // this summary isn't guaranteed to exist here, so first check its validity
    static const DetectorSummary *geSummary = event.GetSummary("ge");

    if (geSummary) {
	// loop over ge signals and identify those with signals in high gain
	for (vector<ChanEvent*>::const_iterator it = geSummary->GetList().begin();
	     it != geSummary->GetList().end(); it++) {
	    if ( (*it)->GetChanID().GetSubtype() == "clover_high") {
		// For some experiments two outputs per clover were used, one was set
		// to high gain on pixie16 and one was set to low gain on pixie16. 
		// Only perform analysis for high gain clover signals. 
		gammaMult++;
	    }
	}
    }

    for (vector<ChanEvent*>::const_iterator it = scintEvents.begin();
	 it != scintEvents.end(); it++) {
	ChanEvent *chan = *it;

	string subtype = chan->GetChanID().GetSubtype();
	int loc        = chan->GetChanID().GetLocation();
	
	if(subtype == "neutr") {
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
	} // if neutr 
    } // end loop over scint types
	
    EndProcess(); // update the processing time
    return true;
}
