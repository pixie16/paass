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

BetaScintProcessor::BetaScintProcessor() : 
    EventProcessor(OFFSET, RANGE, "beta_scint") {
    associatedTypes.insert("beta_scint"); 
}

void BetaScintProcessor::DeclarePlots(void) {
    const int energyBins1 = SE;
    const int energyBins2 = SB;
    const int timeBins = S9;

    DeclareHistogram1D(D_MULT_BETA, S4, "Beta multiplicity");
    DeclareHistogram1D(D_ENERGY_BETA, energyBins1, "Beta energy");

    DeclareHistogram1D(D_MULT_BETA_GATED, S4, "Beta multiplicity gated");
    DeclareHistogram1D(D_ENERGY_BETA_GATED, energyBins1, "Beta energy gated");


    stringstream title_end;
    title_end << "energy/" << timeSpectraEnergyContraction << " vs time "
              << timeSpectraTimeResolution << "/bin (s)";

    stringstream title;

    title << "Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TOTAL, energyBins2, timeBins, 
            title.str().c_str()); 

    title.str("");
    title << "No-gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_NOG, energyBins2, timeBins,
            title.str().c_str()); 

    title.str("");
    title << "Gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_G, energyBins2, timeBins,
            title.str().c_str()); 

    title.str("");
    title << "Tape move Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_TOTAL, energyBins2, timeBins,
            title.str().c_str()); 

    title.str("");
    title << "Tape move No-gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_NOG, energyBins2, timeBins,
            title.str().c_str()); 

    title.str("");
    title << "Tape move Gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_G, energyBins2, timeBins,
            title.str().c_str()); 
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
    //double cycleTime = TreeCorrelator::get()->place("Cycle")->last().time;
    double cycleTime = TreeCorrelator::get()->place("CycleTape")->last().time;

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
        /* Beta events gated by "Beta" place are plotted here 
         * Energy-time spectra are gated
         * */
        for (deque<EventData>::iterator itb = betas->info_.begin();
             itb != betas->info_.end(); ++itb) {
            if (itb->energy == energy && itb->time == time &&
                itb->location == location) {
                ++multiplicity;
                plot(D_ENERGY_BETA_GATED, energy);

                double decayTime = (time - cycleTime) * clockInSeconds;
                int decayTimeBin = int(decayTime / timeSpectraTimeResolution);
                int energyBin = int(energy / timeSpectraEnergyContraction);

                if (tapeMove) {
                    plot(DD_ENERGY_BETA__TIME_TM_TOTAL, 
                         energyBin, decayTimeBin);
                    if (!hasGamma)
                        plot(DD_ENERGY_BETA__TIME_TM_NOG, 
                             energyBin, decayTimeBin);
                    else
                        plot(DD_ENERGY_BETA__TIME_TM_G, 
                             energyBin, decayTimeBin);
                } else {
                    plot(DD_ENERGY_BETA__TIME_TOTAL,
                            energyBin, decayTimeBin);
                    if (!hasGamma)
                        plot(DD_ENERGY_BETA__TIME_NOG,
                             energyBin, decayTimeBin);
                    else
                        plot(DD_ENERGY_BETA__TIME_G,
                             energyBin, decayTimeBin);
                }
                break;
            }
        }
    }

    plot(D_MULT_BETA_GATED, multiplicity);

    EndProcess();
    return true;
}
