/** \file Ge4Hen3Processor.cpp
 *
 * implementation for germanium processor for 3Hen experiment
 */

//? Make a clover specific processor

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <utility>
#include <limits>

#include <cmath>
#include <cstdlib>


#include "Plots.hpp"
#include "PlotsRegister.hpp"
#include "DammPlotIds.hpp"

#include "Correlator.hpp"
#include "DetectorLibrary.hpp"
#include "GeProcessor.hpp"
#include "Ge4Hen3Processor.hpp"
#include "RawEvent.hpp"
#include "TreeCorrelator.hpp"

using namespace std;

namespace dammIds {
    namespace ge {
        namespace neutron {
            const int D_ENERGY = 200;
            const int DD_ENERGY = 202;
            const int DD_ENERGY_NEUTRON_LOC = 203;
            const int D_MULT = 209;
            const int DD_ENERGY__TIMEX = 221; 
            const int DD_ENERGY__TIMEX_GROW = 223; 
            const int DD_ENERGY__TIMEX_DECAY = 225; 

            const int D_ADD_ENERGY = 250; 
            const int DD_ADD_ENERGY = 252;
            const int D_ADD_ENERGY_TOTAL = 255;
            const int DD_ADD_ENERGY__TIMEX = 271;

            namespace betaGated {
                const int D_ENERGY = 210;
                const int D_ENERGY_PROMPT = 211;
                const int DD_ENERGY = 212;
                const int DD_ENERGY__TIMEX = 231; 

                const int D_ADD_ENERGY = 260; 
                const int D_ADD_ENERGY_TOTAL = 265;
                const int DD_ADD_ENERGY = 262;
                const int DD_ADD_ENERGY__TIMEX = 281;
            } 
        }

        namespace multiNeutron {
            const int D_ENERGY = 300;
            const int DD_ENERGY = 302;
            const int DD_ENERGY__TIMEX = 321; 

            const int D_ADD_ENERGY = 350; 
            const int DD_ADD_ENERGY = 352;
            const int D_ADD_ENERGY_TOTAL = 355;
            const int DD_ADD_ENERGY__TIMEX = 371;
            namespace betaGated {
                const int D_ENERGY = 310;
                const int D_ENERGY_PROMPT = 311;
                const int DD_ENERGY = 312;
                const int DD_ENERGY__TIMEX = 331; 

                const int D_ADD_ENERGY = 360; 
                const int DD_ADD_ENERGY = 362;
                const int D_ADD_ENERGY_TOTAL = 365;
                const int DD_ADD_ENERGY__TIMEX = 381; 
            } 
        }
    }
}

Ge4Hen3Processor::Ge4Hen3Processor(double gammaThreshold, double lowRatio,
                         double highRatio, double subEventWindow,
                         double gammaBetaLimit, double gammaGammaLimit,
                         double earlyLowLimit, double earlyHighLimit) :

                         GeProcessor(gammaThreshold, lowRatio, highRatio,
                                     subEventWindow, gammaBetaLimit, 
                                     gammaGammaLimit, earlyLowLimit,
                                     earlyHighLimit)
{
}

