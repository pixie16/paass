/** \file BetaBetaScintProcessor.cpp
 *
 * implementation for beta scintillator processor
 */
#include <vector>
#include <sstream>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "RawEvent.hpp"
#include "BetaScintProcessor.hpp"

using namespace std;
using namespace dammIds::beta_scint;

namespace dammIds {
    namespace beta_scint {
    }
} 

BetaScintProcessor::BetaScintProcessor() : EventProcessor(OFFSET, RANGE)
{
    name = "beta_scint";
    associatedTypes.insert("beta_scint"); 
}

void BetaScintProcessor::DeclarePlots(void)
{

}

bool BetaScintProcessor::PreProcess(RawEvent &event){
    if (!EventProcessor::PreProcess(event))
        return false;

    // Beta energy threshold
    static const int BETA_THRESHOLD = 10;

    static const vector<ChanEvent*> &scintBetaEvents = 
	event.GetSummary("beta_scint:beta")->GetList();

    for (vector<ChanEvent*>::const_iterator it = scintBetaEvents.begin(); 
	 it != scintBetaEvents.end(); it++) {
        string place = (*it)->GetChanID().GetPlaceName();
        if (TreeCorrelator::get().places.count(place) == 1) {
            double time   = (*it)->GetTime();
            double energy = (*it)->GetEnergy();
            // Activate B counter only for betas above some threshold
            if (energy > BETA_THRESHOLD) {
                CorrEventData data(time, true, energy);
                TreeCorrelator::get().places[place]->activate(data);
            }
        } else {
            cerr << "In BetaScintProcessor: beta place " << place
                    << " does not exist." << endl;
            return false;
        }
    }
    return true;
}

bool BetaScintProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
        return false;
    EndProcess();
    return true;
}
