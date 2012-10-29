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
#include "TreeCorrelator.hpp"

using namespace std;

namespace dammIds {
    namespace ge {
        namespace neutron {
            const int D_ENERGY = 200;
            const int D_ADD_ENERGY = 250; 
            const int DD_ENERGY = 201;
            const int DD_ADD_ENERGY = 251;
            const int D_ADD_ENERGY_TOTAL = 255;
            const int DD_ENERGY__TIMEX = 221; 
            const int DD_ADD_ENERGY__TIMEX = 271;

            namespace betaGated {
                const int D_ENERGY = 210;
                const int D_ADD_ENERGY = 260; 
                const int DD_ENERGY = 211;
                const int DD_ADD_ENERGY = 261;
                const int D_ADD_ENERGY_TOTAL = 265;
                const int DD_ENERGY__TIMEX = 231; 
                const int DD_ADD_ENERGY__TIMEX = 281;
            } 
        }

        namespace multiNeutron {
            const int D_ENERGY = 300;
            const int D_ADD_ENERGY = 350; 
            const int DD_ENERGY = 301;
            const int DD_ADD_ENERGY = 351;
            const int D_ADD_ENERGY_TOTAL = 355;
            const int DD_ENERGY__TIMEX = 321; 
            const int DD_ADD_ENERGY__TIMEX = 371;
            namespace betaGated {
                const int D_ENERGY = 310;
                const int D_ADD_ENERGY = 360; 
                const int DD_ENERGY = 311;
                const int DD_ADD_ENERGY = 361;
                const int D_ADD_ENERGY_TOTAL = 365;
                const int DD_ENERGY__TIMEX = 331; 
                const int DD_ADD_ENERGY__TIMEX = 381; 
            } 
        }
    }
}

Ge4Hen3Processor::Ge4Hen3Processor() : GeProcessor() {
}

