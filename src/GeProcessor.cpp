/** \file GeProcessor.cpp
 * \brief Implementation for germanium processor
 * \author David Miller
 * \date August 2009
 * <STRONG> Modified: </STRONG> Krzysztof Miernik - May 2012
 */

//? Make a clover specific processor
//This would probably be a good idea -SVP

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>

#include <limits>
#include <cmath>
#include <cstdlib>


#include "Plots.hpp"
#include "PlotsRegister.hpp"
#include "DammPlotIds.hpp"

#include "Correlator.hpp"
#include "DetectorLibrary.hpp"
#include "GeProcessor.hpp"
#include "RawEvent.hpp"

using namespace std;

/**
 *  Perform walk correction for gamma-ray timing based on its energy
 */
double GeProcessor::WalkCorrection(double e) {
    if(e <= 100) {
        const double a0 = 118.482;
        const double a1 = -2.66767;
        const double a2 = 0.028492;
        const double a3 = -0.000107801;

        const double c  = -40.2608;
        const double e0 = 12.5426;
        const double k  = 3.84917;
        
        return(a0 + a1 * e + a2 * pow(e, 2) + a3 * pow(e, 3) +
               c / (1.0 + exp( (e - e0) / k) ));
    }else {
        const double a0 = 11.2287;
        const double a1 = -0.000939443;
        
        const double b0 = 38.8912;
        const double b1 = -0.121784;
        const double b2 = 0.000268339;

        const double k  = 180.146;
        
        return(a0 + a1 * e + (b0 + b1 * e + b2 * pow(e, 2)) * exp(-e / k));
    }
}

double GeProcessor::GammaBetaDtime(double gTime) {
    PlaceOR* betas = dynamic_cast<PlaceOR*>(
                        TreeCorrelator::get()->place("Beta"));
    if (betas->info_.size() == 0)
        return numeric_limits<double>::max();

    vector<double> dtime;
    for (deque<CorrEventData>::iterator it = betas->info_.begin();
         it != betas->info_.end();
         ++it) {
        dtime.push_back((gTime - it->time) * pixie::clockInSeconds);
    }
    return *min_element(dtime.begin(),dtime.end(), CompareTimes());
}

bool GeProcessor::GoodGammaBeta(double gTime, 
                        double limit_in_sec /* = detectors::gammaBetaLimit*/) {
    if (GammaBetaDtime(gTime) > limit_in_sec)
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
    timeResolution.push_back(10e-3); 

#ifdef GGATES
    /* 
     * Load GammaGates.txt defining gamma-gamma gates for anular
     * distributions and for g-g-g spectra
     *
     */
    ifstream ggFile("GammaGates.txt");
    string line;
    while (getline(ggFile, line) ) {
        istringstream iss;
        iss.str(line);
        if (line[0] != '#' && line.size() > 1) {
            vector<double> gms;
            double g;
            while (iss >> g) {
                gms.push_back(g);
            }
            GGate gate;
            if (gms.size() == 2) {
                double dE = 1.0;
                gate.g1min = gms[0] - dE;
                gate.g1max = gms[0] + dE;
                gate.g2min = gms[1] - dE;
                gate.g2max = gms[1] + dE;
            } else if (gms.size() == 4) {
                gate.g1min = gms[0];
                gate.g1max = gms[1];
                gate.g2min = gms[2];
                gate.g2max = gms[3];
            } else {
                cout << "Incomplete gates definition" << endl;
            }
            if (gate.check()) {
                gGates.push_back(gate);
                cout << "(" << gate.g1min << ", " << gate.g1max << ") <-> " 
                     << "(" << gate.g2min << ", " << gate.g2max << ") " 
                     << " gate loaded " << endl;
            } else {
                cout << "Wrong gates definition" << endl;
            }
        }
    }
    ggFile.close();
#endif
}

