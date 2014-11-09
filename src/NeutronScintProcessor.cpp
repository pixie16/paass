/** \file NeutronScintProcessor.cpp
 *
 * implementation for scintillator processor
 */
#include <vector>
#include <sstream>

#include <cmath>

#include "DammPlotIds.hpp"
#include "RawEvent.hpp"
#include "NeutronScintProcessor.hpp"

using namespace std;
using namespace dammIds::neutron_scint;

namespace dammIds {
    namespace neutron_scint {
        namespace betaGated {
            const int D_ENERGY_DETX = 0; //!< Beta gated hists for 3 detectors (1-3)
        }
        namespace gammaGated {
            const int D_ENERGY_DETX = 3; //!< gamma gated hists  for 3 detectors (1-3)
        }
        namespace betaGammaGated {
            const int D_ENERGY_DETX = 6; //!< beta-gamma gated hists for 3 detectors (1-3)
        }
    }
}

NeutronScintProcessor::NeutronScintProcessor() :
    EventProcessor(OFFSET, RANGE, "neutron_scint") {
    associatedTypes.insert("neutron_scint");
}

void NeutronScintProcessor::DeclarePlots(void) {
	DeclareHistogram1D(betaGated::D_ENERGY_DETX + 0, SE, "beta gated scint1 sig1");
	DeclareHistogram1D(betaGated::D_ENERGY_DETX + 1, SE, "beta gated scint1 sig2");
	DeclareHistogram1D(betaGated::D_ENERGY_DETX + 2, SE, "beta gated 3Hen");

	DeclareHistogram1D(gammaGated::D_ENERGY_DETX + 0, SE, "gamma gated scint1 sig1");
	DeclareHistogram1D(gammaGated::D_ENERGY_DETX + 1, SE, "gamma gated scint1 sig2");
	DeclareHistogram1D(gammaGated::D_ENERGY_DETX + 2, SE, "gamma gated 3Hen");

	DeclareHistogram1D(betaGammaGated::D_ENERGY_DETX + 0, SE, "beta-gamma gated scint1 sig1");
	DeclareHistogram1D(betaGammaGated::D_ENERGY_DETX + 1, SE, "beta-gamma gated scint1 sig2");
	DeclareHistogram1D(betaGammaGated::D_ENERGY_DETX + 2, SE, "beta-gamma gated 3Hen");
}

bool NeutronScintProcessor::PreProcess(RawEvent &event){
    if (!EventProcessor::PreProcess(event))
        return false;
    return true;
}

bool NeutronScintProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
        return false;

    static const vector<ChanEvent*> &scintNeutrEvents =
	event.GetSummary("neutron_scint:neutr")->GetList();

    for (vector<ChanEvent*>::const_iterator it = scintNeutrEvents.begin();
	 it != scintNeutrEvents.end(); it++) {

        ChanEvent *chan = *it;
        int loc = chan->GetChanID().GetLocation();
        double neutronEnergy = chan->GetCalEnergy();

        if (TreeCorrelator::get()->place("Beta")->status()) {
            plot(betaGated::D_ENERGY_DETX + loc, neutronEnergy);
        }
        if (TreeCorrelator::get()->place("Gamma")->status()) {
            plot(gammaGated::D_ENERGY_DETX + loc, neutronEnergy);
        }
        if (TreeCorrelator::get()->place("GammaBeta")->status()) {
            plot(betaGammaGated::D_ENERGY_DETX + loc, neutronEnergy);
        }
    }
    EndProcess();
    return true;
}
