/** \file BetaBetaScintProcessor.cpp
 *
 * implementation for beta scintillator processor
 */
#include <vector>
#include <sstream>

#include "DammPlotIds.hpp"
#include "RawEvent.hpp"
#include "BetaScintProcessor.hpp"

using namespace std;
using namespace dammIds::beta_scint;

namespace dammIds {
    namespace beta_scint {
        const int D_MULT_BETA = 0;
        const int D_ENERGY_BETA = 1;
    }
} 

BetaScintProcessor::BetaScintProcessor() : EventProcessor(OFFSET, RANGE) {
    name = "beta_scint";
    associatedTypes.insert("beta_scint"); 
}

void BetaScintProcessor::DeclarePlots(void) {
    DeclareHistogram1D(D_MULT_BETA, S4, "Beta multiplicity");
    DeclareHistogram1D(D_ENERGY_BETA, SA, "Beta energy");
}

bool BetaScintProcessor::PreProcess(RawEvent &event){
    if (!EventProcessor::PreProcess(event))
        return false;

    static const vector<ChanEvent*> &scintBetaEvents = 
	event.GetSummary("beta_scint:beta")->GetList();

    int multiplicity = 0;
    for (vector<ChanEvent*>::const_iterator it = scintBetaEvents.begin(); 
	 it != scintBetaEvents.end(); it++) {
        string place = (*it)->GetChanID().GetPlaceName();
        if (TreeCorrelator::get()->places.count(place) == 1) {
            double time   = (*it)->GetTime();
            double energy = (*it)->GetEnergy();
            // Activate B counter only for betas above some threshold
            if (energy > detectors::betaThreshold) {
                ++multiplicity;
                CorrEventData data(time, true, energy);
                TreeCorrelator::get()->places[place]->activate(data);
                plot(D_ENERGY_BETA, energy);
            }
        } else {
            cerr << "In BetaScintProcessor: beta place " << place
                    << " does not exist." << endl;
            return false;
        }
    }
    plot(D_MULT_BETA, multiplicity);
    return true;
}

bool BetaScintProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
        return false;
    EndProcess();
    return true;
}
