/** \file GeProcessor.cpp
 * \brief Implementation for germanium processor
 * \author David Miller
 * \date August 2009
 * <STRONG> Modified: </STRONG> Krzysztof Miernik - May 2012
 */
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>

#include "pugixml.hpp"

#include "DammPlotIds.hpp"
#include "DetectorLibrary.hpp"
#include "Exceptions.hpp"
#include "GeProcessor.hpp"
#include "Messenger.hpp"
#include "Notebook.hpp"
#include "Plots.hpp"
#include "PlotsRegister.hpp"
#include "RawEvent.hpp"

using namespace std;
using namespace dammIds::ge;

EventData GeProcessor::BestBetaForGamma(double gTime) {
    PlaceOR* betas = dynamic_cast<PlaceOR*>(
                        TreeCorrelator::get()->place("Beta"));
    unsigned sz = betas->info_.size();

    if (sz == 0)
        return EventData(-1);

    double bestTime = numeric_limits<double>::max();
    unsigned bestIndex = -1;
    for (int index = sz - 1; index >= 0; --index) {
        double dtime = (gTime - betas->info_.at(index).time);
        if (abs(dtime) < abs(bestTime)) {
            bestTime = dtime;
            bestIndex = index;
        }
    }
    return betas->info_.at(bestIndex);
}

bool GeProcessor::GoodGammaBeta(double gb_dtime) {
    if (abs(gb_dtime) > gammaBetaLimit_)
        return false;
    return true;
}

void GeProcessor::symplot(int dammID, double bin1, double bin2) {
    plot(dammID, bin1, bin2);
    plot(dammID, bin2, bin1);
}

GeProcessor::GeProcessor(double gammaThreshold, double lowRatio,
                         double highRatio, double subEventWindow,
                         double gammaBetaLimit, double gammaGammaLimit,
                         double cycle_gate1_min, double cycle_gate1_max,
                         double cycle_gate2_min, double cycle_gate2_max) :
                         EventProcessor(OFFSET, RANGE, "ge"),
                         leafToClover() {
    associatedTypes.insert("ge"); // associate with germanium detectors

    gammaThreshold_ = gammaThreshold;
    lowRatio_ = lowRatio;
    highRatio_ = highRatio;
    subEventWindow_ = subEventWindow;
    gammaBetaLimit_ = gammaBetaLimit;
    gammaGammaLimit_ = gammaGammaLimit;
    cycle_gate1_min_ = cycle_gate1_min;
    cycle_gate1_max_ = cycle_gate1_max;
    cycle_gate2_min_ = cycle_gate2_min;
    cycle_gate2_max_ = cycle_gate2_max;

    // previously used:
    // in seconds/bin
    // 1e-6, 10e-6, 100e-6, 1e-3, 10e-3, 100e-3
    timeResolution.push_back(5e-3);
    timeResolution.push_back(10e-3);
    if (timeResolution.size() > MAX_TIMEX) {
        stringstream ss;
        ss << "Number of requested time resolution spectra is greater then"
            << " MAX_TIMEX = " << MAX_TIMEX << "."
            << " See GeProcessor.hpp for details.";
        throw GeneralException(ss.str());
    }

#ifdef GGATES
    Messenger m;
    m.detail("Loading Gamma-gamma gates", 1);

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("Config.xml");
    if (!result) {
        stringstream ss;
        ss << "DetectorDriver: error parsing file Config.xml";
        ss << " : " << result.description();
        throw IOException(ss.str());
    }

    pugi::xml_node gamma_gates = doc.child("Configuration").child("GammaGates");
    for (pugi::xml_node gate = gamma_gates.child("Gate"); gate;
         gate = gate.next_sibling("Gate")) {
        vector<LineGate> vg;
        bool completeGate = true;
        for (pugi::xml_node line = gate.child("Line"); line;
             line = line.next_sibling("Line")) {
            double min = line.attribute("min").as_double();
            double max = line.attribute("max").as_double();
            LineGate lg = LineGate(min, max);
            if (lg.Check()) {
                vg.push_back(LineGate(min, max));
            } else {
                completeGate = false;
                continue;
            }
        }
        if (vg.size() != 2 && completeGate) {
            throw GeneralException("Gamma-gamma gate size different than 2 is \
not implemented");
        } else {
            stringstream ss;
            ss << "Gate ";
            sort(vg.begin(), vg.end());
            for (vector<LineGate>::iterator it = vg.begin();
                    it != vg.end(); ++it)
                ss << "(" << it->min << "-" << it->max << "), ";
            if (completeGate) {
                gGates.push_back(vg);
                ss << "loaded";
            } else {
                ss << "has bad definition and is skipped!";
            }
            m.detail(ss.str(), 2);
        }
    }
#endif
}

