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

#include <climits>
#include <cmath>
#include <cstdlib>


#include "Plots.hpp"
#include "PlotsRegister.hpp"
#include "DammPlotIds.hpp"

#include "AliasedPair.hpp"
#include "Correlator.hpp"
#include "DetectorDriver.hpp"
#include "DetectorLibrary.hpp"
#include "GeProcessor.hpp"
#include "Ge4Hen3Processor.hpp"
#include "RawEvent.hpp"

using namespace std;

namespace dammIds {
    namespace ge {
        /*
        * Beta offset = 10
        * Decay offset = 20
        * Addback offset = 50
        * Neutron offest = 100
        */

        // corresponds to ungated specra ID's + 10 where applicable
        namespace betaGated {
        } 

        namespace neutronGated {
        }
    }
}

Ge4Hen3Processor::Ge4Hen3Processor() : GeProcessor() {
}

/** Declare plots including many for decay/implant/neutron gated analysis  */
void Ge4Hen3Processor::DeclarePlots(void) 
{
    GeProcessor::DeclarePlots();

    const int energyBins1  = SE;
    const int energyBins2  = SC;
    const int timeBins2    = SA;
    const int granTimeBins = SA;

    using namespace dammIds::ge;

}

bool Ge4Hen3Processor::Process(RawEvent &event) {
    using namespace dammIds::ge;

    if (!EventProcessor::Process(event))
        return false;

    // tapeMove is true if the tape is moving
    bool tapeMove = TreeCorrelator::get().places["TapeMove"]->status();

    // If the tape is moving there is no need of analyzing events
    // as they must belong to background
    if (tapeMove)
        return true;

    // beamOn is true for beam on and false for beam off
    bool beamOn =  TreeCorrelator::get().places["Beam"]->status();

    //Beta places are activated with threshold in ScintProcessor
    bool hasBeta = TreeCorrelator::get().places["Beta"]->status();
    bool hasBeta0 = TreeCorrelator::get().places["beta_scint_beta_0"]->status();
    bool hasBeta1 = TreeCorrelator::get().places["beta_scint_beta_1"]->status();
    double betaTime = -1;
    double betaEnergy = -1;
    if (hasBeta) {
        double betaTime0 = -1;
        double betaEnergy0 = -1;
        if (hasBeta0) {
            betaTime0 = TreeCorrelator::get().places["beta_scint_beta_0"]->last().time;
            betaEnergy0 = TreeCorrelator::get().places["beta_scint_beta_0"]->last().energy;
        }
        double betaTime1 = -1;
        double betaEnergy1 = -1;
        if (hasBeta1) {
            betaTime1 = TreeCorrelator::get().places["beta_scint_beta_1"]->last().time;
            betaEnergy1 = TreeCorrelator::get().places["beta_scint_beta_1"]->last().energy;
        }
        betaTime = max(betaTime0, betaTime1);
        betaEnergy = max(betaEnergy0, betaEnergy1);
    }

    // Cycle time is measured from the begining of last beam on event
    double cycleTime = TreeCorrelator::get().places["Cycle"]->last().time;

    // Note that geEvents_ vector holds only good events (matched
    // low & high gain). See PreProcess

    // Simple spectra (single crystals) -- NOTE WE CAN MAKE PERMANENT CHANGES TO EVENTS
    //   Necessary in order to set corrected time for use in other processors
    for (vector<ChanEvent*>::iterator it = geEvents_.begin(); 
	 it != geEvents_.end(); it++) {
        ChanEvent *chan = *it;
        
        double gEnergy = chan->GetCalEnergy();	
        double gTime   = chan->GetTime() - WalkCorrection(gEnergy);	
        chan->SetCorrectedTime(gTime);
        double decayTime = (gTime - cycleTime) 
                           * pixie::clockInSeconds;

        plot(D_ENERGY, gEnergy);
        granploty(DD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);

        if (hasBeta) {
            plot(betaGated::D_ENERGY, gEnergy);
            granploty(betaGated::DD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);

            int dtime=(int)(gTime - betaTime + 100);
            plot(betaGated::DD_TDIFF__GAMMA_ENERGY, dtime, gEnergy);
            plot(betaGated::DD_TDIFF__BETA_ENERGY, dtime, betaEnergy);

            // individual beta gamma coinc spectra for each beta detector
            if (hasBeta0 == 0) {
                plot(betaGated::D_ENERGY_BETA0, gEnergy);
            } else if (hasBeta1 == 1) {
                plot(betaGated::D_ENERGY_BETA1, gEnergy);
            }
        }
        
        for (vector<ChanEvent*>::const_iterator it2 = it + 1;
                it2 != geEvents_.end(); it2++) {
            ChanEvent *chan2 = *it2;

            double gEnergy2 = chan2->GetCalEnergy();            
            symplot(DD_ENERGY, gEnergy, gEnergy2);
            if (hasBeta)
                symplot(betaGated::DD_ENERGY, gEnergy, gEnergy2);            
        } // iteration over other gammas
    } 
    
    // now we sort the germanium events according to their corrected time
    sort(geEvents_.begin(), geEvents_.end(), CompareCorrectedTime);

    // addbackEvents vector is arranged as:
    // first -> energy, second -> time
    vector< pair<double, double> > addbackEvents[numClovers];
    // tas vector for total energy absorbed
    vector< pair<double, double> > tas;

    double subEventWindow = 10.0; // pixie units
    // guarantee the first event will be greater than the subevent window delayed from reference
    double refTime = -2.0 * subEventWindow; 

    // Do not take into account events with too low energy (avoid summing of noise with real gammas) 
    const double addbackEnergyCut = 25;
    
    for (vector<ChanEvent*>::iterator it = geEvents_.begin(); it != geEvents_.end(); it++) {
	ChanEvent *ch = *it;
        double energy = ch->GetCalEnergy(); 
        double time = ch->GetCorrectedTime();
        int clover = leafToClover[ch->GetChanID().GetLocation()];

        if (energy < addbackEnergyCut)
            continue;

        // entries in map are sorted by time
        // if event time is outside of subEventWindow, we start new 
        //   events for all clovers and "tas"
        if (abs(time - refTime) > subEventWindow) {
            for (unsigned i = 0; i < numClovers; ++i) {
                addbackEvents[i].push_back(pair<double, double>());
            }
            tas.push_back(pair<double, double>());
        }
        // Total addback energy
        addbackEvents[clover].back().first += energy;
        // We store latest time only
        addbackEvents[clover].back().second   = time;
        tas.back().first += energy;
        tas.back().second   = time;
        refTime = time;
    }

    // Plot 'tas' spectra
    unsigned nTas = tas.size();
    for (unsigned i = 0; i < nTas; ++i) {
        double gEnergy = tas[i].first;
        if (gEnergy < 1) 
            continue;
        plot(D_ADD_ENERGY_TOTAL, gEnergy);
        if(hasBeta)
            plot(betaGated::D_ADD_ENERGY_TOTAL, gEnergy);
    }

    // Plot addback spectra 
    // all vectors should have the same size
    for (unsigned int ev = 0; ev < tas.size(); ev++) {
        for (unsigned int det = 0; det < numClovers; ++det) {
            double gEnergy = addbackEvents[det][ev].first;
            double gTime = addbackEvents[det][ev].second;
            double decayTime = (gTime - cycleTime) * pixie::clockInSeconds;
            if (gEnergy < 1) 
                continue;

            plot(D_ADD_ENERGY, gEnergy);
            plot(D_ADD_ENERGY_CLOVERX + det, gEnergy);
            granploty(DD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);

            if(hasBeta) {
                plot(betaGated::D_ADD_ENERGY, gEnergy);
                plot(betaGated::D_ADD_ENERGY_CLOVERX + det, gEnergy);
                granploty(betaGated::DD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
            }

            for (unsigned int det2 = det + 1; 
                    det2 < numClovers; ++det2) {
            double gEnergy2 = addbackEvents[det2][ev].first;
            if (gEnergy2 < 1) 
                continue;

            symplot(DD_ADD_ENERGY, gEnergy, gEnergy2);
            /*
            * Early and late coincidences
            * Only decay part of cycle can be taken
            * Early coin. are between decay cycle start and some arb. point
            * Late are between arb. point and end of cycle
            */
            if (!beamOn) {
                double decayCycleEarly = 0.5;
                double decayCycleEnd   = 1.0;
                // Beam deque should be updated upon beam off so
                // measure time from that point
                double decayTime = (gTime - TreeCorrelator::get().places["Beam"]->last().time) * pixie::clockInSeconds;
                if (decayTime < decayCycleEarly) {
                    symplot(DD_ADD_ENERGY_EARLY, gEnergy, gEnergy2);
                    if (hasBeta)
                        symplot(betaGated::DD_ADD_ENERGY_EARLY, gEnergy, gEnergy2);
                } else if (decayTime < decayCycleEnd) {
                    symplot(DD_ADD_ENERGY_LATE, gEnergy, gEnergy2);
                    if (hasBeta)
                        symplot(betaGated::DD_ADD_ENERGY_LATE, gEnergy, gEnergy2);
                }
            }
            if (hasBeta) {
                symplot(betaGated::DD_ADD_ENERGY, gEnergy, gEnergy2);
                double dTime = (int)(gTime - betaTime);
                
                // Arbitrary chosen limits
                // Compare Gamma-Beta time diff spectrum
                // to pick good numbers
                //? compare both gamma times
                const double promptLimit = 14.0;
                const double promptOnlyLimit = -8.0;

                if (dTime > promptLimit) {
                    symplot(betaGated::DD_ADD_ENERGY_DELAYED, gEnergy, gEnergy2);
                } else if (dTime > promptOnlyLimit) {
                    symplot(betaGated::DD_ADD_ENERGY_PROMPT, gEnergy, gEnergy2);
                }
                    } // has beta
#ifdef GGATES
                // Gamma-gamma angular
                // and gamma-gamma gate
                unsigned ngg = gGates.size();
                for (unsigned ig = 0; ig < ngg; ++ig) {
                    double g1min = 0;
                    double g1max = 0;
                    double g2min = 0;
                    double g2max = 0;
                    if (gGates[ig].g1min < gGates[ig].g2min) {
                        g1min = gGates[ig].g1min;
                        g1max = gGates[ig].g1max;
                        g2min = gGates[ig].g2min;
                        g2max = gGates[ig].g2max;
                    } else {
                        g1min = gGates[ig].g2min;
                        g1max = gGates[ig].g2max;
                        g2min = gGates[ig].g1min;
                        g2max = gGates[ig].g1max;
                    }
                    double e1 = min(gEnergy, gEnergy2);
                    double e2 = max(gEnergy, gEnergy2);
                    if ( (e1 >= g1min && e1 <= g1max) &&
                            (e2 >= g2min && e2 <= g2max) ) {
                        if (det % 2 != det2 % 2) {
                            plot(DD_ANGLE__GATEX, 1, ig);
                            if (hasBeta)
                                plot(betaGated::DD_ANGLE__GATEX, 1, ig);
                        } else {
                        plot(DD_ANGLE__GATEX, 2, ig);
                            if (hasBeta)
                                plot(betaGated::DD_ANGLE__GATEX, 2, ig);
                        }
                        for (unsigned det3 = det2 + 1;
                                det3 < numClovers; ++det3) {
                            double gEnergy3 = addbackEvents[det3][ev].energy;
                            if (gEnergy3 < 1)
                                continue;
                            plot(DD_ENERGY__GATEX, gEnergy3, ig);
                            if (hasBeta)
                                plot(betaGated::DD_ENERGY__GATEX, gEnergy3, ig);
                        }
                    }
                } //for ig
#endif
            } // iteration over other clovers
        } // itertaion over clovers
    } // iteration over events

    // Clear all events stored in geEvents vector
    geEvents_.clear();
    EndProcess(); // update the processing time
    return true;
}
