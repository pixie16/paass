/** \file BetaProcessor.cpp
 *
 * implementation for beta scintillator processor
 */
#include <vector>
#include <sstream>

#include "DammPlotIds.hpp"
#include "RawEvent.hpp"
#include "ChanEvent.hpp"
#include "BetaProcessor.hpp"

using namespace std;
using namespace dammIds::scint::beta;

namespace dammIds {
    namespace scint {
        namespace beta {
            const int D_MULT_BETA = 0; //!< beta multiplicity
            const int D_ENERGY_BETA = 1;//!< beta energy
        }
    }
}

BetaProcessor::BetaProcessor() : EventProcessor(OFFSET, RANGE, "BetaProcessor") {
    associatedTypes.insert("scint");
}

void BetaProcessor::DeclarePlots(void) {
    DeclareHistogram1D(D_MULT_BETA, S4, "Beta multiplicity");
    DeclareHistogram1D(D_ENERGY_BETA, SE, "Beta energy");
}

bool BetaProcessor::PreProcess(RawEvent &event){
    if (!EventProcessor::PreProcess(event))
        return false;

    static const vector<ChanEvent*> &scintBetaEvents =
	event.GetSummary("scint:beta")->GetList();

    int multiplicity = 0;
    for (vector<ChanEvent*>::const_iterator it = scintBetaEvents.begin();
	 it != scintBetaEvents.end(); it++) {
        double energy = (*it)->GetEnergy();
        if (energy > detectors::betaThreshold)
            ++multiplicity;
        plot(D_ENERGY_BETA, energy);
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