/** Declare plots including many for decay/implant/neutron gated analysis  */
void Ge4Hen3Processor::DeclarePlots(void) 
{
    GeProcessor::DeclarePlots();
    using namespace dammIds::ge;

    const int energyBins1  = SD;
    const int energyBins2  = SC;
    const int granTimeBins = SA;

    DeclareHistogram1D(neutron::D_ENERGY, energyBins1,
                      "Gamma singles neutron gated");
    DeclareHistogram1D(neutron::D_MULT, S3,
                       "Gamma multiplicity neutron gated");                  
    DeclareHistogram1D(neutron::D_ADD_ENERGY, energyBins1,
                       "Gamma addback neutron gated");
    DeclareHistogram1D(neutron::D_ADD_ENERGY_TOTAL, energyBins1,
                       "Gamma total addback neutron gated");
    DeclareHistogram2D(neutron::DD_ENERGY, energyBins2, energyBins2,
                       "Gamma gamma neutron gated");
    DeclareHistogram2D(neutron::DD_ENERGY_NEUTRON_LOC, SD, S6,
                       "Gamma energy vs neutron location");
    DeclareHistogram2D(neutron::DD_ADD_ENERGY, energyBins2, energyBins2,
                       "Gamma gamma addback neutron gated");
    DeclareHistogramGranY(neutron::DD_ENERGY__TIMEX, energyBins2, granTimeBins,
                          "E - Time neutron gated", 2, timeResolution, "s");
    DeclareHistogramGranY(neutron::DD_ENERGY__TIMEX_GROW,
                          energyBins2, granTimeBins,
                          "E - Time neutron gated, beam on only",
                          2, timeResolution, "s");
    DeclareHistogramGranY(neutron::DD_ENERGY__TIMEX_DECAY,
                          energyBins2, granTimeBins,
                          "E - Time neutron gated, beam off only",
                          2, timeResolution, "s");
    DeclareHistogramGranY(neutron::DD_ADD_ENERGY__TIMEX,
                          energyBins2, granTimeBins,
                          "Addback E - Time", 2, timeResolution, "s");

    DeclareHistogram1D(neutron::betaGated::D_ENERGY, energyBins1,
                       "Gamma singles beta neutron gated");
    DeclareHistogram1D(neutron::betaGated::D_ENERGY_PROMPT, energyBins1,
                       "Gamma singles beta prompt neutron gated");
    DeclareHistogram1D(neutron::betaGated::D_ADD_ENERGY, energyBins1,
                       "Gamma addback beta neutron gated");
    DeclareHistogram1D(neutron::betaGated::D_ADD_ENERGY_TOTAL, energyBins1,
                       "Gamma total addback beta neutron gated");
    DeclareHistogram2D(neutron::betaGated::DD_ENERGY, energyBins2, energyBins2,
                       "Gamma gamma beta neutron gated");
    DeclareHistogram2D(neutron::betaGated::DD_ADD_ENERGY,
                       energyBins2, energyBins2,
                       "Gamma gamma addback beta neutron gated");
    DeclareHistogramGranY(neutron::betaGated::DD_ENERGY__TIMEX,
                          energyBins2, granTimeBins,
                          "E - Time beta neutron gated",
                          2, timeResolution, "s");
    DeclareHistogramGranY(neutron::betaGated::DD_ADD_ENERGY__TIMEX,
                          energyBins2, granTimeBins,
                          "Addback E - Time beta neutron gated",
                          2, timeResolution, "s");

    DeclareHistogram1D(multiNeutron::D_ENERGY, energyBins1,
                       "Gamma singles multiNeutron gated");
    DeclareHistogram1D(multiNeutron::D_ADD_ENERGY, energyBins1,
                       "Gamma addback multiNeutron gated");
    DeclareHistogram1D(multiNeutron::D_ADD_ENERGY_TOTAL, energyBins1,
                       "Gamma total addback  multiNeutron gated");
    DeclareHistogram2D(multiNeutron::DD_ENERGY, energyBins2, energyBins2,
                       "Gamma gamma multineutron gated");
    DeclareHistogram2D(multiNeutron::DD_ADD_ENERGY, energyBins2, energyBins2,
                       "Gamma gamma addback multineutron gated");
    DeclareHistogramGranY(multiNeutron::DD_ENERGY__TIMEX,
                          energyBins2, granTimeBins,
                          "E - Time multineutron gated",
                          2, timeResolution, "s");
    DeclareHistogramGranY(multiNeutron::DD_ADD_ENERGY__TIMEX,
                          energyBins2, granTimeBins,
                          "Addback E - Time multineutron gated",
                          2, timeResolution, "s");

    DeclareHistogram1D(multiNeutron::betaGated::D_ENERGY, energyBins1,
                       "Gamma singles beta multineutron gated");
    DeclareHistogram1D(multiNeutron::betaGated::D_ENERGY_PROMPT, energyBins1,
                       "Gamma singles beta prompt multineutron gated");
    DeclareHistogram1D(multiNeutron::betaGated::D_ADD_ENERGY, energyBins1,
                       "Gamma addback beta multineutron gated");
    DeclareHistogram1D(multiNeutron::betaGated::D_ADD_ENERGY_TOTAL, energyBins1,
                       "Gamma total addback beta multiNeutron gated");
    DeclareHistogram2D(multiNeutron::betaGated::DD_ENERGY,
                       energyBins2, energyBins2,
                       "Gamma gamma beta multineutron gated");
    DeclareHistogram2D(multiNeutron::betaGated::DD_ADD_ENERGY,
                       energyBins2, energyBins2,
                      "Gamma gamma addback beta multineutron gated");
    DeclareHistogramGranY(multiNeutron::betaGated::DD_ENERGY__TIMEX,
                          energyBins2, granTimeBins,
                          "E - Time beta multineutron gated", 2,
                          timeResolution, "s");
    DeclareHistogramGranY(multiNeutron::betaGated::DD_ADD_ENERGY__TIMEX,
                          energyBins2, granTimeBins,
                          "Addback E - Time beta multineutron gated",
                          2, timeResolution, "s");

}