/** Declare plots including many for decay/implant/neutron gated analysis  */
void GeProcessor::DeclarePlots(void) {
    const int energyBins1  = SD;
    const int energyBins2  = SC;
    const int timeBins1    = S8;
    const int timeBins2    = S9;
    const int granTimeBins = SA;

    using namespace dammIds::ge;

    /** This was moved here from Init because the number of clovers is needed
     * for plots declaration, however Init is called after DeclarePlots */

    /* clover specific routine, determine the number of clover detector
       channels and divide by four to find the total number of clovers
    */
    DetectorLibrary* modChan = DetectorLibrary::get();

    const set<int> &cloverLocations = modChan->GetLocations("ge",
                                                            "clover_high");
    // could set it now but we'll iterate through the locations to set this
    unsigned int cloverChans = 0;

    for ( set<int>::const_iterator it = cloverLocations.begin();
	  it != cloverLocations.end(); it++) {
        leafToClover[*it] = int(cloverChans / 4);
        cloverChans++;
    }

    if (cloverChans % chansPerClover != 0) {
        stringstream ss;
        ss << " There does not appear to be the proper number of"
           << " channels per clover.";
        throw GeneralException(ss.str());
    }

    if (cloverChans != 0) {
        numClovers = cloverChans / chansPerClover;
        Messenger m;
        m.start("Building clovers");

        stringstream ss;
        ss << "A total of " << cloverChans
           << " clover channels were detected: ";
        int lastClover = numeric_limits<int>::min();
        for ( map<int, int>::const_iterator it = leafToClover.begin();
            it != leafToClover.end(); it++ ) {
            if (it->second != lastClover) {
                m.detail(ss.str());
                ss.str("");
                lastClover = it->second;
                ss << "Clover " << lastClover << " : ";
            } else {
                ss << ", ";
            }
            ss << setw(2) << it->first;
        }
        m.detail(ss.str());

        if (numClovers > dammIds::ge::MAX_CLOVERS) {
            m.fail();
            stringstream ss;
            ss << "Number of detected clovers is greater than defined"
               << " MAX_CLOVERS = " << dammIds::ge::MAX_CLOVERS << "."
               << " See GeProcessor.hpp for details.";
            throw GeneralException(ss.str());
        }
        m.done();
    }

    for (unsigned i = 0; i < numClovers; ++i) {
        vector<AddBackEvent> empty;
        addbackEvents_.push_back(empty);
    }

    DeclareHistogram1D(D_ENERGY, energyBins1, "Gamma singles");
    DeclareHistogram1D(D_ENERGY_MOVE, energyBins1,
                       "Gamma singles tape move period");
    DeclareHistogram1D(betaGated::D_ENERGY_MOVE, energyBins1,
                       "Beta gated gamma tape move period");
    DeclareHistogram1D(betaGated::D_ENERGY, energyBins1, "Beta gated gamma");
    DeclareHistogram1D(betaGated::D_ENERGY_PROMPT, energyBins1,
                      "Beta gated gamma prompt");
    DeclareHistogram1D(betaGated::D_ENERGY_BETA0, energyBins1,
                       "Gamma beta0 gate prompt");
    DeclareHistogram1D(betaGated::D_ENERGY_BETA1, energyBins1,
                       "Gamma beta1 gate prompt");
    DeclareHistogram2D(betaGated::DD_ENERGY__BETAGAMMALOC, energyBins2, S4,
                       "Gamma vs. Clover loc * 3 + Beta loc prompt");

    DeclareHistogram1D(D_MULT, S7, "Gamma multiplicity");

    DeclareHistogram1D(D_ADD_ENERGY, energyBins1, "Gamma addback");
    DeclareHistogram1D(multi::D_ADD_ENERGY, energyBins1,
                       "Gamma addback multi-gated");
    DeclareHistogram1D(betaGated::D_ADD_ENERGY, energyBins1,
                       "Beta gated gamma addback");
    DeclareHistogram1D(multi::betaGated::D_ADD_ENERGY, energyBins1,
                       "Beta gated gamma addback multi-gated");
    DeclareHistogram1D(betaGated::D_ADD_ENERGY_PROMPT, energyBins1,
                       "Beta gated gamma addback beta-prompt");
    DeclareHistogram1D(multi::betaGated::D_ADD_ENERGY_PROMPT, energyBins1,
                       "Beta gated gamma addback multi-gated beta-prompt");

    DeclareHistogram1D(D_ADD_ENERGY_TOTAL, energyBins1, "Gamma total");
    DeclareHistogram1D(multi::D_ADD_ENERGY_TOTAL, energyBins1,
                       "Gamma total multi-gated");
    DeclareHistogram1D(betaGated::D_ADD_ENERGY_TOTAL, energyBins1,
                       "Beta gated gamma total");
    DeclareHistogram1D(multi::betaGated::D_ADD_ENERGY_TOTAL, energyBins1,
                       "Beta gated gamma total multi-gated");

    // for each clover
    for (unsigned int i = 0; i < numClovers; i++) {
        stringstream ss;
        ss << "Clover " << i << " gamma";
        DeclareHistogram1D(D_ENERGY_CLOVERX + i,
                energyBins1, ss.str().c_str());

        ss.str("");
        ss << "Clover " << i << " beta gated gamma";
        DeclareHistogram1D(betaGated::D_ENERGY_CLOVERX + i,
                energyBins1, ss.str().c_str());

        ss.str("");
        ss << "Clover " << i << " gamma addback";
            DeclareHistogram1D(D_ADD_ENERGY_CLOVERX + i,
                    energyBins1, ss.str().c_str());

        ss.str("");
        ss << "Clover " << i << " beta gated gamma addback";
            DeclareHistogram1D(betaGated::D_ADD_ENERGY_CLOVERX + i,
                    energyBins1, ss.str().c_str());
    }

    DeclareHistogram2D(DD_ENERGY, energyBins2, energyBins2, "Gamma gamma");
    DeclareHistogram2D(DD_ENERGY_PROMPT, energyBins2, energyBins2,
                       "Gamma gamma prompt");
    DeclareHistogram2D(DD_ENERGY_CGATE1,
                       energyBins2, energyBins2,
                       "Gamma gamma cycle gate 1");
    DeclareHistogram2D(DD_ENERGY_CGATE2,
                       energyBins2, energyBins2,
                       "Gamma gamma cycle gate 2");

    DeclareHistogram2D(betaGated::DD_ENERGY,
                       energyBins2, energyBins2,
                       "Gamma gamma beta prompt gated");
    DeclareHistogram2D(betaGated::DD_ENERGY_PROMPT,
                       energyBins2, energyBins2,
                       "Gamma gamma prompt beta prompt gated");
    DeclareHistogram2D(betaGated::DD_ENERGY_BDELAYED,
                       energyBins2, energyBins2,
                       "Beta-gated gamma gamma - beta delayed");

    DeclareHistogram2D(betaGated::DD_ENERGY_CGATE1,
                       energyBins2, energyBins2,
                       "Beta gated gamma gamma cycle gate 1");
    DeclareHistogram2D(betaGated::DD_ENERGY_CGATE2,
                       energyBins2, energyBins2,
                       "Beta gated gamma gamma cycle gate 2");

    DeclareHistogram2D(DD_ADD_ENERGY,
                       energyBins2, energyBins2, "Gamma gamma addback");
    DeclareHistogram2D(multi::DD_ADD_ENERGY,
                       energyBins2, energyBins2,
                       "Gamma gamma addback multi-gated");
    DeclareHistogram2D(betaGated::DD_ADD_ENERGY,
            energyBins2, energyBins2, "Beta-gated gamma-gamma addback");
    DeclareHistogram2D(multi::betaGated::DD_ADD_ENERGY,
                       energyBins2, energyBins2,
                       "Beta-gated gamma-gamma addback multi-gated");
    DeclareHistogram2D(betaGated::DD_ADD_ENERGY_PROMPT,
                       energyBins2, energyBins2,
                       "Beta-gated Gamma gamma addback beta-prompt");
    DeclareHistogram2D(multi::betaGated::DD_ADD_ENERGY_PROMPT,
                       energyBins2, energyBins2,
                    "Beta-gated gamma-gamma addback multi-gated beta-prompt");

    DeclareHistogram2D(
            DD_TDIFF__GAMMA_GAMMA_ENERGY,
            timeBins1, energyBins2,
            "Same clover, gamma energy, gamma-gamma time diff + 100 (10 ns)");
    DeclareHistogram2D(
            DD_TDIFF__GAMMA_GAMMA_ENERGY_SUM,
            timeBins1,
            energyBins2,
           "Same clover, gamma energy sum, gamma-gamma time diff + 100 (10 ns)");
    DeclareHistogram2D(
            betaGated::DD_TDIFF__GAMMA_ENERGY,
            timeBins2, energyBins2,
            "Gamma energy, gamma - beta time diff + 100 (10 ns)");
    DeclareHistogram2D(
            betaGated::DD_TDIFF__BETA_ENERGY,
            timeBins1, energyBins2,
            "Beta energy/10, gamma- beta time diff + 100 (10 ns)");

#ifdef GGATES
    DeclareHistogram2D(DD_TDIFF__GATEX, timeBins1, S5,
                        "g_g time diff + 100 (10 ns) vs gate");
    DeclareHistogram2D(betaGated::DD_TDIFF__GATEX, timeBins1, S5,
                        "g_g beta gated time diff + 100 (10 ns) vs gate");
    DeclareHistogram2D(DD_ENERGY__GATEX, energyBins2, S5,
                       "g_g gated gamma energy");
    DeclareHistogram2D(betaGated::DD_ENERGY__GATEX, energyBins2, S5,
                       "g_g_beta gated gamma energy vs gate");
    DeclareHistogram2D(DD_ANGLE__GATEX, S2, S5,
                       "g_g gated angle vs gate");
    DeclareHistogram2D(betaGated::DD_ANGLE__GATEX, S2, S5,
                       "g_g_beta gated angle vs gate");
#endif

    DeclareHistogramGranY(DD_ENERGY__TIMEX,
                          energyBins2, granTimeBins,
                          "E - Time", 2, timeResolution, "s");
    DeclareHistogramGranY(DD_ADD_ENERGY__TIMEX,
                          energyBins2, granTimeBins,
                          "Addback E - Time", 2, timeResolution, "s");
    DeclareHistogramGranY(betaGated::DD_ENERGY__TIMEX_GROW,
                          energyBins2, granTimeBins,
                          "Beta-gated E - Time, beam on only prompt",
                          2, timeResolution, "s");
    DeclareHistogramGranY(betaGated::DD_ENERGY__TIMEX_DECAY,
                          energyBins2, granTimeBins,
                          "Beta-gated E - Time, beam off only prompt",
                          2, timeResolution, "s");
    DeclareHistogramGranY(betaGated::DD_ENERGY__TIMEX,
                          energyBins2, granTimeBins,
                          "Beta-gated E - Time prompt", 2, timeResolution, "s");
    DeclareHistogramGranY(betaGated::DD_ADD_ENERGY__TIMEX,
                          energyBins2, granTimeBins,
                          "Beta-gated addback E - Time",
                          2, timeResolution, "s");
}


