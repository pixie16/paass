/** \file ScintProcessor.cpp
 *
 * implementation for scintillator processor
 *
 * KM 10/20/12:
 *
 * Obsolete file, kept in case of issues with new version
 * Now ScintProcessor is broken into BetaScint, NeutronScint
 * and LiquidScint Processors.
 *
 */
#include <vector>
#include <sstream>

#include <cmath>

#include "DammPlotIds.hpp"
#include "RawEvent.hpp"
#include "ScintProcessor.hpp"
#include "HighResTimingData.hpp"
#include "Trace.hpp"

using namespace std;
using namespace dammIds::scint;

namespace dammIds {
    namespace scint {
        namespace neutr {
            namespace betaGated {
                const int D_ENERGY_DETX = 0; //!< Beta Gated for 3 detectors (1-3)
            }
            namespace gammaGated {
                const int D_ENERGY_DETX = 3; //!< Gamma Gated for 3 detectors (1-3)
            }
            namespace betaGammaGated {
                const int D_ENERGY_DETX = 6; //!< BetaGamma Gated  for 3 detectors (1-3)
            }
        } // neutr namespace
        const int DD_TQDCBETA         = 9;//!< TQDCs for beta scints
        const int DD_MAXBETA          = 10;//!< Maximum for beta scints
        const int DD_TQDCLIQUID       = 11;//!< Trace QDC distribution for liquid
        const int DD_MAXLIQUID        = 12;//!< Max Value distributions for Liquid
        const int DD_DISCRIM          = 13;//!< N-G discrimination
        const int DD_TOFLIQUID        = 14;//!< ToF for Liquid Scints
        const int DD_TRCLIQUID        = 15;//!< Traces for Liquid Scints
        const int DD_TQDCVSDISCRIM    = 16;//!< Trace QDC vs. NG Discrimination
        const int DD_TOFVSDISCRIM     = 18;//!< ToF vs. Discrimination
        const int DD_NEVSDISCRIM      = 20;//!< Neutron energy vs. N-Gamma Discrimination
        const int DD_TQDCVSLIQTOF     = 22;//!< Trace QDC vs. Liquid TOF
        const int DD_TQDCVSENERGY     = 24;//!< Trace QDC vs. Energy
    }
}

ScintProcessor::ScintProcessor() : EventProcessor(OFFSET, RANGE, "ScintProcessor") {
    associatedTypes.insert("scint");
}

void ScintProcessor::DeclarePlots(void) {
    {
        using namespace neutr::betaGated;
        DeclareHistogram1D(D_ENERGY_DETX + 0, SE, "beta gated scint1 sig1");
        DeclareHistogram1D(D_ENERGY_DETX + 1, SE, "beta gated scint1 sig2");
        DeclareHistogram1D(D_ENERGY_DETX + 2, SE, "beta gated 3Hen");
    }
    {
        using namespace neutr::gammaGated;
        DeclareHistogram1D(D_ENERGY_DETX + 0, SE, "gamma gated scint1 sig1");
        DeclareHistogram1D(D_ENERGY_DETX + 1, SE, "gamma gated scint1 sig2");
        DeclareHistogram1D(D_ENERGY_DETX + 2, SE, "gamma gated 3Hen");
    }

    {
        using namespace neutr::betaGammaGated;
        DeclareHistogram1D(D_ENERGY_DETX + 0, SE, "beta-gamma gated scint1 sig1");
        DeclareHistogram1D(D_ENERGY_DETX + 1, SE, "beta-gamma gated scint1 sig2");
        DeclareHistogram1D(D_ENERGY_DETX + 2, SE, "beta-gamma gated 3Hen");
    }

//for the beta detectors from LeRIBSS
    DeclareHistogram2D(DD_TQDCBETA, SC, S3, "Start vs. Trace QDC");
    DeclareHistogram2D(DD_MAXBETA, SC, S3, "Start vs. Maximum");
}

bool ScintProcessor::PreProcess(RawEvent &event) {
    if(!EventProcessor::PreProcess(event))
        return false;

    static const int BETA_THRESHOLD = 10;

    static const vector<ChanEvent*> &scintBetaEvents =
        event.GetSummary("scint:beta")->GetList();

    for(vector<ChanEvent*>::const_iterator it = scintBetaEvents.begin();
            it != scintBetaEvents.end(); it++) {
            string place = (*it)->GetChanID().GetPlaceName();
            if(TCorrelator::get().places.count(place) == 1) {
                    double time   = (*it)->GetTime();
                    double energy = (*it)->GetEnergy();
                    if(energy > BETA_THRESHOLD) {
                            EventData data(time, true, energy);
                            TCorrelator::get().places[place]->activate(data);
                        }
                }
            else {
                    cerr << "In ScintProcessor: beta place " << place
                         << " does not exist." << endl;
                    return false;
                }
        }

    static const vector<ChanEvent*> &scintNeutrEvents =
        event.GetSummary("scint:neutr")->GetList();

    for(vector<ChanEvent*>::const_iterator it = scintNeutrEvents.begin();
            it != scintNeutrEvents.end(); it++) {
            string place = (*it)->GetChanID().GetPlaceName();
            if(TCorrelator::get().places.count(place) == 1) {
                    double time   = (*it)->GetTime();
                    double energy = (*it)->GetCalEnergy();
                    EventData data(time, true, energy);
                    TCorrelator::get().places[place]->activate(data);
                }
            else {
                    cerr << "In ScintProcessor: beta place " << place
                         << " does not exist." << endl;
                    return false;
                }
        }

    return true;
}