/** Declare plots including many for decay/implant/neutron gated analysis  */
void GeProcessor::DeclarePlots(void) 
{
    const int energyBins1  = SE;
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
        //print statement
        cout << "A total of " << cloverChans << " clover channels were detected: ";
        int lastClover = numeric_limits<int>::min();
        for ( map<int, int>::const_iterator it = leafToClover.begin();
            it != leafToClover.end(); it++ ) {
            if (it->second != lastClover) {
                lastClover = it->second;
                cout << endl << "  " << lastClover << " : ";
            } else {
                cout << ", ";
            }
                cout << setw(2) << it->first ;
        }

        if (numClovers > dammIds::ge::MAX_CLOVERS) {
            cout << "This is greater than MAX_CLOVERS for spectra definition."
            << "  Check the spectrum definition file and try again." << endl;
            exit(EXIT_FAILURE);
        }   
    }

    for (unsigned i = 0; i < numClovers; ++i) {
        vector< pair<double, double> > empty;
        addbackEvents_.push_back(empty);
    }

    DeclareHistogram1D(D_ENERGY                 , energyBins1, "Gamma singles");
    DeclareHistogram1D(betaGated::D_ENERGY      , energyBins1, "Beta gated gamma");
    DeclareHistogram1D(betaGated::D_ENERGY_BETA0, energyBins1, "Gamma beta0 gate");
    DeclareHistogram1D(betaGated::D_ENERGY_BETA1, energyBins1, "Gamma beta1 gate");
    DeclareHistogram1D(D_ENERGY_LOWGAIN         , energyBins1, "Gamma singles, low gain");
    DeclareHistogram1D(D_ENERGY_HIGHGAIN        , energyBins1, "Gamma singles, high gain");
    DeclareHistogram1D(D_MULT, S3, "Gamma multiplicity");                  

    DeclareHistogram1D(D_ADD_ENERGY                 , energyBins1, "Gamma addback");
    DeclareHistogram1D(betaGated::D_ADD_ENERGY      , energyBins1, "Beta gated gamma addback");
    DeclareHistogram1D(D_ADD_ENERGY_TOTAL           , energyBins1, "Gamma total");
    DeclareHistogram1D(betaGated::D_ADD_ENERGY_TOTAL, energyBins1, "Beta gated gamma total");
    
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
    DeclareHistogram2D(betaGated::DD_ENERGY,
                       energyBins2, energyBins2, "Gamma gamma beta gated");
    DeclareHistogram2D(DD_ADD_ENERGY,
                       energyBins2, energyBins2, "Gamma gamma addback");
    DeclareHistogram2D(DD_ADD_ENERGY_EARLY,
            energyBins2, energyBins2, "Gamma gamma addback early decay cycle");
    DeclareHistogram2D(DD_ADD_ENERGY_LATE,
            energyBins2, energyBins2, "Gamma gamma addback late decay cycle");

    DeclareHistogram2D(betaGated::DD_ADD_ENERGY,
            energyBins2, energyBins2, "Beta-gated Gamma gamma addback");
    DeclareHistogram2D(betaGated::DD_ADD_ENERGY_DELAYED,
      energyBins2, energyBins2, "Beta-gated Gamma gamma addback, beta delayed");
    DeclareHistogram2D(betaGated::DD_ADD_ENERGY_EARLY, energyBins2, energyBins2, "Beta-gated Gamma gamma addback early decay cycle");
    DeclareHistogram2D(betaGated::DD_ADD_ENERGY_LATE, energyBins2, energyBins2, "Beta-gated Gamma gamma addback late cycle");


    DeclareHistogram2D(DD_TDIFF__GAMMA_GAMMA_ENERGY, timeBins1,
                energyBins2, "Gamma energy, gamma-gamma same clover time diff + 100 (10 ns)");
    DeclareHistogram2D(DD_TDIFF__GAMMA_GAMMA_ENERGY_SUM, timeBins1,
            energyBins2, "Sum Gamma energy, gamma-gamma same clover time diff + 100 (10 ns)");
    DeclareHistogram2D(betaGated::DD_TDIFF__GAMMA_ENERGY, timeBins1,
                       energyBins2, "Gamma energy, beta time diff + 100 (10 ns)");
    DeclareHistogram2D(betaGated::DD_TDIFF__BETA_ENERGY, timeBins1,
                       energyBins2, "Beta energy, gamma time diff + 100 (10 ns)");

    DeclareHistogram2D(DD_CLOVER_ENERGY_RATIO, S4, S6, "high/low energy ratio (x10)");

#ifdef GGATES
    DeclareHistogram2D(DD_ANGLE__GATEX, S2, S5,
                       "g_g angular distrubution vs g_g gate number");
    DeclareHistogram2D(betaGated::DD_ANGLE__GATEX, S2, S5,
                     "beta gated g_g angular distrubution vs g_g gate number");
    DeclareHistogram2D(DD_ENERGY__GATEX, energyBins2, S5,
                       "g_g gated gamma energy");
    DeclareHistogram2D(betaGated::DD_ENERGY__GATEX, energyBins2, S5,
                       "g_g_beta gated gamma energy");
#endif

    DeclareHistogramGranY(DD_ENERGY__TIMEX               , energyBins2, granTimeBins, "E - Time", 2, timeResolution, "s");
    DeclareHistogramGranY(DD_ADD_ENERGY__TIMEX           , energyBins2, granTimeBins, "Addback E - Time", 2, timeResolution, "s");
    DeclareHistogramGranY(betaGated::DD_ENERGY__TIMEX    , energyBins2, granTimeBins, "Beta-gated E - Time", 2, timeResolution, "s");
    DeclareHistogramGranY(betaGated::DD_ADD_ENERGY__TIMEX, energyBins2, granTimeBins, "Beta-gated addback E - Time", 2, timeResolution, "s");
}