bool GeProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    geEvents_.clear();
    for (unsigned i = 0; i < numClovers; ++i)
        addbackEvents_[i].clear();
    tas_.clear();

    static const vector<ChanEvent*> &highEvents =
        event.GetSummary("ge:clover_high", true)->GetList();
    static const vector<ChanEvent*> &lowEvents  =
        event.GetSummary("ge:clover_low", true)->GetList();

    /** Only the high gain events are going to be used. The events where
     * low/high gain mismatches, saturation or pileup is marked are rejected
     */
    for (vector<ChanEvent*>::const_iterator itHigh = highEvents.begin();
	 itHigh != highEvents.end(); itHigh++) {
        int location = (*itHigh)->GetChanID().GetLocation();
        if ( (*itHigh)->IsSaturated() || (*itHigh)->IsPileup() )
            continue;

        vector <ChanEvent*>::const_iterator itLow = lowEvents.begin();
        for (; itLow != lowEvents.end(); itLow++) {
            if ( (*itLow)->GetChanID().GetLocation() == location ) {
                break;
            }
        }
        if ( itLow != lowEvents.end() ) {
            double ratio = (*itHigh)->GetEnergy() / (*itLow)->GetEnergy();
            if (ratio < lowRatio_ || ratio > highRatio_)
                continue;
        }
        geEvents_.push_back(*itHigh);
    }

    // now we sort the germanium events according to their corrected time
    sort(geEvents_.begin(), geEvents_.end(), CompareCorrectedTime);

    /** Here the addback spectra is constructed.
     *  addbackEvents_ is a vector for each clover
     *  holding a vector of pairs <energy, time> (both double),
     *  where each element is a one addback event for a given clover.
     * */

    /** guarantee the first event will be greater than
     * the subevent window delayed from reference
     */
    double refTime = -2.0 * subEventWindow_;

    for (vector<ChanEvent*>::iterator it = geEvents_.begin(); it != geEvents_.end(); it++) {
        ChanEvent *chan = *it;
        double energy = chan->GetCalEnergy();
        double time = chan->GetCorrectedTime();
        int clover = leafToClover[chan->GetChanID().GetLocation()];

        /**
        * Do not take into account events with too low energy
        * (avoid summing of noise with real gammas)
        */
        if (energy < gammaThreshold_)
            continue;

        // entries in map are sorted by time
        // if event time is outside of subEventWindow, we start new
        //   events for all clovers and "tas"
        double dtime = abs(time - refTime) * Globals::get()->clockInSeconds();
        if (dtime > subEventWindow_) {
            for (unsigned i = 0; i < numClovers; ++i) {
                addbackEvents_[i].push_back(AddBackEvent());
            }
            tas_.push_back(AddBackEvent());
        }
        // Total addback energy
        addbackEvents_[clover].back().energy += energy;
        // We store latest time only
        addbackEvents_[clover].back().time = time;
        addbackEvents_[clover].back().multiplicity += 1;
        tas_.back().energy += energy;
        tas_.back().time = time;
        tas_.back().multiplicity += 1;
        refTime = time;
    }

    return true;
}

