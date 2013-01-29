/** \file BetaBetaScintProcessor.cpp
 *
 * implementation for beta scintillator processor
 */
#include <vector>
#include <sstream>

#include "DammPlotIds.hpp"
#include "RawEvent.hpp"
#include "ChanEvent.hpp"
#include "BetaScintProcessor.hpp"

using namespace std;
using namespace dammIds::beta_scint;

namespace dammIds {
    namespace beta_scint {
        const int D_MULT_BETA = 0;
        const int D_ENERGY_BETA = 1;
        const int D_MULT_BETA_GATED = 2;
        const int D_ENERGY_BETA_GATED = 3;
    }
} 

BetaScintProcessor::BetaScintProcessor() : 
    EventProcessor(OFFSET, RANGE, "beta_scint") {
    associatedTypes.insert("beta_scint"); 
}

void BetaScintProcessor::DeclarePlots(void) {
    DeclareHistogram1D(D_MULT_BETA, S4, "Beta multiplicity");
    DeclareHistogram1D(D_ENERGY_BETA, SE, "Beta energy");

    DeclareHistogram1D(D_MULT_BETA_GATED, S4, "Beta multiplicity gated");
    DeclareHistogram1D(D_ENERGY_BETA_GATED, SE, "Beta energy gated");
}

bool BetaScintProcessor::PreProcess(RawEvent &event){
    if (!EventProcessor::PreProcess(event))
        return false;

    static const vector<ChanEvent*> &scintBetaEvents = 
	event.GetSummary("beta_scint:beta", true)->GetList();

    int multiplicity = 0;
    for (vector<ChanEvent*>::const_iterator it = scintBetaEvents.begin(); 
	 it != scintBetaEvents.end(); it++) {
        double energy = (*it)->GetCalEnergy();
        ++multiplicity;
        plot(D_ENERGY_BETA, energy);
    }
    plot(D_MULT_BETA, multiplicity);
    return true;
}

bool BetaScintProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
        return false;

    static const vector<ChanEvent*> &scintBetaEvents = 
	event.GetSummary("beta_scint:beta", true)->GetList();

    /* Beta events gated by "Beta" place are plotted here, to make sanity 
     * check of selected gates */
    int multiplicity = 0;
    for (vector<ChanEvent*>::const_iterator it = scintBetaEvents.begin(); 
	 it != scintBetaEvents.end(); it++) {
        double energy = (*it)->GetCalEnergy();
        double time = (*it)->GetTime();
        int location = (*it)->GetChanID().GetLocation();

        PlaceOR* betas = dynamic_cast<PlaceOR*>(
                            TreeCorrelator::get()->place("Beta"));
        for (deque<EventData>::iterator itb = betas->info_.begin();
             itb != betas->info_.end(); ++itb) {
            if (itb->energy == energy && itb->time == time &&
                itb->location == location) {
                ++multiplicity;
                plot(D_ENERGY_BETA_GATED, energy);
            }
        }
    }
    plot(D_MULT_BETA_GATED, multiplicity);

    EndProcess();
    return true;
}
