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

}

bool ScintProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
	return false;
    
    static const vector<ChanEvent*> &scintNeutrEvents = 
	event.GetSummary("scint:neutr")->GetList();

    int betaMult = event.GetSummary("scint:beta")->GetMult();
    int gammaMult = event.GetSummary("ge:clover_high")->GetMult();

    for (vector<ChanEvent*>::const_iterator it = scintNeutrEvents.begin();
	 it != scintNeutrEvents.end(); it++) {
	ChanEvent *chan = *it;

	int loc        = chan->GetChanID().GetLocation();
	
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
    } // end loop over scint types
	
    EndProcess(); // update the processing time
    return true;
}