bool Ge4Hen3Processor::Process(RawEvent &event) {
    using namespace dammIds::ge;

    if (!EventProcessor::Process(event))
        return false;

    // Call base class processing
    GeProcessor::Process(event);

    double clockInSeconds = Globals::get()->clockInSeconds();

    /** Place Cycle is activated by BeamOn event and deactivated by TapeMove
     *  This will therefore skip events after tape was moved and before 
     *  beam hit the new spot
     */
    if (!TreeCorrelator::get()->place("Cycle")->status())
        return true;

    /** Cycle time is measured from the begining of the last BeamON event */
    double cycleTime = TreeCorrelator::get()->place("Cycle")->last().time;
    // beamOn is true for beam on and false for beam off
    bool beamOn =  TreeCorrelator::get()->place("Beam")->status();

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

    plot(neutron::D_MULT, geEvents_.size());

    /* Beta places are activated with threshold in ScintProcessor. */
    bool hasBeta = TreeCorrelator::get()->place("Beta")->status();

    for (vector<ChanEvent*>::iterator it1 = geEvents_.begin(); 
	 it1 != geEvents_.end(); ++it1) {
        ChanEvent *chan = *it1;
        
        double gEnergy = chan->GetCalEnergy();	
        double gTime   = chan->GetCorrectedTime();
        double decayTime = (gTime - cycleTime) * clockInSeconds;
        if (gEnergy < gammaThreshold_)
            continue;

        plot(neutron::D_ENERGY, gEnergy);
        granploty(neutron::DD_ENERGY__TIMEX,
                    gEnergy, decayTime, timeResolution);

        if (beamOn) {
            granploty(neutron::DD_ENERGY__TIMEX_GROW, 
                    gEnergy, decayTime, timeResolution);
        } else {
            double decayTimeOff = (gTime - 
                    TreeCorrelator::get()->place("Beam")->last().time) *
                    clockInSeconds;
            granploty(neutron::DD_ENERGY__TIMEX_DECAY, 
                    gEnergy, decayTimeOff, timeResolution);
        }

        double gb_dtime = numeric_limits<double>::max();
        if (hasBeta) {
            EventData bestBeta = BestBetaForGamma(gTime);
            gb_dtime = (gTime - bestBeta.time) * clockInSeconds;
            plot(neutron::betaGated::D_ENERGY, gEnergy);
			if (GoodGammaBeta(gb_dtime)) {
                plot(neutron::betaGated::D_ENERGY_PROMPT, gEnergy);
                granploty(neutron::betaGated::DD_ENERGY__TIMEX,
                            gEnergy, decayTime, timeResolution);
            }
		}

        /** Take detailed info on selected energy events */
        /* Commented in the master version
        vector< pair<double, double> > gates;
        gates.push_back(pair<double, double>(99.0, 102.0));
        gates.push_back(pair<double, double>(106.0, 109.0));
        gates.push_back(pair<double, double>(248.0, 253.0));
        gates.push_back(pair<double, double>(621.0, 626.0));
        gates.push_back(pair<double, double>(773.0, 776.0));
        for (vector< pair<double, double> >::iterator itp = gates.begin();
             itp != gates.end(); ++itp) {

            if (gEnergy < itp->first || gEnergy > itp->second)
                continue;

            ofstream gatedata("gatedata.txt", ios::app);
            double nTime = (gTime - 
                      TreeCorrelator::get()->place("Neutrons")->last().time) *
                      clockInSeconds;
            gatedata << gEnergy << " " << decayTime << " "  
                     << hasBeta << " " << GoodGammaBeta(gb_dtime) << " " 
                     << gb_dtime << " " << nTime;
            if (!beamOn) {
                double decayTimeOff = (gTime - 
                        TreeCorrelator::get()->place("Beam")->last().time) *
                        clockInSeconds;
                gatedata << " " << decayTimeOff;
            } else {
                gatedata << " " << beamOn;
            }
            gatedata << endl;
            gatedata.close();
        }
        */
        /*END*/

        for (unsigned l = 0; l < 48; ++l) {
            stringstream neutron;
            neutron << "Neutron_" << l;
            if (TreeCorrelator::get()->place(neutron.str())->status()) {
                plot(neutron::DD_ENERGY_NEUTRON_LOC, gEnergy, l);
            }
        }

        if (neutron_count > 1) {
            plot(multiNeutron::D_ENERGY, gEnergy);
            granploty(multiNeutron::DD_ENERGY__TIMEX,
                      gEnergy, decayTime, timeResolution);
            if (hasBeta) {
                plot(multiNeutron::betaGated::D_ENERGY, gEnergy);
                if (GoodGammaBeta(gb_dtime)) {
                    plot(multiNeutron::betaGated::D_ENERGY_PROMPT, gEnergy);
                    granploty(multiNeutron::betaGated::DD_ENERGY__TIMEX,
                            gEnergy, decayTime, timeResolution);
                }
            }
        }
        
        for (vector<ChanEvent*>::const_iterator it2 = it1 + 1;
                it2 != geEvents_.end(); ++it2) {

            ChanEvent *chan2 = *it2;

            double gEnergy2 = chan2->GetCalEnergy();            
            if (gEnergy2 < gammaThreshold_) 
                continue;

            symplot(neutron::DD_ENERGY, gEnergy, gEnergy2);            
            if (hasBeta && GoodGammaBeta(gb_dtime)) {
                symplot(neutron::betaGated::DD_ENERGY,
                        gEnergy, gEnergy2);            
            }
            if (neutron_count > 1) {
                symplot(multiNeutron::DD_ENERGY, gEnergy, gEnergy2);            
                if (hasBeta && GoodGammaBeta(gb_dtime)) {
                    symplot(multiNeutron::betaGated::DD_ENERGY,
                            gEnergy, gEnergy2);            
                }
            }
        }
    } 

    unsigned nEvents = tas_.size();

    for (unsigned i = 0; i < nEvents; ++i) {
        double gEnergy = tas_[i].first;
        double gTime = tas_[i].second;
        if (gEnergy < gammaThreshold_)
            continue;

        plot(neutron::D_ADD_ENERGY_TOTAL, gEnergy);
        double gb_dtime = numeric_limits<double>::max();
        if (hasBeta) {
            EventData bestBeta = BestBetaForGamma(gTime);
            gb_dtime = (gTime - bestBeta.time) * clockInSeconds;
        }

        if (hasBeta && GoodGammaBeta(gb_dtime)) {
            plot(neutron::betaGated::D_ADD_ENERGY_TOTAL, gEnergy);
        }

        if (neutron_count > 1) {
            plot(multiNeutron::D_ADD_ENERGY_TOTAL, gEnergy);
            if (hasBeta && GoodGammaBeta(gb_dtime)) {
                plot(multiNeutron::betaGated::D_ADD_ENERGY_TOTAL, gEnergy);
            }
        }
    }

    for (unsigned int ev = 0; ev < nEvents; ev++) {
        for (unsigned int det = 0; det < numClovers; ++det) {
            double gEnergy = addbackEvents_[det][ev].first;
            double gTime = addbackEvents_[det][ev].second;
            double decayTime = (gTime - cycleTime) * clockInSeconds;
            if (gEnergy < gammaThreshold_)
                continue;

            plot(neutron::D_ADD_ENERGY, gEnergy);
            granploty(neutron::DD_ADD_ENERGY__TIMEX, gEnergy,
                      decayTime, timeResolution);

            double gb_dtime = numeric_limits<double>::max();
            if (hasBeta) {
                EventData bestBeta = BestBetaForGamma(gTime);
                gb_dtime = (gTime - bestBeta.time) * clockInSeconds;
            }

            if (hasBeta && GoodGammaBeta(gb_dtime)) {
                plot(neutron::betaGated::D_ADD_ENERGY, gEnergy);
                granploty(neutron::betaGated::DD_ADD_ENERGY__TIMEX,
                          gEnergy, decayTime, timeResolution);
            }
            if (neutron_count > 1) {
                plot(multiNeutron::D_ADD_ENERGY, gEnergy);
                granploty(multiNeutron::DD_ADD_ENERGY__TIMEX,
                          gEnergy, decayTime, timeResolution);
                if (hasBeta && GoodGammaBeta(gb_dtime)) {
                    plot(multiNeutron::betaGated::D_ADD_ENERGY, gEnergy);
                    granploty(multiNeutron::betaGated::DD_ADD_ENERGY__TIMEX,
                              gEnergy, decayTime, timeResolution);
                }
            }

            for (unsigned int det2 = det + 1; det2 < numClovers; ++det2) {
                double gEnergy2 = addbackEvents_[det2][ev].first;
                if (gEnergy2 < gammaThreshold_)
                    continue;

                symplot(neutron::DD_ADD_ENERGY, gEnergy, gEnergy2);
                if (hasBeta && GoodGammaBeta(gb_dtime)) {
                    symplot(neutron::betaGated::DD_ADD_ENERGY, gEnergy,
                            gEnergy2);
                }
                if (neutron_count > 1) {
                    symplot(multiNeutron::DD_ADD_ENERGY, gEnergy, gEnergy2);
                    if (hasBeta && GoodGammaBeta(gb_dtime)) {
                        symplot(multiNeutron::betaGated::DD_ADD_ENERGY,
                                gEnergy, gEnergy2);
                    }
                }
            } // iteration over other clovers
        } // iteration over clovers
    } // iteration over events

    EndProcess(); 
    return true;
}