/** Declare plots including many for decay/implant/neutron gated analysis  */
void Ge4Hen3Processor::DeclarePlots(void) 
{
    GeProcessor::DeclarePlots();
    using namespace dammIds::ge;

    const int energyBins1  = SE;
    const int energyBins2  = SC;
    const int granTimeBins = SA;

    DeclareHistogram1D(neutron::D_ENERGY, energyBins1, "Gamma singles neutron gated");
    DeclareHistogram1D(neutron::D_ADD_ENERGY, energyBins1, "Gamma addback neutron gated");
    DeclareHistogram1D(neutron::D_ADD_ENERGY_TOTAL, energyBins1,
                       "Gamma total addback neutron gated");
    DeclareHistogram2D(neutron::DD_ENERGY, energyBins2, energyBins2, "Gamma gamma neutron gated");
    DeclareHistogram2D(neutron::DD_ADD_ENERGY, energyBins2, energyBins2,
                       "Gamma gamma addback neutron gated");
    DeclareHistogramGranY(neutron::DD_ENERGY__TIMEX, energyBins2, granTimeBins,
                          "E - Time neutron gated", 2, timeResolution, "s");
    DeclareHistogramGranY(neutron::DD_ADD_ENERGY__TIMEX, energyBins2, granTimeBins,
                          "Addback E - Time", 2, timeResolution, "s");

    DeclareHistogram1D(neutron::betaGated::D_ENERGY, energyBins1,
                       "Gamma singles beta neutron gated");
    DeclareHistogram1D(neutron::betaGated::D_ADD_ENERGY, energyBins1,
                       "Gamma addback beta neutron gated");
    DeclareHistogram1D(neutron::betaGated::D_ADD_ENERGY_TOTAL, energyBins1,
                       "Gamma total addback beta neutron gated");
    DeclareHistogram2D(neutron::betaGated::DD_ENERGY, energyBins2, energyBins2,
                       "Gamma gamma beta neutron gated");
    DeclareHistogram2D(neutron::betaGated::DD_ADD_ENERGY, energyBins2, energyBins2,
                       "Gamma gamma addback beta neutron gated");
    DeclareHistogramGranY(neutron::betaGated::DD_ENERGY__TIMEX, energyBins2, granTimeBins,
                          "E - Time beta neutron gated", 2, timeResolution, "s");
    DeclareHistogramGranY(neutron::betaGated::DD_ADD_ENERGY__TIMEX, energyBins2, granTimeBins,
                          "Addback E - Time beta neutron gated", 2, timeResolution, "s");

    DeclareHistogram1D(multiNeutron::D_ENERGY, energyBins1, "Gamma singles multiNeutron gated");
    DeclareHistogram1D(multiNeutron::D_ADD_ENERGY, energyBins1, "Gamma addback multiNeutron gated");
    DeclareHistogram1D(multiNeutron::D_ADD_ENERGY_TOTAL, energyBins1,
                       "Gamma total addback  multiNeutron gated");
    DeclareHistogram2D(multiNeutron::DD_ENERGY, energyBins2, energyBins2,
                       "Gamma gamma multineutron gated");
    DeclareHistogram2D(multiNeutron::DD_ADD_ENERGY, energyBins2, energyBins2,
                       "Gamma gamma addback multineutron gated");
    DeclareHistogramGranY(multiNeutron::DD_ENERGY__TIMEX, energyBins2, granTimeBins,
                          "E - Time multineutron gated", 2, timeResolution, "s");
    DeclareHistogramGranY(multiNeutron::DD_ADD_ENERGY__TIMEX, energyBins2, granTimeBins,
                          "Addback E - Time multineutron gated", 2, timeResolution, "s");

    DeclareHistogram1D(multiNeutron::betaGated::D_ENERGY, energyBins1,
                       "Gamma singles beta multineutron gated");
    DeclareHistogram1D(multiNeutron::betaGated::D_ADD_ENERGY, energyBins1,
                       "Gamma addback beta multineutron gated");
    DeclareHistogram1D(multiNeutron::betaGated::D_ADD_ENERGY_TOTAL, energyBins1,
                       "Gamma total addback beta multiNeutron gated");
    DeclareHistogram2D(multiNeutron::betaGated::DD_ENERGY, energyBins2, energyBins2,
                       "Gamma gamma beta multineutron gated");
    DeclareHistogram2D(multiNeutron::betaGated::DD_ADD_ENERGY, energyBins2, energyBins2,
                      "Gamma gamma addback beta multineutron gated");
    DeclareHistogramGranY(multiNeutron::betaGated::DD_ENERGY__TIMEX, energyBins2, granTimeBins,
                          "E - Time beta multineutron gated", 2, timeResolution, "s");
    DeclareHistogramGranY(multiNeutron::betaGated::DD_ADD_ENERGY__TIMEX, energyBins2, granTimeBins,
                          "Addback E - Time beta multineutron gated", 2, timeResolution, "s");
}