bool ScintProcessor::Process(RawEvent &event) {
    if(!EventProcessor::Process(event))
        return false;

    static const vector<ChanEvent*> &scintNeutrEvents =
        event.GetSummary("scint:neutr")->GetList();

    for(vector<ChanEvent*>::const_iterator it = scintNeutrEvents.begin();
            it != scintNeutrEvents.end(); it++) {

            ChanEvent *chan = *it;
            int loc = chan->GetChanID().GetLocation();
            using namespace neutr;
            double neutronEnergy = chan->GetCalEnergy();
            if(TCorrelator::get().places["Beta"]->status()) {
                    plot(betaGated::D_ENERGY_DETX + loc, neutronEnergy);
                }
            if(TCorrelator::get().places["Gamma"]->status()) {
                    plot(gammaGated::D_ENERGY_DETX + loc, neutronEnergy);
                    if(TCorrelator::get().places["Beta"]->status()) {
                            plot(betaGammaGated::D_ENERGY_DETX + loc, neutronEnergy);
                        }
                }
        }

    LiquidAnalysis(event);
    EndProcess();
    return true;
}

void ScintProcessor::LiquidAnalysis(RawEvent &event) {
    static const vector<ChanEvent*> &liquidEvents =
        event.GetSummary("scint:liquid")->GetList();
    static const vector<ChanEvent*> &betaStartEvents =
        event.GetSummary("scint:beta:start")->GetList();
    static const vector<ChanEvent*> &liquidStartEvents =
        event.GetSummary("scint:liquid:start")->GetList();

    vector<ChanEvent*> startEvents;
    startEvents.insert(startEvents.end(), betaStartEvents.begin(),
                       betaStartEvents.end());
    startEvents.insert(startEvents.end(), liquidStartEvents.begin(),
                       liquidStartEvents.end());

    for(vector<ChanEvent*>::const_iterator itLiquid = liquidEvents.begin();
            itLiquid != liquidEvents.end(); itLiquid++) {
            unsigned int loc = (*itLiquid)->GetChanID().GetLocation();
            TimingData liquid((*itLiquid));

            if(liquid.discrimination == 0) {
                    for(Trace::const_iterator i = liquid.trace.begin();
                            i != liquid.trace.end(); i++)
                        plot(DD_TRCLIQUID, int(i-liquid.trace.begin()),
                             counter, int(*i)-liquid.aveBaseline);
                    counter++;
                }

            if(liquid.dataValid) {
                    plot(DD_TQDCLIQUID, liquid.tqdc, loc);
                    plot(DD_MAXLIQUID, liquid.maxval, loc);

                    double discrimNorm =
                        liquid.discrimination/liquid.tqdc;

                    double discRes = 1000;
                    double discOffset = 100;

                    TimingCal calibration =
                        GetTimingCal(make_pair(loc, "liquid"));

                    if(discrimNorm > 0)
                        plot(DD_DISCRIM, discrimNorm*discRes+discOffset, loc);
                    plot(DD_TQDCVSDISCRIM, discrimNorm*discRes+discOffset,
                         liquid.tqdc);

                    if((*itLiquid)->GetChanID().HasTag("start"))
                        continue;

                    for(vector<ChanEvent*>::iterator itStart = startEvents.begin();
                            itStart != startEvents.end(); itStart++) {
                            unsigned int startLoc = (*itStart)->GetChanID().GetLocation();
                            TimingData start((*itStart));
                            int histLoc = loc + startLoc;
                            const int resMult = 2;
                            const int resOffset = 2000;

                            if(start.dataValid) {
                                double tofOffset;
                                if(startLoc == 0)
                                    tofOffset = calibration.tofOffset0;
                                else
                                    tofOffset = calibration.tofOffset1;

                                double TOF = liquid.highResTime -
                                                 start.highResTime - tofOffset; //in ns
                                double nEnergy = CalcEnergy(TOF, calibration.r0);

                                plot(DD_TOFLIQUID, TOF*resMult+resOffset, histLoc);
                                plot(DD_TOFVSDISCRIM+histLoc,
                                        discrimNorm*discRes+discOffset, TOF*resMult+resOffset);
                                plot(DD_NEVSDISCRIM+histLoc, discrimNorm*discRes+discOffset, nEnergy);
                                plot(DD_TQDCVSLIQTOF+histLoc, TOF*resMult+resOffset,
                                        liquid.tqdc);
                                plot(DD_TQDCVSENERGY+histLoc, nEnergy, liquid.tqdc);
                            }
                    } //Loop over starts
            } // Good Liquid Check
    }//end loop over liquid events
}//void ScintProcessor::LiquidAnalysis
