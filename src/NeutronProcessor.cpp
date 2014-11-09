/** \file NeutronProcessor.cpp
 *
 * implementation for scintillator processor
 */
#include <vector>
#include <sstream>

#include <cmath>

#include "DammPlotIds.hpp"
#include "RawEvent.hpp"
#include "NeutronProcessor.hpp"

using namespace std;
using namespace dammIds::scint::neutron;

namespace dammIds {
    namespace scint {
        namespace neutron {
            namespace betaGated {
                const int D_ENERGY_DETX = 0; //!< Beta gated hists for 3 detectors (1-3)
            }
            namespace gammaGated {
                const int D_ENERGY_DETX = 3; //!< gamma gated hists for 3 detectors (1-3)
            }
            namespace betaGammaGated {
                const int D_ENERGY_DETX = 6; //!< beta-gamma gated hists for 3 detectors (1-3)
            }
        }
    }
}

NeutronProcessor::NeutronProcessor() : EventProcessor(OFFSET, RANGE)
{
    name = "Neutron";
    associatedTypes.insert("scint");
}

void NeutronProcessor::DeclarePlots(void)
{
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

bool NeutronProcessor::PreProcess(RawEvent &event){
    if (!EventProcessor::PreProcess(event))
        return false;
    return true;
}

bool NeutronProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
        return false;

    static const vector<ChanEvent*> &scintNeutrEvents =
	event.GetSummary("scint:neutr")->GetList();

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
