/** \file GeProcessor.cpp
 *
 * implementation for germanium processor
 * David Miller, August 2009
 * Overhaul: Krzysztof Miernik, May 2012
 */

//? Make a clover specific processor

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>

#include <limits>
#include <cmath>
#include <cstdlib>

#include "pugixml.hpp"
#include "PathHolder.hpp"

#include "Plots.hpp"
#include "PlotsRegister.hpp"
#include "DammPlotIds.hpp"

#include "Correlator.hpp"
#include "DetectorLibrary.hpp"
#include "GeProcessor.hpp"
#include "RawEvent.hpp"
#include "Messenger.hpp"
#include "Exceptions.hpp"

using namespace std;

/**
 *  Perform walk correction for gamma-ray timing based on its energy
 */
double GeProcessor::WalkCorrection(double e) {
    // SVP version
    /*
    const double b  = 3042.22082339404;
    const double t3 = 29.5154635093362;
    const double y1 = 7.79788784543252;
    return y1 + b / (e + t3);
    */
    //return 0;
    if (e <= 100) {
        const double a0 = 118.482;
        const double a1 = -2.66767;
        const double a2 = 0.028492;
        const double a3 = -0.000107801;

        const double c  = -40.2608;
        const double e0 = 12.5426;
        const double k  = 3.84917;

        return a0 + a1 * e + a2 * pow(e, 2) + a3 * pow(e, 3) +
            c / (1.0 + exp( (e - e0) / k) );
    } else {
        const double a0 = 11.2287;
        const double a1 = -0.000939443;
        
        const double b0 = 38.8912;
        const double b1 = -0.121784;
        const double b2 = 0.000268339;

        const double k  = 180.146;

        return a0 + a1 * e + (b0 + b1 * e + b2 * pow(e, 2)) * exp(-e / k);
    }
}

pair<double, EventData> GeProcessor::BestBetaForGamma(double gTime) {
    PlaceOR* betas = dynamic_cast<PlaceOR*>(
                        TreeCorrelator::get()->place("Beta"));
    unsigned sz = betas->info_.size();
    if (sz == 0)
        return pair<double, EventData>(numeric_limits<double>::max(), 
                                       EventData(-1));

    double bestTime = numeric_limits<double>::max();
    unsigned bestIndex = -1;
    for (int index = sz - 1; index >= 0; --index) {
        double dtime = (gTime - betas->info_.at(index).time) *
                       pixie::clockInSeconds;
        if (abs(dtime) < abs(bestTime)) {
            bestTime = dtime;
            bestIndex = index;
        }
    }
    return pair<double, EventData>(bestTime, betas->info_.at(bestIndex));
}

bool GeProcessor::GoodGammaBeta(double gb_dtime, 
                        double limit_in_sec /* = detectors::gammaBetaLimit*/) {
    if (abs(gb_dtime) > limit_in_sec)
        return false;
    return true;
}

// useful function for symmetrically incrementing 2D plots
void GeProcessor::symplot(int dammID, double bin1, double bin2)
{
    plot(dammID, bin1, bin2);
    plot(dammID, bin2, bin1);
}

using namespace dammIds::ge;

