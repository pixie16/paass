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

    // Call base class processing
    GeProcessor::Process(event);

    /* tapeMove is true if the tape is moving */
    bool tapeMove = TreeCorrelator::get()->place("TapeMove")->status();

    /** If the tape is moving there is no need of analyzing events
     *  as they are background.
     */
    if (tapeMove)
        return true;

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

    /* Beta places are activated with threshold in ScintProcessor. */
    bool hasBeta = TreeCorrelator::get()->place("Beta")->status();
    /* Cycle time is measured from the begining of last beam on event.*/
    double cycleTime = TreeCorrelator::get()->place("Cycle")->last().time;

    for (vector<ChanEvent*>::iterator it = geEvents_.begin(); 
	 it != geEvents_.end(); it++) {
        ChanEvent *chan = *it;
        
        double gEnergy = chan->GetCalEnergy();	
        double gTime   = chan->GetCorrectedTime();
        double decayTime = (gTime - cycleTime) * pixie::clockInSeconds;
        if (gEnergy < 1) 
            continue;

        if (neutron_count == 1) {
            plot(neutron::D_ENERGY, gEnergy);
            granploty(neutron::DD_ENERGY__TIMEX,
                      gEnergy, decayTime, timeResolution);
            if (hasBeta && GoodGammaBeta(gTime)) {
                plot(neutron::betaGated::D_ENERGY, gEnergy);
                granploty(neutron::betaGated::DD_ENERGY__TIMEX,
                          gEnergy, decayTime, timeResolution);
            }
        } else if (neutron_count > 1) {
            plot(multiNeutron::D_ENERGY, gEnergy);
            granploty(multiNeutron::DD_ENERGY__TIMEX,
                      gEnergy, decayTime, timeResolution);
            if (hasBeta && GoodGammaBeta(gTime)) {
                plot(multiNeutron::betaGated::D_ENERGY, gEnergy);
                granploty(multiNeutron::betaGated::DD_ENERGY__TIMEX,
                          gEnergy, decayTime, timeResolution);
            }
        }
        
        for (vector<ChanEvent*>::const_iterator it2 = it + 1;
                it2 != geEvents_.end(); it2++) {

            ChanEvent *chan2 = *it2;

            double gEnergy2 = chan2->GetCalEnergy();            
            if (gEnergy2 < 1) 
                continue;

            if (neutron_count == 1) {
                symplot(neutron::DD_ENERGY, gEnergy, gEnergy2);            
                if (hasBeta && GoodGammaBeta(gTime)) {
                    symplot(neutron::betaGated::DD_ENERGY,
                            gEnergy, gEnergy2);            
                }
            } else if (neutron_count > 1) {
                symplot(multiNeutron::DD_ENERGY, gEnergy, gEnergy2);            
                if (hasBeta && GoodGammaBeta(gTime)) {
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
        if (gEnergy < 1)
            continue;

        if (neutron_count == 1) {
            plot(neutron::D_ADD_ENERGY_TOTAL, gEnergy);
            if (hasBeta && GoodGammaBeta(gTime)) {
                plot(neutron::betaGated::D_ADD_ENERGY_TOTAL, gEnergy);
            }
        } else if (neutron_count > 1) {
            plot(multiNeutron::D_ADD_ENERGY_TOTAL, gEnergy);
            if (hasBeta && GoodGammaBeta(gTime)) {
                plot(multiNeutron::betaGated::D_ADD_ENERGY_TOTAL, gEnergy);
            }
        }
    }

    for (unsigned int ev = 0; ev < nEvents; ev++) {
        for (unsigned int det = 0; det < numClovers; ++det) {
            double gEnergy = addbackEvents_[det][ev].first;
            double gTime = addbackEvents_[det][ev].second;
            double decayTime = (gTime - cycleTime) * pixie::clockInSeconds;
            if (gEnergy < 1)
                continue;

            if (neutron_count == 1) {
                plot(neutron::D_ADD_ENERGY, gEnergy);
                granploty(neutron::DD_ADD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
                if (hasBeta && GoodGammaBeta(gTime)) {
                    plot(neutron::betaGated::D_ADD_ENERGY, gEnergy);
                    granploty(neutron::betaGated::DD_ADD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
                }
            } else if (neutron_count > 1) {
                plot(multiNeutron::D_ADD_ENERGY, gEnergy);
                granploty(multiNeutron::DD_ADD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
                if (hasBeta && GoodGammaBeta(gTime)) {
                    plot(multiNeutron::betaGated::D_ADD_ENERGY, gEnergy);
                    granploty(multiNeutron::betaGated::DD_ADD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
                }
            }

            for (unsigned int det2 = det + 1; det2 < numClovers; ++det2) {
                double gEnergy2 = addbackEvents_[det2][ev].first;
                if (gEnergy2 < 1)
                    continue;

                if (neutron_count == 1) {
                    symplot(neutron::DD_ADD_ENERGY, gEnergy, gEnergy2);
                    if (hasBeta && GoodGammaBeta(gTime)) {
                        symplot(neutron::betaGated::DD_ADD_ENERGY, gEnergy, gEnergy2);
                    }
                } else if (neutron_count > 1) {
                    symplot(multiNeutron::DD_ADD_ENERGY, gEnergy, gEnergy2);
                    if (hasBeta && GoodGammaBeta(gTime)) {
                        symplot(multiNeutron::betaGated::DD_ADD_ENERGY, gEnergy, gEnergy2);
                    }
                }
            } // iteration over other clovers
        } // iteration over clovers
    } // iteration over events

    EndProcess(); 
    return true;
}