bool Ge4Hen3Processor::Process(RawEvent &event) {
    using namespace dammIds::ge;

    if (!EventProcessor::Process(event))
        return false;

    /* tapeMove is true if the tape is moving */
    bool tapeMove = TreeCorrelator::get().places["TapeMove"]->status();

    /** If the tape is moving there is no need of analyzing events
     *  as they are background.
     */
    if (tapeMove)
        return true;

    /* beamOn is true for beam on tape */
    bool beamOn =  TreeCorrelator::get().places["Beam"]->status();

    /* Beta places are activated with threshold in ScintProcessor. */
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

    /* Number of neutrons as selected by gates on 3hen spectrum.
     * See DetectorDriver::InitCorrelator for gates. */
    int neutron_count = dynamic_cast<PlaceCounter*>(TreeCorrelator::get().places["Neutrons"])->getCounter();

    /* Cycle time is measured from the begining of last beam on event.*/
    double cycleTime = TreeCorrelator::get().places["Cycle"]->last().time;

    /** Note that geEvents_ vector holds only good events (matched
     *  low & high gain). See PreProcess.
     *
     * Simple spectra (single crystals) --
     * WE CAN MAKE PERMANENT CHANGES TO EVENTS!
     * (time is being corrected for walk)
     * Necessary in order to set corrected time for use in other processors
     */
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

        if (neutron_count == 1) {
            plot(neutron::D_ENERGY, gEnergy);
            granploty(neutron::DD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
            if (hasBeta) {
                plot(neutron::betaGated::D_ENERGY, gEnergy);
                granploty(neutron::betaGated::DD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
            }
        } else if (neutron_count > 1) {
            plot(multiNeutron::D_ENERGY, gEnergy);
            granploty(multiNeutron::DD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
            if (hasBeta) {
                plot(multiNeutron::betaGated::D_ENERGY, gEnergy);
                granploty(multiNeutron::betaGated::DD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
            }
        }

        if (hasBeta) {
            plot(betaGated::D_ENERGY, gEnergy);
            granploty(betaGated::DD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);

            int dtime=(int)(gTime - betaTime + 100);
            plot(betaGated::DD_TDIFF__GAMMA_ENERGY, dtime, gEnergy);
            plot(betaGated::DD_TDIFF__BETA_ENERGY, dtime, betaEnergy);

            /* individual beta gamma coinc spectra for each beta detector*/
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

            if (neutron_count == 1) {
                symplot(neutron::DD_ENERGY, gEnergy, gEnergy2);            
                if (hasBeta) {
                    symplot(neutron::betaGated::DD_ENERGY, gEnergy, gEnergy2);            
                }
            } else if (neutron_count > 1) {
                symplot(multiNeutron::DD_ENERGY, gEnergy, gEnergy2);            
                if (hasBeta) {
                    symplot(multiNeutron::betaGated::DD_ENERGY, gEnergy, gEnergy2);            
                }
            }
        }
    } 
    
    /* now we sort the germanium events according to their corrected time */
    sort(geEvents_.begin(), geEvents_.end(), CompareCorrectedTime);

    /** addbackEvents vector is arranged as pair where
     *  first -> energy, second -> time
     */
    vector< pair<double, double> > addbackEvents[numClovers];

    /* "TAS" vector for total energy absorbed */
    vector< pair<double, double> > tas;

    /** Give value in seconds  */
    double subEventWindow = 200 * 1e-9;

    subEventWindow = subEventWindow / pixie::clockInSeconds; 

    /** guarantee the first event will be greater than the subevent window delayed from reference */
    double refTime = -2.0 * subEventWindow; 

    /** Do not take into account events with too low energy (avoid summing of noise with real gammas) */
    const double addbackEnergyCut = 25;
    
    for (vector<ChanEvent*>::iterator it = geEvents_.begin(); it != geEvents_.end(); it++) {
	ChanEvent *ch = *it;
        double energy = ch->GetCalEnergy(); 
        double time = ch->GetCorrectedTime();
        int clover = leafToClover[ch->GetChanID().GetLocation()];

        if (energy < addbackEnergyCut)
            continue;

        /** entries in geEvents_ are sorted by time
         * if current event time is outside of subEventWindow, we start new 
         * events for all clovers and "tas"
         */
        if (abs(time - refTime) > subEventWindow) {
            for (unsigned i = 0; i < numClovers; ++i) {
                addbackEvents[i].push_back(pair<double, double>());
            }
            tas.push_back(pair<double, double>());
        }
        /* Total addback energy */
        addbackEvents[clover].back().first += energy;
        /* We store latest time only */
        addbackEvents[clover].back().second = time;
        tas.back().first += energy;
        tas.back().second = time;
        refTime = time;
    }

    unsigned nTas = tas.size();
    for (unsigned i = 0; i < nTas; ++i) {
        double gEnergy = tas[i].first;
        if (gEnergy < 1) 
            continue;
        plot(D_ADD_ENERGY_TOTAL, gEnergy);
        if (hasBeta)
            plot(betaGated::D_ADD_ENERGY_TOTAL, gEnergy);
        if (neutron_count == 1) {
            plot(neutron::D_ADD_ENERGY_TOTAL, gEnergy);
            if (hasBeta) {
                plot(neutron::betaGated::D_ADD_ENERGY_TOTAL, gEnergy);
            }
        } else if (neutron_count > 1) {
            plot(multiNeutron::D_ADD_ENERGY_TOTAL, gEnergy);
            if (hasBeta) {
                plot(multiNeutron::betaGated::D_ADD_ENERGY_TOTAL, gEnergy);
            }
        }
    }

    for (unsigned int ev = 0; ev < tas.size(); ev++) {
        for (unsigned int det = 0; det < numClovers; ++det) {
            double gEnergy = addbackEvents[det][ev].first;
            double gTime = addbackEvents[det][ev].second;
            double decayTime = (gTime - cycleTime) * pixie::clockInSeconds;
            if (gEnergy < 1) 
                continue;

            plot(D_ADD_ENERGY, gEnergy);
            plot(D_ADD_ENERGY_CLOVERX + det, gEnergy);
            granploty(DD_ADD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
            if (neutron_count == 1) {
                plot(neutron::D_ADD_ENERGY, gEnergy);
                granploty(neutron::DD_ADD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
                if (hasBeta) {
                    plot(neutron::betaGated::D_ADD_ENERGY, gEnergy);
                    granploty(neutron::betaGated::DD_ADD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
                }
            } else if (neutron_count > 1) {
                plot(multiNeutron::D_ADD_ENERGY, gEnergy);
                granploty(multiNeutron::DD_ADD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
                if (hasBeta) {
                    plot(multiNeutron::betaGated::D_ADD_ENERGY, gEnergy);
                    granploty(multiNeutron::betaGated::DD_ADD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
                }
            }

            if(hasBeta) {
                plot(betaGated::D_ADD_ENERGY, gEnergy);
                plot(betaGated::D_ADD_ENERGY_CLOVERX + det, gEnergy);
                granploty(betaGated::DD_ADD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
            }

            for (unsigned int det2 = det + 1; 
                    det2 < numClovers; ++det2) {
                double gEnergy2 = addbackEvents[det2][ev].first;
                if (gEnergy2 < 1) 
                    continue;

                symplot(DD_ADD_ENERGY, gEnergy, gEnergy2);
                if (neutron_count == 1) {
                    symplot(neutron::DD_ADD_ENERGY, gEnergy, gEnergy2);
                    if (hasBeta) {
                        symplot(neutron::betaGated::DD_ADD_ENERGY, gEnergy, gEnergy2);
                    }
                } else if (neutron_count > 1) {
                    symplot(multiNeutron::DD_ADD_ENERGY, gEnergy, gEnergy2);
                    if (hasBeta) {
                        symplot(multiNeutron::betaGated::DD_ADD_ENERGY, gEnergy, gEnergy2);
                    }
                }
                /*
                * Early and late coincidences
                * Only decay part of cycle can be taken
                * Early coin. are between decay cycle start and some arb. point
                * Late are between arb. point and end of cycle
                */
                if (!beamOn) {
                    double decayCycleEarly = 0.5;
                    double decayCycleEnd   = 1.0;
                    // Beam fifo should be updated upon beam off event so
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
                    
                    /** Arbitrary chosen limits for prompt g-g
                     * Compare Gamma-Beta time diff spectrum
                     * to pick good numbers
                     * compare both gamma times */
                    const double promptLimit = 14.0;
                    const double promptOnlyLimit = -8.0;

                    if (dTime > promptLimit) {
                        symplot(betaGated::DD_ADD_ENERGY_DELAYED, gEnergy, gEnergy2);
                    } else if (dTime > promptOnlyLimit) {
                        symplot(betaGated::DD_ADD_ENERGY_PROMPT, gEnergy, gEnergy2);
                    }
                }
            } // iteration over other clovers
        } // iteration over clovers
    } // iteration over events

    geEvents_.clear();
    EndProcess(); 
    return true;
}