bool GeProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    // Clear all events stored in vectors from previous event
    geEvents_.clear();
    for (unsigned i = 0; i < numClovers; ++i)
        addbackEvents_[i].clear();
    tas_.clear();

    // makes a copy so we can remove (or rearrange) bad events 
    //   based on poorly matched high-low gain energies
    geEvents_ = sumMap["ge"]->GetList();
    vector<ChanEvent*>::iterator geEnd = geEvents_.end();

    //   correspond properly to raw low gain energy
    const double lowRatio = 1.5, highRatio = 3.0;
    static const vector<ChanEvent*> &highEvents = event.GetSummary("ge:clover_high", true)->GetList();
    static const vector<ChanEvent*> &lowEvents  = event.GetSummary("ge:clover_low", true)->GetList();

    for (vector<ChanEvent*>::const_iterator itHigh = highEvents.begin();
	 itHigh != highEvents.end(); itHigh++) {
        
        //Purge the cases where the high gain saturated.
        if((*itHigh)->IsSaturated()) {
            geEnd = remove(geEvents_.begin(), geEnd, *itHigh);
            continue;
        }
        
        // find the matching low gain event
        int location = (*itHigh)->GetChanID().GetLocation();
        plot(D_ENERGY_HIGHGAIN, (*itHigh)->GetCalEnergy());
        vector <ChanEvent*>::const_iterator itLow = lowEvents.begin();
        for (; itLow != lowEvents.end(); itLow++) {
            if ( (*itLow)->GetChanID().GetLocation() == location ) {
                break;
            }
        }
        if ( itLow != lowEvents.end() ) {
            double ratio = (*itHigh)->GetEnergy() / (*itLow)->GetEnergy();
            plot(DD_CLOVER_ENERGY_RATIO, location, ratio * 10.);
            if (ratio < lowRatio || ratio > highRatio) {
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

    // tapeMove is true if the tape is moving
    bool tapeMove = TreeCorrelator::get()->place("TapeMove")->status();

    // If the tape is moving there is no need of analyzing events
    // as they must belong to background
    if (tapeMove)
        return true;

    // beamOn is true for beam on and false for beam off
    bool beamOn =  TreeCorrelator::get()->place("Beam")->status();

    bool hasBeta = TreeCorrelator::get()->place("Beta")->status();
    bool hasBeta0 = TreeCorrelator::get()->place("scint_beta_0")->status();
    bool hasBeta1 = TreeCorrelator::get()->place("scint_beta_1")->status();
    double betaEnergy = -1;
    if (hasBeta) {
        double betaEnergy0 = -1;
        if (hasBeta0) {
            betaEnergy0 = TreeCorrelator::get()->place("scint_beta_0")->last().energy;
        }
        double betaEnergy1 = -1;
        if (hasBeta1) {
            betaEnergy1 = TreeCorrelator::get()->place("scint_beta_1")->last().energy;
        }
        betaEnergy = max(betaEnergy0, betaEnergy1);
    }
    
    // Cycle time is measured from the begining of last beam on event
    double cycleTime = TreeCorrelator::get()->place("Cycle")->last().time;

    // Note that geEvents_ vector holds only good events (matched
    // low & high gain). See PreProcess
    for (vector<ChanEvent*>::iterator it = geEvents_.begin(); 
	 it != geEvents_.end(); it++) {
        ChanEvent *chan = *it;
        
        double gEnergy = chan->GetCalEnergy();	
        double gTime = chan->GetCorrectedTime();
        double decayTime = (gTime - cycleTime) 
                           * pixie::clockInSeconds;
        int det = leafToClover[chan->GetChanID().GetLocation()];
        if (gEnergy < detectors::gammaThreshold) 
            continue;

        plot(D_ENERGY, gEnergy);
        plot(D_ENERGY_CLOVERX + det, gEnergy);
        granploty(DD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);

        if (hasBeta) {
            double dtime = GammaBetaDtime(gTime);

            double plotResolution = 10e-9;
            plot(betaGated::DD_TDIFF__GAMMA_ENERGY,
                    (int)(dtime / plotResolution + 100), gEnergy);
            /** Notice that betaEnergy here is not good one!
             * But this plot is approx. anyway and for tuning only*/
            plot(betaGated::DD_TDIFF__BETA_ENERGY, 
                    (int)(dtime / plotResolution + 100), betaEnergy); 

            if (GoodGammaBeta(gTime)) {
                plot(betaGated::D_ENERGY, gEnergy);
                plot(betaGated::D_ENERGY_CLOVERX + det, gEnergy);
                granploty(betaGated::DD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);

                // individual beta gamma coinc spectra for each beta detector
                if (hasBeta0 == 0) {
                    plot(betaGated::D_ENERGY_BETA0, gEnergy);
                } else if (hasBeta1 == 1) {
                    plot(betaGated::D_ENERGY_BETA1, gEnergy);
                }
            }
        }
        
        for (vector<ChanEvent*>::const_iterator it2 = it + 1;
                it2 != geEvents_.end(); it2++) {
            ChanEvent *chan2 = *it2;

            double gEnergy2 = chan2->GetCalEnergy();            
            int det2 = leafToClover[chan2->GetChanID().GetLocation()];
            if (gEnergy2 < detectors::gammaThreshold) 
                continue;

            /** Plot timediff between events in the same clover
             * to monitor addback subevent gates. */
            if (det == det2) {
                double gTime2 = chan2->GetCorrectedTime();
                double plotResolution = 10e-9;
                double dtime = (gTime2 - gTime) * pixie::clockInSeconds;
                plot(DD_TDIFF__GAMMA_GAMMA_ENERGY, 
                        (int)(dtime / plotResolution + 100), gEnergy);
                plot(DD_TDIFF__GAMMA_GAMMA_ENERGY_SUM, 
                      (int)(dtime / plotResolution + 100), gEnergy + gEnergy2);
            }

            symplot(DD_ENERGY, gEnergy, gEnergy2);
            if (hasBeta && GoodGammaBeta(gTime))
                symplot(betaGated::DD_ENERGY, gEnergy, gEnergy2);            
        } // iteration over other gammas
    } 
    // Vectors tas and addbackEvents should have the same size
    unsigned nEvents = tas_.size();

    // Plot 'tas' spectra
    for (unsigned i = 0; i < nEvents; ++i) {
        double gEnergy = tas_[i].first;
        double gTime = tas_[i].second;
        if (gEnergy < detectors::gammaThreshold)
            continue;
        plot(D_ADD_ENERGY_TOTAL, gEnergy);
        if (hasBeta && GoodGammaBeta(gTime))
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

            if (hasBeta && GoodGammaBeta(gTime)) {
                plot(betaGated::D_ADD_ENERGY, gEnergy);
                plot(betaGated::D_ADD_ENERGY_CLOVERX + det, gEnergy);
                granploty(betaGated::DD_ADD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
            }

            for (unsigned int det2 = det + 1; 
                    det2 < numClovers; ++det2) {
                double gEnergy2 = addbackEvents_[det2][ev].first;
                if (gEnergy2 < detectors::gammaThreshold)
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
                    double decayTime = (gTime - 
                         TreeCorrelator::get()->place("Beam")->last().time) *
                         pixie::clockInSeconds;
                    if (decayTime < decayCycleEarly) {
                        symplot(DD_ADD_ENERGY_EARLY, gEnergy, gEnergy2);
                        if (hasBeta && GoodGammaBeta(gTime)) 
                            symplot(betaGated::DD_ADD_ENERGY_EARLY,
                                    gEnergy, gEnergy2);
                    } else if (decayTime < decayCycleEnd) {
                        symplot(DD_ADD_ENERGY_LATE, gEnergy, gEnergy2);
                        if (hasBeta && GoodGammaBeta(gTime)) 
                            symplot(betaGated::DD_ADD_ENERGY_LATE,
                                    gEnergy, gEnergy2);
                    }
                }
                if (hasBeta) {
                    if (GoodGammaBeta(gTime)) {
                        symplot(betaGated::DD_ADD_ENERGY, gEnergy, gEnergy2);
                    } else {
                        if (GammaBetaDtime(gTime) > detectors::gammaBetaLimit)
                            symplot(betaGated::DD_ADD_ENERGY_DELAYED,
                                    gEnergy, gEnergy2);
                    }
                }
                    
#ifdef GGATES
                    /**
                     * Gamma-gamma angular and gamma-gamma gate
                     */
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
                                if (hasBeta && GoodGammaBeta(gTime))
                                    plot(betaGated::DD_ANGLE__GATEX, 1, ig);
                            } else {
                            plot(DD_ANGLE__GATEX, 2, ig);
                                if (hasBeta && GoodGammaBeta(gTime))
                                    plot(betaGated::DD_ANGLE__GATEX, 2, ig);
                            }
                            for (unsigned det3 = det2 + 1;
                                    det3 < numClovers; ++det3) {
                                double gEnergy3 = addbackEvents_[det3][ev].first;
                                if (gEnergy3 < detectors::gammaThreshold)
                                    continue;
                                plot(DD_ENERGY__GATEX, gEnergy3, ig);
                                if (hasBeta && GoodGammaBeta(gTime))
                                    plot(betaGated::DD_ENERGY__GATEX, gEnergy3, ig);
                            }
                        }
                    } //for ig
#endif

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
	histo.DeclareHistogram2D(dammId + i, xsize, ysize, fullTitle.str().c_str(), halfWordsPerChan, 1, 1);
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