GeProcessor::GeProcessor() : EventProcessor(OFFSET, RANGE), leafToClover() {
    name = "ge";
    associatedTypes.insert("ge"); // associate with germanium detectors

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

    PathHolder* conf_path = new PathHolder();
    string xmlFileName = conf_path->GetFullPath("Config.xml");
    delete conf_path;

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(xmlFileName.c_str());
    if (!result) {
        stringstream ss;
        ss << "GeProcessor: could not parse file " << xmlFileName;
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
            double min = strings::to_double(line.attribute("min").value());
            double max = strings::to_double(line.attribute("max").value());
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
void GeProcessor::DeclarePlots(void) 
{
    const int energyBins1  = SD;
    const int energyBins2  = SC;
    const int timeBins1    = S8;
    const int granTimeBins = SA;

    using namespace dammIds::ge;

    /** This was moved here from Init because the number of clovers is needed
     * for plots declaration, however Init is called after DeclarePlots */

    /* clover specific routine, determine the number of clover detector
       channels and divide by four to find the total number of clovers
    */
    DetectorLibrary* modChan = DetectorLibrary::get();

    const set<int> &cloverLocations = modChan->GetLocations("ge", "clover_high");
    // could set it now but we'll iterate through the locations to set this
    unsigned int cloverChans = 0;

    for ( set<int>::const_iterator it = cloverLocations.begin();
	  it != cloverLocations.end(); it++) {
        leafToClover[*it] = int(cloverChans / 4); 
        cloverChans++;
    }

    if (cloverChans % chansPerClover != 0) {
        cout << " There does not appear to be the proper number of"
            << " channels per clover.\n Program terminating." << endl;
        exit(EXIT_FAILURE);	
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
        vector< pair<double, double> > empty;
        addbackEvents_.push_back(empty);
    }

    DeclareHistogram1D(D_ENERGY, energyBins1, "Gamma singles");
    DeclareHistogram1D(betaGated::D_ENERGY, energyBins1, "Beta gated gamma");
    DeclareHistogram1D(betaGated::D_ENERGY_BETA0, energyBins1,
                       "Gamma beta0 gate");
    DeclareHistogram1D(betaGated::D_ENERGY_BETA1, energyBins1,
                       "Gamma beta1 gate");
    DeclareHistogram2D(betaGated::DD_ENERGY__BETAGAMMALOC, energyBins2, S4,
                       "Gamma vs. Clover loc * 3 + Beta loc");

    DeclareHistogram1D(D_ENERGY_LOWGAIN, energyBins1,
                       "Gamma singles, low gain");
    DeclareHistogram1D(D_ENERGY_HIGHGAIN, energyBins1,
                       "Gamma singles, high gain");
    DeclareHistogram2D(DD_CLOVER_ENERGY_RATIO, S4, S6,
            "high/low energy ratio (x10)");

    DeclareHistogram1D(D_MULT, S3, "Gamma multiplicity");                  

    DeclareHistogram1D(D_ADD_ENERGY, energyBins1, "Gamma addback");
    DeclareHistogram1D(betaGated::D_ADD_ENERGY, energyBins1,
                       "Beta gated gamma addback");
    DeclareHistogram1D(D_ADD_ENERGY_TOTAL, energyBins1, "Gamma total");
    DeclareHistogram1D(betaGated::D_ADD_ENERGY_TOTAL, energyBins1,
                       "Beta gated gamma total");
    
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
    DeclareHistogram2D(DD_ENERGY_PROMPT_EARLY,
                       energyBins2, energyBins2,
                       "Gamma gamma prompt early");
    DeclareHistogram2D(DD_ENERGY_PROMPT_LATE,
                       energyBins2, energyBins2,
                       "Gamma gamma prompt late");

    DeclareHistogram2D(betaGated::DD_ENERGY,
                       energyBins2, energyBins2, "Gamma gamma beta gated");
    DeclareHistogram2D(betaGated::DD_ENERGY_PROMPT,
                       energyBins2, energyBins2, 
                       "Gamma gamma prompt beta gated");
    DeclareHistogram2D(betaGated::DD_ENERGY_PROMPT_EARLY,
                       energyBins2, energyBins2,
                       "Gamma gamma prompt beta gated early");
    DeclareHistogram2D(betaGated::DD_ENERGY_PROMPT_LATE,
                       energyBins2, energyBins2,
                       "Gamma gamma prompt beta gated late");

    DeclareHistogram2D(DD_ADD_ENERGY,
                       energyBins2, energyBins2, "Gamma gamma addback");
    DeclareHistogram2D(betaGated::DD_ENERGY_BDELAYED,
      energyBins2, energyBins2, "Beta-gated gamma gamma - beta delayed");

    DeclareHistogram2D(betaGated::DD_ADD_ENERGY,
            energyBins2, energyBins2, "Beta-gated Gamma gamma addback");

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
            timeBins1, energyBins2,
            "Gamma energy, gamma - beta time diff + 100 (10 ns)");
    DeclareHistogram2D(
            betaGated::DD_TDIFF__BETA_ENERGY,
            timeBins1, energyBins2,
            "Beta energy, gamma- beta time diff + 100 (10 ns)");

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
                          "Beta-gated E - Time, beam on only",
                          2, timeResolution, "s");
    DeclareHistogramGranY(betaGated::DD_ENERGY__TIMEX_DECAY,
                          energyBins2, granTimeBins,
                          "Beta-gated E - Time, beam off only",
                          2, timeResolution, "s");
    DeclareHistogramGranY(betaGated::DD_ENERGY__TIMEX,
                          energyBins2, granTimeBins,
                          "Beta-gated E - Time", 2, timeResolution, "s");
    DeclareHistogramGranY(betaGated::DD_ADD_ENERGY__TIMEX,
                          energyBins2, granTimeBins,
                          "Beta-gated addback E - Time",
                          2, timeResolution, "s");
}


