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
        namespace neutron{
            const int D_ENERGY_BETA_GAMMA_GATED = 7;
            const int DD_ENERGY_BETA__GAMMA = 8;

            const int DD_ENERGY_BETA__TIME_TOTAL = 15;
            const int DD_ENERGY_BETA__TIME_NOG = 16;
            const int DD_ENERGY_BETA__TIME_G = 17;

            const int DD_ENERGY_BETA__TIME_TM_TOTAL = 25;
            const int DD_ENERGY_BETA__TIME_TM_NOG = 26;
            const int DD_ENERGY_BETA__TIME_TM_G = 27;
        }

        namespace multiNeutron {
            const int DD_ENERGY_BETA__TIME_TOTAL = 30;
            const int DD_ENERGY_BETA__TIME_NOG = 31;
            const int DD_ENERGY_BETA__TIME_G = 32;

            const int DD_ENERGY_BETA__TIME_TM_TOTAL = 35;
            const int DD_ENERGY_BETA__TIME_TM_NOG = 36;
            const int DD_ENERGY_BETA__TIME_TM_G = 37;
        }
    }
} 

Beta4Hen3Processor::Beta4Hen3Processor(double gammaBetaLimit,
                                       double energyContraction) :
    BetaScintProcessor(gammaBetaLimit, energyContraction)
{
}

void Beta4Hen3Processor::DeclarePlots(void) {
    BetaScintProcessor::DeclarePlots();

    const int energyBins = SB;
    const int timeBins = S9;

    stringstream title_end;
    title_end << "energy/" << energyContraction_ << " vs time "
              << timeSpectraTimeResolution << "/bin (s)";

    stringstream title;

    title.str("");
    title << "Beta energy/" << energyContraction_ << " neutron gamma gated";
    DeclareHistogram1D(neutron::D_ENERGY_BETA_GAMMA_GATED, energyBins,
                       title.str().c_str());

    title.str("");
    title << "Beta energy/" << energyContraction_ 
          << " vs gamma energy neutron gated ";
    DeclareHistogram2D(neutron::DD_ENERGY_BETA__GAMMA, energyBins, SC, 
                        title.str().c_str()); 

    title << "Neutron-gated Beta " << title_end.str();
    DeclareHistogram2D(neutron::DD_ENERGY_BETA__TIME_TOTAL, energyBins,
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Neutron-gated No-gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(neutron::DD_ENERGY_BETA__TIME_NOG, energyBins,
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Neutron-gated Gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(neutron::DD_ENERGY_BETA__TIME_G, energyBins, 
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Neutron-gated Tape move Beta " << title_end.str();
    DeclareHistogram2D(neutron::DD_ENERGY_BETA__TIME_TM_TOTAL, energyBins,
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Neutron-gated Tape move No-gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(neutron::DD_ENERGY_BETA__TIME_TM_NOG, energyBins,
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Neutron-gated Tape move Gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(neutron::DD_ENERGY_BETA__TIME_TM_G, energyBins,
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Multi-Neutron-gated Beta " << title_end.str();
    DeclareHistogram2D(multiNeutron::DD_ENERGY_BETA__TIME_TOTAL, energyBins,
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Multi-Neutron-gated No-gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(multiNeutron::DD_ENERGY_BETA__TIME_NOG, energyBins,
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Multi-Neutron-gated Gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(multiNeutron::DD_ENERGY_BETA__TIME_G, energyBins, 
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Multi-Neutron-gated Tape move Beta " << title_end.str();
    DeclareHistogram2D(multiNeutron::DD_ENERGY_BETA__TIME_TM_TOTAL, energyBins,
                       timeBins, title.str().c_str()); 

    title.str("");
    title << "Multi-Neutron-gated Tape move No-gamma-gated Beta " 
          << title_end.str();
    DeclareHistogram2D(multiNeutron::DD_ENERGY_BETA__TIME_TM_NOG,
            energyBins, timeBins, title.str().c_str()); 

    title.str("");
    title << "Multi-Neutron-gated Tape move Gamma-gated Beta " 
          << title_end.str();
    DeclareHistogram2D(multiNeutron::DD_ENERGY_BETA__TIME_TM_G,
            energyBins, timeBins, title.str().c_str()); 
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

    for (vector<ChanEvent*>::const_iterator it = scintBetaEvents.begin(); 
	 it != scintBetaEvents.end(); it++) {
        double energy = (*it)->GetCalEnergy();
        int energyBin = int(energy / energyContraction_);
        double time = (*it)->GetTime();

        //Skip the energy-time spectra for zero energy events
        if (energyBin < 1)
            continue;

        double decayTime = (time - cycleTime) * clockInSeconds;
        int decayTimeBin = int(decayTime / timeSpectraTimeResolution);

        EventData bestGamma = BestGammaForBeta(time);
        double gb_dtime = (time - bestGamma.time) * clockInSeconds;

        if (GoodGammaBeta(gb_dtime)) {
            plot(neutron::D_ENERGY_BETA_GAMMA_GATED, energyBin);
            plot(neutron::DD_ENERGY_BETA__GAMMA, energyBin, bestGamma.energy);
        }

        if (tapeMove) {
            plot(neutron::DD_ENERGY_BETA__TIME_TM_TOTAL, 
                energyBin, decayTimeBin);
            if (GoodGammaBeta(gb_dtime))
                plot(neutron::DD_ENERGY_BETA__TIME_TM_G, 
                    energyBin, decayTimeBin);
            else
                plot(neutron::DD_ENERGY_BETA__TIME_TM_NOG, 
                    energyBin, decayTimeBin);
            if (neutron_count > 1) {
                plot(multiNeutron::DD_ENERGY_BETA__TIME_TM_TOTAL, 
                    energyBin, decayTimeBin);
                if (GoodGammaBeta(gb_dtime))
                    plot(multiNeutron::DD_ENERGY_BETA__TIME_TM_G, 
                        energyBin, decayTimeBin);
                else
                    plot(multiNeutron::DD_ENERGY_BETA__TIME_TM_NOG, 
                        energyBin, decayTimeBin);
            }
        } else {
            plot(neutron::DD_ENERGY_BETA__TIME_TOTAL,
                    energyBin, decayTimeBin);
            if (GoodGammaBeta(gb_dtime))
                plot(neutron::DD_ENERGY_BETA__TIME_G,
                    energyBin, decayTimeBin);
            else
                plot(neutron::DD_ENERGY_BETA__TIME_NOG,
                    energyBin, decayTimeBin);
            if (neutron_count > 1) {
                plot(multiNeutron::DD_ENERGY_BETA__TIME_TOTAL,
                        energyBin, decayTimeBin);
                if (GoodGammaBeta(gb_dtime))
                    plot(multiNeutron::DD_ENERGY_BETA__TIME_G,
                        energyBin, decayTimeBin);
                else
                    plot(multiNeutron::DD_ENERGY_BETA__TIME_NOG,
                        energyBin, decayTimeBin);
            }
        }
    } // ChanEvent loop

    EndProcess();
    return true;
}
