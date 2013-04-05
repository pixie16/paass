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

        const int DD_ENERGY_BETA__TIME_NONEUTRON_TOTAL = 10;
        const int DD_ENERGY_BETA__TIME_NONEUTRON_NOG = 11;
        const int DD_ENERGY_BETA__TIME_NONEUTRON_G = 12;

        const int DD_ENERGY_BETA__TIME_NEUTRON_TOTAL = 15;
        const int DD_ENERGY_BETA__TIME_NEUTRON_NOG = 16;
        const int DD_ENERGY_BETA__TIME_NEUTRON_G = 17;

        const int DD_ENERGY_BETA__TIME_TM_NONEUTRON_TOTAL = 20;
        const int DD_ENERGY_BETA__TIME_TM_NONEUTRON_NOG = 21;
        const int DD_ENERGY_BETA__TIME_TM_NONEUTRON_G = 22;

        const int DD_ENERGY_BETA__TIME_TM_NEUTRON_TOTAL = 25;
        const int DD_ENERGY_BETA__TIME_TM_NEUTRON_NOG = 26;
        const int DD_ENERGY_BETA__TIME_TM_NEUTRON_G = 27;
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

    DeclareHistogram2D(DD_ENERGY_BETA__TIME_NONEUTRON_TOTAL, SC, S9,
                       "No n total beta energy/4 vs time 10 ms / bin");
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_NONEUTRON_NOG, SC, S9,
                       "No n no gamma beta energy/4 vs time 10 ms / bin");
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_NONEUTRON_G, SC, S9,
                       "No n gamma beta energy/4 vs time 10 ms / bin");

    DeclareHistogram2D(DD_ENERGY_BETA__TIME_NEUTRON_TOTAL, SC, S9,
                       "Neutron, total beta energy/4 vs time 10 ms / bin");
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_NEUTRON_NOG, SC, S9,
                       "Neutron, no gamma beta energy/4 vs time 10 ms / bin");
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_NEUTRON_G, SC, S9,
                       "Neutron gamma beta energy/4 vs time 10 ms / bin");

    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_NONEUTRON_TOTAL, SC, S9,
                "Tape move No n total beta energy/4 vs time 10 ms / bin");
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_NONEUTRON_NOG, SC, S9,
                "Tape move No n no gamma beta energy/4 vs time 10 ms / bin");
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_NONEUTRON_G, SC, S9,
                "Tape move No n gamma beta energy/4 vs time 10 ms / bin");

    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_NEUTRON_TOTAL, SC, S9,
            "Tape move Neutron, total beta energy/4 vs time 10 ms / bin");
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_NEUTRON_NOG, SC, S9,
            "Tape move Neutron, no gamma beta energy/4 vs time 10 ms / bin");
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_NEUTRON_G, SC, S9,
            "Tape move Neutron gamma beta energy/4 vs time 10 ms / bin");
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

    double clockInSeconds = Globals::get()->clockInSeconds();

    /** Place Cycle is activated by BeamOn event and deactivated by TapeMove*/
    bool tapeMove = !(TreeCorrelator::get()->place("Cycle")->status());

    /** Cycle time is measured from the begining of the last BeamON event */
    double cycleTime = TreeCorrelator::get()->place("Cycle")->last().time;
    /** Time plot resolution 5 ms / bin */
    double plotResolution = 10e-3;

    /* Number of neutrons as selected by gates on 3hen spectrum.
     * See DetectorDriver::InitCorrelator for gates. */
    int neutron_count = 
        dynamic_cast<PlaceCounter*>(
                TreeCorrelator::get()->place("Neutrons"))->getCounter();

    /** True if gammas were recorded during the event */
    bool hasGamma = TreeCorrelator::get()->place("Gamma")->status();

    int multiplicity = 0;
    for (vector<ChanEvent*>::const_iterator it = scintBetaEvents.begin(); 
	 it != scintBetaEvents.end(); it++) {
        double energy = (*it)->GetCalEnergy();
        double time = (*it)->GetTime();
        int location = (*it)->GetChanID().GetLocation();

        PlaceOR* betas = dynamic_cast<PlaceOR*>(
                            TreeCorrelator::get()->place("Beta"));
        /* Beta events gated by "Beta" place are plotted here, to make sanity 
        * check of selected gates */
        for (deque<EventData>::iterator itb = betas->info_.begin();
             itb != betas->info_.end(); ++itb) {
            if (itb->energy == energy && itb->time == time &&
                itb->location == location) {
                ++multiplicity;
                plot(D_ENERGY_BETA_GATED, energy);
            }
        }
        /** Skip the rest if beta energy is smaller then 1 */
        if (energy < 1)
            continue;

        double decayTime = (time - cycleTime) * clockInSeconds;

        int decayTimeBin = int(decayTime / plotResolution);
        int energyBin = int(energy / 4.0);

        if (tapeMove) {
            if (neutron_count == 0) {
                plot(DD_ENERGY_BETA__TIME_TM_NONEUTRON_TOTAL,
                     energyBin, decayTimeBin);
                if (!hasGamma)
                    plot(DD_ENERGY_BETA__TIME_TM_NONEUTRON_NOG,
                         energyBin, decayTimeBin);
                else
                    plot(DD_ENERGY_BETA__TIME_TM_NONEUTRON_G,
                         energyBin, decayTimeBin);
            } else {
                plot(DD_ENERGY_BETA__TIME_TM_NEUTRON_TOTAL,
                     energyBin, decayTimeBin);
                if (!hasGamma)
                    plot(DD_ENERGY_BETA__TIME_TM_NEUTRON_NOG,
                         energyBin, decayTimeBin);
                else
                    plot(DD_ENERGY_BETA__TIME_TM_NEUTRON_G,
                         energyBin, decayTimeBin);
            }
        } else {
            if (neutron_count == 0) {
                plot(DD_ENERGY_BETA__TIME_NONEUTRON_TOTAL,
                     energyBin, decayTimeBin);
                if (!hasGamma)
                    plot(DD_ENERGY_BETA__TIME_NONEUTRON_NOG,
                         energyBin, decayTimeBin);
                else
                    plot(DD_ENERGY_BETA__TIME_NONEUTRON_G,
                         energyBin, decayTimeBin);
            } else {
                plot(DD_ENERGY_BETA__TIME_NEUTRON_TOTAL,
                     energyBin, decayTimeBin);
                if (!hasGamma)
                    plot(DD_ENERGY_BETA__TIME_NEUTRON_NOG,
                         energyBin, decayTimeBin);
                else
                    plot(DD_ENERGY_BETA__TIME_NEUTRON_G,
                         energyBin, decayTimeBin);
            }
        }
    }

    plot(D_MULT_BETA_GATED, multiplicity);

    EndProcess();
    return true;
}