bool GeProcessor::Process(RawEvent &event) {
    using namespace dammIds::ge;

    if (!EventProcessor::Process(event))
        return false;

    double clockInSeconds = Globals::get()->clockInSeconds();

    /** Cycle time is measured from the begining of the last BeamON event */
    double cycleTime = TreeCorrelator::get()->place("Cycle")->last().time;

    // beamOn is true for beam on and false for beam off
    bool beamOn =  TreeCorrelator::get()->place("Beam")->status();
    bool hasBeta = TreeCorrelator::get()->place("Beta")->status();

    /** Place Cycle is activated by BeamOn event and deactivated by TapeMove
     *  This condition will therefore skip events registered during
     *  tape movement period and before the end of move and the beam start
     */
    if (!TreeCorrelator::get()->place("Cycle")->status()) {
        for (vector<ChanEvent*>::iterator it = geEvents_.begin();
        it != geEvents_.end(); ++it) {
            ChanEvent* chan = *it;
            double gEnergy = chan->GetCalEnergy();
            if (gEnergy < gammaThreshold_)
                continue;
            plot(D_ENERGY_MOVE, gEnergy);
            if (hasBeta)
                plot(betaGated::D_ENERGY_MOVE, gEnergy);
        }
        return true;
    }

    plot(D_MULT, geEvents_.size());

    // Note that geEvents_ vector holds only good events (matched
    // low & high gain). See PreProcess
    for (vector<ChanEvent*>::iterator it1 = geEvents_.begin();
	 it1 != geEvents_.end(); ++it1) {
        ChanEvent* chan = *it1;

        double gEnergy = chan->GetCalEnergy();
        if (gEnergy < gammaThreshold_)
            continue;

        double gTime = chan->GetCorrectedTime();
        double decayTime = (gTime - cycleTime) * clockInSeconds;
        int det = leafToClover[chan->GetChanID().GetLocation()];

        plot(D_ENERGY, gEnergy);
        plot(D_ENERGY_CLOVERX + det, gEnergy);
        granploty(DD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);

        double gb_dtime = numeric_limits<double>::max();
        if (hasBeta) {
            EventData bestBeta = BestBetaForGamma(gTime);
            gb_dtime = (gTime - bestBeta.time) * clockInSeconds;
            double betaEnergy = bestBeta.energy;
            int betaLocation = bestBeta.location;

            double plotResolution = clockInSeconds;
            plot(betaGated::DD_TDIFF__GAMMA_ENERGY,
                    (int)(gb_dtime / plotResolution + 100), gEnergy);
            plot(betaGated::DD_TDIFF__BETA_ENERGY,
                    (int)(gb_dtime / plotResolution + 100), betaEnergy/10);

            plot(betaGated::D_ENERGY, gEnergy);
            if (GoodGammaBeta(gb_dtime)) {
                plot(betaGated::D_ENERGY_PROMPT, gEnergy);
                plot(betaGated::D_ENERGY_CLOVERX + det, gEnergy);
                granploty(betaGated::DD_ENERGY__TIMEX,
                          gEnergy, decayTime, timeResolution);

                if (beamOn) {
                    granploty(betaGated::DD_ENERGY__TIMEX_GROW,
                            gEnergy, decayTime, timeResolution);
                } else {
                    /** Beam deque should be updated upon beam off so
                    * measure time from that point
                    * (t = 0 is time beam went off)
                    */
                    double decayTimeOff = (gTime -
                         TreeCorrelator::get()->place("Beam")->last().time) *
                         clockInSeconds;
                    granploty(betaGated::DD_ENERGY__TIMEX_DECAY,
                            gEnergy, decayTimeOff, timeResolution);
                }

                // individual beta gamma coinc spectra for each beta detector
                if (betaLocation == 0) {
                    plot(betaGated::D_ENERGY_BETA0, gEnergy);
                } else if (betaLocation == 1) {
                    plot(betaGated::D_ENERGY_BETA1, gEnergy);
                }
                plot(betaGated::DD_ENERGY__BETAGAMMALOC,
                     gEnergy, det * 3 +  betaLocation);
            }
        }

        for (vector<ChanEvent*>::const_iterator it2 = it1 + 1;
                it2 != geEvents_.end(); it2++) {
            ChanEvent* chan2 = *it2;

            double gEnergy2 = chan2->GetCalEnergy();
            int det2 = leafToClover[chan2->GetChanID().GetLocation()];
            double gTime2 = chan2->GetCorrectedTime();

            if (gEnergy2 < gammaThreshold_)
                continue;

            double gg_dtime = (gTime2 - gTime) * clockInSeconds;

            /** Plot timediff between events in the same clover
             * to monitor addback subevent gates. */
            if (det == det2) {
                double plotResolution = clockInSeconds;
                plot(DD_TDIFF__GAMMA_GAMMA_ENERGY,
                     (int)(gg_dtime / plotResolution + 100),
                     gEnergy);
                plot(DD_TDIFF__GAMMA_GAMMA_ENERGY_SUM,
                      (int)(gg_dtime / plotResolution + 100),
                      gEnergy + gEnergy2);
            }

            /*
             * This condition removes coincidences within the same
             * clover significantly reducing "cross-talk"
             * but also reducing efficiency
             * (by 20% approx)
             */
            if (det2 != det) {
                symplot(DD_ENERGY, gEnergy, gEnergy2);

                if (decayTime > cycle_gate1_min_ &&
                    decayTime < cycle_gate1_max_)
                    symplot(DD_ENERGY_CGATE1, gEnergy, gEnergy2);
                if (decayTime > cycle_gate2_min_ &&
                    decayTime < cycle_gate2_max_)
                    symplot(DD_ENERGY_CGATE2, gEnergy, gEnergy2);

                if (hasBeta) {
                    if (GoodGammaBeta(gb_dtime)) {
                        symplot(betaGated::DD_ENERGY, gEnergy,
                                gEnergy2);
                        if (decayTime > cycle_gate1_min_ &&
                            decayTime < cycle_gate1_max_)
                            symplot(betaGated::DD_ENERGY_CGATE1,
                                    gEnergy, gEnergy2);
                        if (decayTime > cycle_gate2_min_ &&
                            decayTime < cycle_gate2_max_)
                            symplot(betaGated::DD_ENERGY_CGATE2,
                                    gEnergy, gEnergy2);

                    } else if (gb_dtime > gammaBetaLimit_) {
                            symplot(betaGated::DD_ENERGY_BDELAYED,
                                    gEnergy, gEnergy2);
                    }
                }

                if (abs(gg_dtime) < gammaGammaLimit_) {
                    symplot(DD_ENERGY_PROMPT, gEnergy, gEnergy2);
                    if (hasBeta && GoodGammaBeta(gb_dtime)) {
                        symplot(betaGated::DD_ENERGY_PROMPT,
                                gEnergy, gEnergy2);
                    }
                }
            }
#ifdef GGATES
            /**
            * Gamma-gamma gate
            */
            unsigned ig = 0;
            double e1 = min(gEnergy, gEnergy2);
            double e2 = max(gEnergy, gEnergy2);
            for (vector< vector<LineGate> >::iterator it_gate =
                    gGates.begin();
                    it_gate != gGates.end(); ++it_gate) {
                if ((*it_gate).size() != 2)
                    throw NotImplemented("Gamma gates of size different than 2 are not implemented");
                if ((*it_gate)[0].IsWithin(e1) &&
                    (*it_gate)[1].IsWithin(e2)) {

                    double plotResolution = clockInSeconds;
                    plot(DD_TDIFF__GATEX,
                         (int)(gg_dtime / plotResolution + 100), ig);
                    if (hasBeta && GoodGammaBeta(gb_dtime))
                        plot(betaGated::DD_TDIFF__GATEX,
                            (int)(gg_dtime / plotResolution + 100), ig);

                    /** Angular corelations:
                     * 4 clover setup :
                     *     |0|
                     * |3|     |1|
                     *     |2|
                     *
                     * bin 0 -> same clover (0 deg), 1 -> 90 deg, 2 -> 180 deg
                     */
                    if (det == det2) {
                        plot(DD_ANGLE__GATEX, 0, ig);
                        if (hasBeta && GoodGammaBeta(gb_dtime))
                            plot(betaGated::DD_ANGLE__GATEX, 0, ig);
                    } else if (det % 2 != det2 % 2) {
                        plot(DD_ANGLE__GATEX, 1, ig);
                        if (hasBeta && GoodGammaBeta(gb_dtime))
                            plot(betaGated::DD_ANGLE__GATEX, 1, ig);
                    } else {
                        plot(DD_ANGLE__GATEX, 2, ig);
                        if (hasBeta && GoodGammaBeta(gb_dtime))
                            plot(betaGated::DD_ANGLE__GATEX, 2, ig);
                    }

                    for (vector<ChanEvent*>::const_iterator it3 = it2 + 1;
                            it3 != geEvents_.end(); it3++) {
                        double gEnergy3 = (*it3)->GetCalEnergy();
                        if (gEnergy3 < gammaThreshold_)
                            continue;
                        plot(DD_ENERGY__GATEX, gEnergy3, ig);
                        if (hasBeta && GoodGammaBeta(gb_dtime))
                            plot(betaGated::DD_ENERGY__GATEX, gEnergy3, ig);
                    }
                }
                ++ig;
            }
#endif
        } // iteration over other gammas
    }

    // Vectors tas and addbackEvents should have the same size
    unsigned nEvents = tas_.size();

    // Plot 'tas' spectra
    for (unsigned i = 0; i < nEvents; ++i) {
        double gEnergy = tas_[i].energy;
        double gTime = tas_[i].time;
        double gMulti = tas_[i].multiplicity;

        if (gEnergy < gammaThreshold_)
            continue;

        double gb_dtime = numeric_limits<double>::max();
        if (hasBeta) {
            EventData bestBeta = BestBetaForGamma(gTime);
            gb_dtime = (gTime - bestBeta.time) * clockInSeconds;
        }

        plot(D_ADD_ENERGY_TOTAL, gEnergy);
        if (gMulti == 1)
            plot(multi::D_ADD_ENERGY_TOTAL, gEnergy);
        if (hasBeta && GoodGammaBeta(gb_dtime)) {
            plot(betaGated::D_ADD_ENERGY_TOTAL, gEnergy);
            if (gMulti == 1)
                plot(multi::betaGated::D_ADD_ENERGY_TOTAL, gEnergy);
        }
    }

    // Plot addback spectra
    for (unsigned int ev = 0; ev < nEvents; ev++) {
        for (unsigned int det = 0; det < numClovers; ++det) {
            double gEnergy = addbackEvents_[det][ev].energy;
            if (gEnergy < gammaThreshold_)
                continue;

            double gTime = addbackEvents_[det][ev].time;
            double gMulti = addbackEvents_[det][ev].multiplicity;
            double decayTime = (gTime - cycleTime) * clockInSeconds;

            plot(D_ADD_ENERGY, gEnergy);
            plot(D_ADD_ENERGY_CLOVERX + det, gEnergy);
            granploty(DD_ADD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
            if (gMulti == 1)
                plot(multi::D_ADD_ENERGY, gEnergy);

            double gb_dtime = numeric_limits<double>::max();
            if (hasBeta) {
                EventData bestBeta = BestBetaForGamma(gTime);
                gb_dtime = (gTime - bestBeta.time) * clockInSeconds;

                plot(betaGated::D_ADD_ENERGY, gEnergy);
                if (gMulti == 1)
                    plot(multi::betaGated::D_ADD_ENERGY, gEnergy);
                plot(betaGated::D_ADD_ENERGY_CLOVERX + det, gEnergy);
                if (GoodGammaBeta(gb_dtime)) {
                    plot(betaGated::D_ADD_ENERGY_PROMPT, gEnergy);
                    if (gMulti == 1)
                        plot(multi::betaGated::D_ADD_ENERGY_PROMPT, gEnergy);
                    granploty(betaGated::DD_ADD_ENERGY__TIMEX, gEnergy,
                            decayTime, timeResolution);
                }
            }

            for (unsigned int det2 = det + 1;
                    det2 < numClovers; ++det2) {
                double gEnergy2 = addbackEvents_[det2][ev].energy;
                if (gEnergy2 < gammaThreshold_)
                    continue;

                double gTime2 = addbackEvents_[det2][ev].time;
                double gMulti2 = addbackEvents_[det2][ev].multiplicity;
                double gg_dtime = (gTime2 - gTime) * clockInSeconds;
                if (abs(gg_dtime) > gammaGammaLimit_)
                    continue;

                symplot(DD_ADD_ENERGY, gEnergy, gEnergy2);
                if (gMulti == 1 && gMulti2 == 1)
                    symplot(multi::DD_ADD_ENERGY, gEnergy, gEnergy2);
                if (hasBeta) {
                    symplot(betaGated::DD_ADD_ENERGY, gEnergy, gEnergy2);
                    if (gMulti == 1 && gMulti2 == 1)
                        symplot(multi::betaGated::DD_ADD_ENERGY,
                                gEnergy, gEnergy2);
                    if (GoodGammaBeta(gb_dtime)) {
                        symplot(betaGated::DD_ADD_ENERGY_PROMPT,
                                gEnergy, gEnergy2);
                        if (gMulti == 1 && gMulti2 == 1)
                            symplot(multi::betaGated::DD_ADD_ENERGY_PROMPT,
                                    gEnergy, gEnergy2);
                    }
                }
            } // iteration over other clovers
        } // itertaion over clovers
    } // iteration over events

    EndProcess(); // update the processing time
    return true;
}

/**
 * Declare a 2D plot with a range of granularites on the Y axis
 */
void GeProcessor::DeclareHistogramGranY(int dammId, int xsize, int ysize,
					const char *title, int halfWordsPerChan,
					const vector<float> &granularity, const char *units)
{
    stringstream fullTitle;

    for (unsigned int i=0; i < granularity.size(); i++) {
        //? translate scientific units to engineering units
        fullTitle << title << " (" << granularity[i] << " " << units << "/bin)";
        histo.DeclareHistogram2D(dammId + i, xsize, ysize,
                                 fullTitle.str().c_str(),
                                 halfWordsPerChan, 1, 1);
        fullTitle.str("");
    }
}

/**
 * Plot to a granularity spectrum
 */
void GeProcessor::granploty(int dammId, double x, double y, const vector<float> &granularity)
{
    for (unsigned int i=0; i < granularity.size(); i++) {
        plot(dammId + i, x, y / granularity[i]);
    }
}