bool GeProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    // Clear all events stored in vectors from previous event
    geEvents_.clear();
    for (unsigned i = 0; i < numClovers; ++i)
        addbackEvents_[i].clear();
    tas_.clear();

    static const vector<ChanEvent*> &highEvents = event.GetSummary("ge:clover_high", true)->GetList();
    static const vector<ChanEvent*> &lowEvents  = event.GetSummary("ge:clover_low", true)->GetList();

    for (vector<ChanEvent*>::const_iterator itHigh = highEvents.begin();
	 itHigh != highEvents.end(); itHigh++) {
        int location = (*itHigh)->GetChanID().GetLocation();
        plot(D_ENERGY_HIGHGAIN, (*itHigh)->GetCalEnergy());

        if ( (*itHigh)->IsSaturated() || (*itHigh)->IsPileup() )
            continue;

        // find the matching low gain event
        vector <ChanEvent*>::const_iterator itLow = lowEvents.begin();
        for (; itLow != lowEvents.end(); itLow++) {
            if ( (*itLow)->GetChanID().GetLocation() == location ) {
                break;
            }
        }
        if ( itLow != lowEvents.end() ) {
            double ratio = (*itHigh)->GetEnergy() / (*itLow)->GetEnergy();
            plot(DD_CLOVER_ENERGY_RATIO, location, ratio * 10.);
            if ( (ratio < detectors::geLowRatio ||
                  ratio > detectors::geHighRatio) )
                continue;
        }
        geEvents_.push_back(*itHigh);
    }
    // makes a copy so we can remove (or rearrange) bad events 
    //   based on poorly matched high-low gain energies
    //
    /*
    geEvents_ = sumMap["ge"]->GetList();
    vector<ChanEvent*>::iterator geEnd = geEvents_.end();

    static const vector<ChanEvent*> &highEvents = event.GetSummary("ge:clover_high", true)->GetList();
    static const vector<ChanEvent*> &lowEvents  = event.GetSummary("ge:clover_low", true)->GetList();

    for (vector<ChanEvent*>::const_iterator itHigh = highEvents.begin();
	 itHigh != highEvents.end(); itHigh++) {
        int location = (*itHigh)->GetChanID().GetLocation();
        plot(D_ENERGY_HIGHGAIN, (*itHigh)->GetCalEnergy());

        // Remove saturated and pileups
        if ( (*itHigh)->IsSaturated() || (*itHigh)->IsPileup() ) {
            geEnd = remove(geEvents_.begin(), geEnd, *itHigh);
        }

        // find the matching low gain event
        vector <ChanEvent*>::const_iterator itLow = lowEvents.begin();
        for (; itLow != lowEvents.end(); itLow++) {
            if ( (*itLow)->GetChanID().GetLocation() == location ) {
                break;
            }
        }
        if ( itLow != lowEvents.end() ) {
            double ratio = (*itHigh)->GetEnergy() / (*itLow)->GetEnergy();
            plot(DD_CLOVER_ENERGY_RATIO, location, ratio * 10.);
            if ( (ratio < lowRatio || ratio > highRatio) ) {
                // put these bad events at the end of the vector
                geEnd = remove(geEvents_.begin(), geEnd, *itHigh);
                geEnd = remove(geEvents_.begin(), geEnd, *itLow);
            }
        }
    }
    // Now throw out any remaining clover low-gain events (for now)
    for ( vector<ChanEvent*>::const_iterator itLow = lowEvents.begin(); 
	  itLow != lowEvents.end(); itLow++ ) {
        geEnd = remove(geEvents_.begin(), geEnd, *itLow);
    }

    // this purges the bad events for good from this processor which "remove"
    //   has moved to the end
    geEvents_.erase(geEnd, geEvents_.end());
    */

    /** NOTE we do permanents changes to events here
     *  Necessary in order to set corrected time for use in correlator
     */
    for (vector<ChanEvent*>::iterator it = geEvents_.begin(); 
	 it != geEvents_.end(); it++) {
        double energy = (*it)->GetCalEnergy();
        double time   = (*it)->GetTime() - WalkCorrection(energy);	
        (*it)->SetCorrectedTime(time);
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
    double refTime = -2.0 * detectors::subEventWindow; 

    for (vector<ChanEvent*>::iterator it = geEvents_.begin(); it != geEvents_.end(); it++) {
        ChanEvent *chan = *it;
        double energy = chan->GetCalEnergy(); 
        double time = chan->GetCorrectedTime();
        int clover = leafToClover[chan->GetChanID().GetLocation()];

        /**
        * Do not take into account events with too low energy
        * (avoid summing of noise with real gammas)
        */ 
        if (energy < detectors::addbackEnergyCut)
            continue;

        // entries in map are sorted by time
        // if event time is outside of subEventWindow, we start new 
        //   events for all clovers and "tas"
        double dtime = abs(time - refTime) * pixie::clockInSeconds;
        if (dtime > detectors::subEventWindow) {
            for (unsigned i = 0; i < numClovers; ++i) {
                addbackEvents_[i].push_back(pair<double, double>());
            }
            tas_.push_back(pair<double, double>());
        }
        // Total addback energy
        addbackEvents_[clover].back().first += energy;
        // We store latest time only
        addbackEvents_[clover].back().second   = time;
        tas_.back().first += energy;
        tas_.back().second   = time;
        refTime = time;
    }

    return true;
} 

