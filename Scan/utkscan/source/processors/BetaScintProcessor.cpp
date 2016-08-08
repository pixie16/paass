/** \file BetaScintProcessor.cpp
 *
 * implementation for beta scintillator processor
 * \author K. A. Miernik
 * \date 2012
 */

#include <limits>
#include <vector>
#include <sstream>

#include "DammPlotIds.hpp"
#include "RawEvent.hpp"
#include "ChanEvent.hpp"
#include "BetaScintProcessor.hpp"

using namespace std;
using namespace dammIds::beta_scint;

/*! Magic number for the resolution of the time spectra */
static const double timeSpectraTimeResolution = 10.e-3;


BetaScintProcessor::BetaScintProcessor(double gammaBetaLimit,
                                       double energyContraction) :
    EventProcessor(OFFSET, RANGE, "BetaScintProcessor") {
    associatedTypes.insert("beta_scint");
    gammaBetaLimit_ = gammaBetaLimit;
    energyContraction_ = energyContraction;
}

EventData BetaScintProcessor::BestGammaForBeta(double bTime) {
    PlaceOR* gammas =
        dynamic_cast<PlaceOR*>(TreeCorrelator::get()->place("Gamma"));
    unsigned sz = gammas->info_.size();

    if (sz == 0)
        return EventData(-1);

    double bestTime = numeric_limits<double>::max();
    unsigned bestIndex = -1;
    for (int index = sz - 1; index >= 0; --index) {
        double dtime = (bTime - gammas->info_.at(index).time);
        if (abs(dtime) < abs(bestTime)) {
            bestTime = dtime;
            bestIndex = index;
        }
    }
    return gammas->info_.at(bestIndex);
}

bool BetaScintProcessor::GoodGammaBeta(double gb_dtime) {
    if (abs(gb_dtime) > gammaBetaLimit_)
        return false;
    return true;
}

void BetaScintProcessor::DeclarePlots(void) {
    const int energyBins = SB;
    const int timeBins = S9;

    stringstream title;

    DeclareHistogram1D(D_MULT_BETA, S4, "Beta multiplicity");

    title.str("");
    title << "Beta energy/" << energyContraction_;
    DeclareHistogram1D(D_ENERGY_BETA, energyBins, title.str().c_str());

    DeclareHistogram1D(D_MULT_BETA_THRES_GATED, S4,
            "Beta multiplicity threshold gated");

    title.str("");
    title << "Beta energy/" << energyContraction_ << " threshold gated";
    DeclareHistogram1D(D_ENERGY_BETA_THRES_GATED, energyBins,
                       title.str().c_str());

    DeclareHistogram1D(D_MULT_BETA_GAMMA_GATED, S4,
             "Beta multiplicity gamma gated");

    title.str("");
    title << "Beta energy/" << energyContraction_ << " gamma gated";
    DeclareHistogram1D(D_ENERGY_BETA_GAMMA_GATED, energyBins,
                       title.str().c_str());

    title.str("");
    title << "Beta energy/" << energyContraction_ << " vs gamma energy ";
    DeclareHistogram2D(DD_ENERGY_BETA__GAMMA, energyBins, SC,
                        title.str().c_str());

    stringstream title_end;
    title_end << "energy/" << energyContraction_ << " vs time "
              << timeSpectraTimeResolution << "/bin (s)";

    title.str("");
    title << "Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TOTAL, energyBins, timeBins,
            title.str().c_str());

    title.str("");
    title << "No-gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_NOG, energyBins, timeBins,
            title.str().c_str());

    title.str("");
    title << "Gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_G, energyBins, timeBins,
            title.str().c_str());

    title.str("");
    title << "Tape move Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_TOTAL, energyBins, timeBins,
            title.str().c_str());

    title.str("");
    title << "Tape move No-gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_NOG, energyBins, timeBins,
            title.str().c_str());

    title.str("");
    title << "Tape move Gamma-gated Beta " << title_end.str();
    DeclareHistogram2D(DD_ENERGY_BETA__TIME_TM_G, energyBins, timeBins,
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
        int energyBin = int(energy / energyContraction_);
        ++multiplicity;
        plot(D_ENERGY_BETA, energyBin);
    }
    plot(D_MULT_BETA, multiplicity);
    return(true);
}

bool BetaScintProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    static const vector<ChanEvent*> &scintBetaEvents =
	event.GetSummary("beta_scint:beta", true)->GetList();

    double clockInSeconds = Globals::get()->clockInSeconds();

    /** Place Cycle is activated by BeamOn event and deactivated by TapeMove*/
    bool tapeMove = !(TreeCorrelator::get()->place("Cycle")->status());

    /** Cycle time is measured from the begining of the last BeamON event */
    double cycleTime = TreeCorrelator::get()->place("Cycle")->last().time;

    /** True if gammas were recorded during the event */
    int multiplicityThres = 0;
    int multiplicityGamma = 0;
    for (vector<ChanEvent*>::const_iterator it = scintBetaEvents.begin();
	 it != scintBetaEvents.end(); it++) {
        double energy = (*it)->GetCalEnergy();
        int energyBin = int(energy / energyContraction_);
        double time = (*it)->GetTime();
        int location = (*it)->GetChanID().GetLocation();

        PlaceOR* betas =
            dynamic_cast<PlaceOR*>(TreeCorrelator::get()->place("Beta"));
        /* Beta events gated by "Beta" place are plotted here
         * Energy-time spectra are gated
         * */
        for (deque<EventData>::iterator itb = betas->info_.begin();
             itb != betas->info_.end(); ++itb) {
            if (itb->energy == energy && itb->time == time &&
                itb->location == location) {
                ++multiplicityThres;
                plot(D_ENERGY_BETA_THRES_GATED, energyBin);
                //Break the deque loop since we found the matching event
                break;
            }
        }

        //Skip the energy-time spectra for zero energy events
        if (energyBin < 1)
            continue;

        double decayTime = (time - cycleTime) * clockInSeconds;
        int decayTimeBin = int(decayTime / timeSpectraTimeResolution);

        EventData bestGamma = BestGammaForBeta(time);
        double gb_dtime = (time - bestGamma.time) * clockInSeconds;

        if (GoodGammaBeta(gb_dtime)) {
            plot(D_ENERGY_BETA_GAMMA_GATED, energyBin);
            plot(DD_ENERGY_BETA__GAMMA, energyBin, bestGamma.energy);
            ++multiplicityGamma;
        }

        if (tapeMove) {
            plot(DD_ENERGY_BETA__TIME_TM_TOTAL,
                    energyBin, decayTimeBin);
            if (GoodGammaBeta(gb_dtime))
                plot(DD_ENERGY_BETA__TIME_TM_G,
                        energyBin, decayTimeBin);
            else
                plot(DD_ENERGY_BETA__TIME_TM_NOG,
                        energyBin, decayTimeBin);
        } else {
            plot(DD_ENERGY_BETA__TIME_TOTAL,
                    energyBin, decayTimeBin);
            if (GoodGammaBeta(gb_dtime))
                plot(DD_ENERGY_BETA__TIME_G,
                        energyBin, decayTimeBin);
            else
                plot(DD_ENERGY_BETA__TIME_NOG,
                        energyBin, decayTimeBin);
        }
    }

    plot(D_MULT_BETA_THRES_GATED, multiplicityThres);
    plot(D_MULT_BETA_GAMMA_GATED, multiplicityGamma);

    EndProcess();
    return(true);
}
