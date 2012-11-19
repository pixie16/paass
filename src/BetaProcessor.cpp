/** \file BetaProcessor.cpp
 *
 * implementation for beta scintillator processor
 */
#include <vector>
#include <sstream>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "RawEvent.hpp"
#include "BetaProcessor.hpp"

using namespace std;
using namespace dammIds::scint::beta;

namespace dammIds {
    namespace scint {
        namespace beta {
        const int D_MULT_BETA = 0;
        }
    }
} 

BetaProcessor::BetaProcessor() : EventProcessor(OFFSET, RANGE) {
    name = "Beta";
    associatedTypes.insert("scint"); 
}

void BetaProcessor::DeclarePlots(void) {
    DeclareHistogram1D(D_MULT_BETA, S4, "Beta multiplicity");
}

bool BetaProcessor::PreProcess(RawEvent &event){
    if (!EventProcessor::PreProcess(event))
        return false;

    static const vector<ChanEvent*> &scintBetaEvents = 
	event.GetSummary("scint:beta")->GetList();

    int multiplicity = 0;
    for (vector<ChanEvent*>::const_iterator it = scintBetaEvents.begin(); 
	 it != scintBetaEvents.end(); it++) {
        string place = (*it)->GetChanID().GetPlaceName();
        if (TreeCorrelator::get().places.count(place) == 1) {
            double time   = (*it)->GetTime();
            double energy = (*it)->GetEnergy();
            // Activate B counter only for betas above some threshold
            if (energy > detectors::betaThreshold) {
                ++multiplicity;
                CorrEventData data(time, true, energy);
                TreeCorrelator::get().places[place]->activate(data);
            }
        } else {
            cerr << "In BetaProcessor: beta place " << place
                    << " does not exist." << endl;
            return false;
        }
    }
    plot(D_MULT_BETA, multiplicity);
    return true;
}

bool BetaProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
        return false;
    EndProcess();
    return true;
}
