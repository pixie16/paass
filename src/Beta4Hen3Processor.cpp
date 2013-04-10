/** \file Beta4Hen3Processor.cpp
 *
 * implementation for beta scintillator processor for 3Hen hybrid experiment
 */
#include <vector>
#include <sstream>

#include "DammPlotIds.hpp"
#include "RawEvent.hpp"
#include "ChanEvent.hpp"
#include "BetaScintProcessor.hpp"
#include "Beta4Hen3Processor.hpp"

using namespace std;
using namespace dammIds::beta_scint;

namespace dammIds {
    namespace beta_scint {

        const int DD_ENERGY_BETA__TIME_NEUTRON_TOTAL = 15;
        const int DD_ENERGY_BETA__TIME_NEUTRON_NOG = 16;
        const int DD_ENERGY_BETA__TIME_NEUTRON_G = 17;

        const int DD_ENERGY_BETA__TIME_TM_NEUTRON_TOTAL = 25;
        const int DD_ENERGY_BETA__TIME_TM_NEUTRON_NOG = 26;
        const int DD_ENERGY_BETA__TIME_TM_NEUTRON_G = 27;

        const int DD_ENERGY_BETA__TIME_MNEUTRON_TOTAL = 30;
        const int DD_ENERGY_BETA__TIME_MNEUTRON_NOG = 31;
        const int DD_ENERGY_BETA__TIME_MNEUTRON_G = 32;

        const int DD_ENERGY_BETA__TIME_TM_MNEUTRON_TOTAL = 35;
        const int DD_ENERGY_BETA__TIME_TM_MNEUTRON_NOG = 36;
        const int DD_ENERGY_BETA__TIME_TM_MNEUTRON_G = 37;
    }
} 

Beta4Hen3Processor::Beta4Hen3Processor() : BetaScintProcessor()
{
}

void Beta4Hen3Processor::DeclarePlots(void) {
    BetaScintProcessor::DeclarePlots();

    const int energyBins2 = SB;
    const int timeBins = S9;

    stringstream title_end;
    title_end << "energy/" << timeSpectraEnergyContraction << " vs time "
              << timeSpectraTimeResolution << "/bin (s)";

    stringstream title;

    title << "Neutron-gated Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_NEUTRON_TOTAL, energyBins2,
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Neutron-gated No-gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_NEUTRON_NOG, energyBins2,
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Neutron-gated Gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_NEUTRON_G, energyBins2, 
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Neutron-gated Tape move Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_NEUTRON_TOTAL, energyBins2,
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Neutron-gated Tape move No-gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_NEUTRON_NOG, energyBins2,
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Neutron-gated Tape move Gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_NEUTRON_G, energyBins2,
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Multi-Neutron-gated Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_MNEUTRON_TOTAL, energyBins2,
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Multi-Neutron-gated No-gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_MNEUTRON_NOG, energyBins2,
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Multi-Neutron-gated Gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_MNEUTRON_G, energyBins2, 
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Multi-Neutron-gated Tape move Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_MNEUTRON_TOTAL, energyBins2,
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Multi-Neutron-gated Tape move No-gamma-gated Beta " 
          << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_MNEUTRON_NOG, energyBins2,
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Multi-Neutron-gated Tape move Gamma-gated Beta " 
          << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_MNEUTRON_G, energyBins2,
                       timeBins, title.str().c_str()); 
}

bool Beta4Hen3Processor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
        return false;

    // Call base class processing
    BetaScintProcessor::Process(event);

    /* Number of neutrons as selected by gates on 3hen spectrum.
     * See DetectorDriver::InitCorrelator for gates. */
    int neutron_count = 
        dynamic_cast<PlaceCounter*>(
                TreeCorrelator::get()->place("Neutrons"))->getCounter();

    /** All non-neutron related spectra are processed in the base class 
     * Here we are interested in neutron spectra only so there is no need
     * of further processing. */
    if (neutron_count < 1)
        return true;

    static const vector<ChanEvent*> &scintBetaEvents = 
	event.GetSummary("beta_scint:beta", true)->GetList();

    double clockInSeconds = Globals::get()->clockInSeconds();

    /** Place Cycle is activated by BeamOn event and deactivated by TapeMove*/
    bool tapeMove = !(TreeCorrelator::get()->place("Cycle")->status());

    /** Cycle time is measured from the begining of the last BeamON event */
    double cycleTime = TreeCorrelator::get()->place("Cycle")->last().time;
    //double cycleTime = TreeCorrelator::get()->place("CycleTape")->last().time;

    /** True if gammas were recorded during the event */
    bool hasGamma = TreeCorrelator::get()->place("Gamma")->status();

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
                double decayTime = (time - cycleTime) * clockInSeconds;
                int decayTimeBin = int(decayTime / timeSpectraTimeResolution);
                int energyBin = int(energy / timeSpectraEnergyContraction);

                if (tapeMove) {
                    plot(DD_ENERGY_BETA__TIME_TM_NEUTRON_TOTAL, 
                        energyBin, decayTimeBin);
                    if (!hasGamma)
                        plot(DD_ENERGY_BETA__TIME_TM_NEUTRON_NOG, 
                            energyBin, decayTimeBin);
                    else
                        plot(DD_ENERGY_BETA__TIME_TM_NEUTRON_G, 
                            energyBin, decayTimeBin);
                    if (neutron_count > 1) {
                        plot(DD_ENERGY_BETA__TIME_TM_MNEUTRON_TOTAL, 
                            energyBin, decayTimeBin);
                        if (!hasGamma)
                            plot(DD_ENERGY_BETA__TIME_TM_MNEUTRON_NOG, 
                                energyBin, decayTimeBin);
                        else
                            plot(DD_ENERGY_BETA__TIME_TM_MNEUTRON_G, 
                                energyBin, decayTimeBin);
                    }
                } else {
                    plot(DD_ENERGY_BETA__TIME_NEUTRON_TOTAL,
                            energyBin, decayTimeBin);
                    if (!hasGamma)
                        plot(DD_ENERGY_BETA__TIME_NEUTRON_NOG,
                            energyBin, decayTimeBin);
                    else
                        plot(DD_ENERGY_BETA__TIME_NEUTRON_G,
                            energyBin, decayTimeBin);
                    if (neutron_count > 1) {
                        plot(DD_ENERGY_BETA__TIME_MNEUTRON_TOTAL,
                                energyBin, decayTimeBin);
                        if (!hasGamma)
                            plot(DD_ENERGY_BETA__TIME_MNEUTRON_NOG,
                                energyBin, decayTimeBin);
                        else
                            plot(DD_ENERGY_BETA__TIME_MNEUTRON_G,
                                energyBin, decayTimeBin);
                    }
                }
                //Break the deque loop since we found the matching
                //event
                break;
            } //if
        } //Deque loop
    } // ChanEvent loop

    EndProcess();
    return true;
}