bool GeProcessor::Process(RawEvent &event) {
    using namespace dammIds::ge;

    if (!EventProcessor::Process(event))
        return false;

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
    bool hasBeta = TreeCorrelator::get()->place("Beta")->status();

    // Good gamma multiplicity
    plot(D_MULT, geEvents_.size());

    // Note that geEvents_ vector holds only good events (matched
    // low & high gain). See PreProcess
    for (vector<ChanEvent*>::iterator it = geEvents_.begin(); 
	 it != geEvents_.end(); it++) {
        ChanEvent* chan = *it;
        
        double gEnergy = chan->GetCalEnergy();	
        if (gEnergy < detectors::gammaThreshold) 
            continue;

        double gTime = chan->GetCorrectedTime();
        double decayTime = (gTime - cycleTime) *
                           pixie::clockInSeconds;
        int det = leafToClover[chan->GetChanID().GetLocation()];

        plot(D_ENERGY, gEnergy);
        plot(D_ENERGY_CLOVERX + det, gEnergy);
        granploty(DD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);

        double gb_dtime = numeric_limits<double>::max();
        if (hasBeta) {
            pair<double, EventData> bestBeta = BestBetaForGamma(gTime);
            gb_dtime = bestBeta.first;
            double betaEnergy = bestBeta.second.energy;
            int betaLocation = bestBeta.second.location;

            double plotResolution = 10e-9;
            plot(betaGated::DD_TDIFF__GAMMA_ENERGY,
                    (int)(gb_dtime / plotResolution + 100), gEnergy);
            plot(betaGated::DD_TDIFF__BETA_ENERGY, 
                    (int)(gb_dtime / plotResolution + 100), betaEnergy); 

            if (GoodGammaBeta(gb_dtime)) {
                plot(betaGated::D_ENERGY, gEnergy);
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
                         pixie::clockInSeconds;
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
        
        for (vector<ChanEvent*>::const_iterator it2 = it + 1;
                it2 != geEvents_.end(); it2++) {
            ChanEvent* chan2 = *it2;

            double gEnergy2 = chan2->GetCalEnergy();            
            int det2 = leafToClover[chan2->GetChanID().GetLocation()];
            double gTime2 = chan2->GetCorrectedTime();
            if (gEnergy2 < detectors::gammaThreshold) 
                continue;

            double gg_dtime = (gTime2 - gTime) * pixie::clockInSeconds;

            /** Plot timediff between events in the same clover
             * to monitor addback subevent gates. */
            if (det == det2) {
                double plotResolution = 10e-9;
                plot(DD_TDIFF__GAMMA_GAMMA_ENERGY, 
                     (int)(gg_dtime / plotResolution + 100),
                     gEnergy);
                plot(DD_TDIFF__GAMMA_GAMMA_ENERGY_SUM, 
                      (int)(gg_dtime / plotResolution + 100),
                      gEnergy + gEnergy2);
            }

            symplot(DD_ENERGY, gEnergy, gEnergy2);
            if (hasBeta) {
                if (GoodGammaBeta(gb_dtime)) {
                    symplot(betaGated::DD_ENERGY, gEnergy,
                            gEnergy2);            
                } else if (gb_dtime > detectors::gammaBetaLimit) {
                        symplot(betaGated::DD_ENERGY_BDELAYED,
                                gEnergy, gEnergy2);
                }
            }

            if (abs(gg_dtime) < detectors::gammaGammaLimit) {
                symplot(DD_ENERGY_PROMPT, gEnergy, gEnergy2);
                if (decayTime > detectors::earlyLowLimit) { 
                    if (decayTime < detectors::earlyHighLimit) 
                        symplot(DD_ENERGY_PROMPT_EARLY, 
                                gEnergy, gEnergy2);            
                    else
                        symplot(DD_ENERGY_PROMPT_LATE, 
                                gEnergy, gEnergy2);            
                }

                if (hasBeta && GoodGammaBeta(gb_dtime)) {
                    symplot(betaGated::DD_ENERGY_PROMPT, 
                            gEnergy, gEnergy2);            
                    if (decayTime > detectors::earlyLowLimit) { 
                        if (decayTime < detectors::earlyHighLimit) 
                            symplot(betaGated::DD_ENERGY_PROMPT_EARLY, 
                                    gEnergy, gEnergy2);            
                        else
                            symplot(betaGated::DD_ENERGY_PROMPT_LATE, 
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

                    double plotResolution = 10e-9;
                    plot(DD_TDIFF__GATEX, 
                         (int)(gg_dtime / plotResolution + 100), ig);
                    if (hasBeta && GoodGammaBeta(gb_dtime))
                        plot(betaGated::DD_TDIFF__GATEX, 
                            (int)(gg_dtime / plotResolution + 100), ig);

                    /** Only fast coincidences for other g-g gates histograms 
                    if (abs(gg_dtime) < detectors::gammaGammaLimit)
                        continue;*/

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
                        if (gEnergy3 < detectors::gammaThreshold)
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
        double gEnergy = tas_[i].first;
        if (gEnergy < detectors::gammaThreshold)
            continue;
        plot(D_ADD_ENERGY_TOTAL, gEnergy);
        if (hasBeta)
            plot(betaGated::D_ADD_ENERGY_TOTAL, gEnergy);
    }

    // Plot addback spectra 
    for (unsigned int ev = 0; ev < nEvents; ev++) {
        for (unsigned int det = 0; det < numClovers; ++det) {
            double gEnergy = addbackEvents_[det][ev].first;
            double gTime = addbackEvents_[det][ev].second;
            double decayTime = (gTime - cycleTime) * pixie::clockInSeconds;
            if (gEnergy < detectors::gammaThreshold)
                continue;

            plot(D_ADD_ENERGY, gEnergy);
            plot(D_ADD_ENERGY_CLOVERX + det, gEnergy);
            granploty(DD_ADD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);

            if (hasBeta) {
                plot(betaGated::D_ADD_ENERGY, gEnergy);
                plot(betaGated::D_ADD_ENERGY_CLOVERX + det, gEnergy);
                granploty(betaGated::DD_ADD_ENERGY__TIMEX, gEnergy,
                          decayTime, timeResolution);
            }

            for (unsigned int det2 = det + 1; 
                    det2 < numClovers; ++det2) {
                double gEnergy2 = addbackEvents_[det2][ev].first;
                if (gEnergy2 < detectors::gammaThreshold)
                    continue;

                symplot(DD_ADD_ENERGY, gEnergy, gEnergy2);

                if (hasBeta) {
                        symplot(betaGated::DD_ADD_ENERGY, gEnergy, gEnergy2);
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
