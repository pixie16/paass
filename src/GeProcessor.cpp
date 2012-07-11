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
#include "RawEvent.hpp"

using namespace std;

// Clover hits contain the total energy and walk-corrected time of last energy deposit
DEFINE_ALIASED_PAIR(CloverHit, double, double, time, energy);

namespace dammIds {
    namespace ge {
	// clovers
	const unsigned int MAX_CLOVERS = 4; // for *_DETX spectra

    /*
     * Beta offset = 10
     * Decay offset = 20
     * Addback offset = 50
     */

	const int D_ENERGY              = 0;
	const int D_ENERGY_CLOVERX      = 1; // leaf by clover for X detectors

	const int D_ENERGY_LOWGAIN      = 7;
	const int D_ENERGY_HIGHGAIN     = 8;
	const int D_MULT                = 9;

	const int D_ENERGY_WITH_DECAY   = 20;
	// These spectra are squeezed into a gap in IDs
	const int D_ENERGY_WITH_IMPLANT = 25;
	const int D_ENERGY_WITH_EXT0    = 26; // with external detector (e.g. 3hen, vandle) 
	const int D_ENERGY_WITH_EXT1    = 27; // with external detector (e.g. 3hen, vandle) 

	const int D_ADD_ENERGY          = 50; 
	const int D_ADD_ENERGY_CLOVERX  = 51;
	const int D_ADD_ENERGY_TOTAL    = 55;

	// 2D spectra
	const int DD_ENERGY                = 100;
	const int DD_CLOVER_ENERGY_RATIO   = 107;
	const int DD_ENERGY_WITH_DECAY     = 120;
	const int DD_ADD_ENERGY            = 150;
	const int DD_ADD_ENERGY_EARLY      = 151;
	const int DD_ADD_ENERGY_LATE       = 152;
    // Gamma-Gamma angular distribution
    const int DD_ANGLE__GATEX         = 155;
    const int DD_ENERGY__GATEX        = 156;

	const int DD_ADD_ENERGY_WITH_DECAY = 170;
	// note these only make sense with decay 
	const int DD_ENERGY__TIMEX           = 121; // with x granularities
	const int DD_ADD_ENERGY__TIMEX       = 171; // with x granularities

	// corresponds to ungated specra ID's + 10 where applicable
	namespace betaGated {
	    const int D_ENERGY             = 10;
	    const int D_ENERGY_CLOVERX     = 11;
	    const int D_ENERGY_BETA0       = 15; 
	    const int D_ENERGY_BETA1       = 16; 
	    const int D_ENERGY_WITH_EXT0   = 36; 
	    const int D_ENERGY_WITH_EXT1   = 37; 
	    const int D_ADD_ENERGY         = 60; 
	    const int D_ADD_ENERGY_CLOVERX = 61; 
	    const int D_ADD_ENERGY_TOTAL   = 65; 

	    // 2d spectra
	    const int DD_ENERGY              = 110; 
	    const int DD_TDIFF__GAMMA_ENERGY = 105;
	    const int DD_TDIFF__BETA_ENERGY  = 106;
	    const int DD_ADD_ENERGY          = 160; 
        const int DD_ADD_ENERGY_EARLY    = 161;
        const int DD_ADD_ENERGY_LATE     = 162;
	    const int DD_ADD_ENERGY_PROMPT   = 163;
	    const int DD_ADD_ENERGY_DELAYED  = 164;
        const int DD_ANGLE__GATEX        = 165; 
        const int DD_ENERGY__GATEX       = 166; 
	    const int DD_ENERGY__TIMEX       = 131; 
	    const int DD_ADD_ENERGY__TIMEX   = 181;
	}
    } // end namespace ge
}

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

/** 
 * Initialize processor and determine number of clovers from global Identifiers
 */
bool GeProcessor::Init(DetectorDriver &driver)
{
    if (!EventProcessor::Init(driver))
	return false;

    /* clover specific routine, determine the number of clover detector
       channels and divide by four to find the total number of clovers
    */
    extern DetectorLibrary modChan;

    const set<int> &cloverLocations = modChan.GetLocations("ge", "clover_high");
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
	int lastClover = INT_MIN;
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

    return true;
}

/** Declare plots including many for decay/implant/neutron gated analysis  */
void GeProcessor::DeclarePlots(void) 
{
    const int energyBins1  = SE;
    const int energyBins2  = SC;
    const int timeBins2    = SA;
    const int granTimeBins = SA;

    using namespace dammIds::ge;

    DeclareHistogram1D(D_ENERGY                 , energyBins1, "Gamma singles");
    DeclareHistogram1D(betaGated::D_ENERGY      , energyBins1, "Beta gated gamma");
    DeclareHistogram1D(betaGated::D_ENERGY_BETA0, energyBins1, "Gamma beta0 gate");
    DeclareHistogram1D(betaGated::D_ENERGY_BETA1, energyBins1, "Gamma beta1 gate");
    DeclareHistogram1D(D_ENERGY_LOWGAIN         , energyBins1, "Gamma singles, low gain");
    DeclareHistogram1D(D_ENERGY_HIGHGAIN        , energyBins1, "Gamma singles, high gain");
    DeclareHistogram1D(D_ENERGY_WITH_DECAY      , energyBins1, "Decay gamma singles");
    DeclareHistogram1D(D_ENERGY_WITH_IMPLANT    , energyBins1, "Implant gated gamma");
    DeclareHistogram1D(D_ENERGY_WITH_EXT0       , energyBins1, "Gamma singles with external 0");
    DeclareHistogram1D(D_ENERGY_WITH_EXT1       , energyBins1, "Gamma singles with external 1");
    DeclareHistogram1D(betaGated::D_ENERGY_WITH_EXT0, energyBins1, "Gamma singles with beta-ext0");
    DeclareHistogram1D(betaGated::D_ENERGY_WITH_EXT1, energyBins1, "Gamma singles with beta-ext1");
    
    DeclareHistogram1D(D_MULT, S3, "Gamma multiplicity");                  

    DeclareHistogram1D(D_ADD_ENERGY                 , energyBins1, "Gamma addback");
    DeclareHistogram1D(betaGated::D_ADD_ENERGY      , energyBins1, "Beta gated gamma addback");
    DeclareHistogram1D(D_ADD_ENERGY_TOTAL           , energyBins1, "Gamma total");
    DeclareHistogram1D(betaGated::D_ADD_ENERGY_TOTAL, energyBins1, "Beta gated gamma total");
    
    // for each clover
    for (unsigned int i=0; i < numClovers; i++) {
        stringstream ss;
        ss << "Clover " << i << " gamma";
        DeclareHistogram1D(D_ENERGY_CLOVERX + i, energyBins1, ss.str().c_str());

        ss.str("");
        ss << "Clover " << i << " beta gated gamma";
        DeclareHistogram1D(betaGated::D_ENERGY_CLOVERX + i, energyBins1, ss.str().c_str());

	ss.str("");
	ss << "Clover " << i << " beta gated gamma addback";
        DeclareHistogram1D(betaGated::D_ADD_ENERGY_CLOVERX + i, energyBins1, ss.str().c_str());
    }

    DeclareHistogram2D(DD_ENERGY                       , energyBins2, energyBins2, "Gamma gamma");
    DeclareHistogram2D(betaGated::DD_ENERGY            , energyBins2, energyBins2, "Gamma gamma beta gated");
    DeclareHistogram2D(DD_ENERGY_WITH_DECAY            , energyBins2, energyBins2, "Gamma gamma decay gated");
    DeclareHistogram2D(DD_ADD_ENERGY                   , energyBins2, energyBins2, "Gamma gamma addback");
    DeclareHistogram2D(DD_ADD_ENERGY_EARLY             , energyBins2, energyBins2, "Gamma gamma addback");
    DeclareHistogram2D(DD_ADD_ENERGY_LATE              , energyBins2, energyBins2, "Gamma gamma addback");

    DeclareHistogram2D(betaGated::DD_ADD_ENERGY        , energyBins2, energyBins2, "Beta-gated Gamma gamma addback");
    DeclareHistogram2D(betaGated::DD_ADD_ENERGY_PROMPT , energyBins2, energyBins2, "Beta-gated Gamma gamma addback, beta prompt");
    DeclareHistogram2D(betaGated::DD_ADD_ENERGY_DELAYED, energyBins2, energyBins2, "Beta-gated Gamma gamma addback, beta delayed");
    DeclareHistogram2D(betaGated::DD_ADD_ENERGY_EARLY   , energyBins2, energyBins2, "Beta-gated Gamma gamma addback early");
    DeclareHistogram2D(betaGated::DD_ADD_ENERGY_LATE    , energyBins2, energyBins2, "Beta-gated Gamma gamma addback late");

    DeclareHistogram2D(DD_ADD_ENERGY_WITH_DECAY        , energyBins2, energyBins2, "Gamma gamma addback, decay gated");

    DeclareHistogram2D(betaGated::DD_TDIFF__GAMMA_ENERGY, timeBins2, energyBins2, "Gamma energy, beta time diff");
    DeclareHistogram2D(betaGated::DD_TDIFF__BETA_ENERGY, timeBins2, energyBins2, "Beta energy, gamma time diff"); //? reduce energy bins

    DeclareHistogram2D(DD_CLOVER_ENERGY_RATIO, S4, S6, "high/low energy ratio (x10)");

    DeclareHistogram2D(DD_ANGLE__GATEX, S2, S5, "g_g angular distrubution vs g_g gate number");
    DeclareHistogram2D(betaGated::DD_ANGLE__GATEX, S2, S5, "beta gated g_g angular distrubution vs g_g gate number");
    DeclareHistogram2D(DD_ENERGY__GATEX, energyBins2, S5, "g_g gated gamma energy");
    DeclareHistogram2D(betaGated::DD_ENERGY__GATEX, energyBins2, S5, "g_g_beta gated gamma energy");

    DeclareHistogramGranY(DD_ENERGY__TIMEX               , energyBins2, granTimeBins, "E - Time", 2, timeResolution, "s");
    DeclareHistogramGranY(DD_ADD_ENERGY__TIMEX           , energyBins2, granTimeBins, "Addback E - Time", 2, timeResolution, "s");
    DeclareHistogramGranY(betaGated::DD_ENERGY__TIMEX    , energyBins2, granTimeBins, "Beta-gated E - Time", 2, timeResolution, "s");
    DeclareHistogramGranY(betaGated::DD_ADD_ENERGY__TIMEX, energyBins2, granTimeBins, "Beta-gated addback E - Time", 2, timeResolution, "s");
}

/** process the event */
bool GeProcessor::Process(RawEvent &event) {
    using namespace dammIds::ge;

    if (!EventProcessor::Process(event))
        return false;
    
    // makes a copy so we can remove (or rearrange) bad events 
    //   based on poorly matched high-low gain energies
    vector<ChanEvent*> geEvents = sumMap["ge"]->GetList();
    vector<ChanEvent*>::iterator geEnd = geEvents.end();

    static const DetectorSummary *betaSummary = event.GetSummary("scint:beta");
    
    bool hasDecay = 
	(event.GetCorrelator().GetCondition() == Correlator::VALID_DECAY);
    bool hasImplant =
	(event.GetCorrelator().GetCondition() == Correlator::VALID_IMPLANT);
    double betaEnergy = NAN;
    double betaTime = NAN;
    int betaLoc = INT_MIN;
    bool hasExt0 = false;
    bool hasExt1 = false;

    // find all beta and neutron signals for ge gating
    if (betaSummary) {	
        for (vector<ChanEvent*>::const_iterator it = betaSummary->GetList().begin();
            it != betaSummary->GetList().end(); it++) {
            ChanEvent *chan = *it;

	    betaEnergy = chan->GetCalEnergy();
	    betaLoc    = chan->GetChanID().GetLocation();
	    betaTime   = chan->GetTime();
        }
    } // beta summary present
    const double betaThresh = 10;
    bool hasBeta = (betaEnergy > betaThresh);

    //? make a search for tagged events for external detectors

    // reject events for clover where raw high gain energy does not
    //   correspond properly to raw low gain energy
    const double lowRatio = 1.5, highRatio = 3.0;
    static const vector<ChanEvent*> &highEvents = event.GetSummary("ge:clover_high", true)->GetList();
    static const vector<ChanEvent*> &lowEvents  = event.GetSummary("ge:clover_low", true)->GetList();

    // first plot low gain energies
    // 
    for (vector<ChanEvent*>::const_iterator itLow  = lowEvents.begin();
	 itLow != lowEvents.end(); itLow++) {
        plot(D_ENERGY_LOWGAIN, (*itLow)->GetCalEnergy());
    }
    for (vector<ChanEvent*>::const_iterator itHigh = highEvents.begin();
	 itHigh != highEvents.end(); itHigh++) {
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
                geEnd = remove(geEvents.begin(), geEnd, *itHigh);
                geEnd = remove(geEvents.begin(), geEnd, *itLow);
            }
        }
    }
    // Now throw out any remaining clover low-gain events (for now)
    for ( vector<ChanEvent*>::const_iterator itLow = lowEvents.begin(); 
	  itLow != lowEvents.end(); itLow++ ) {
	geEnd = remove(geEvents.begin(), geEnd, *itLow);
    }

    // this purges the bad events for good from this processor which "remove"
    //   has moved to the end
    geEvents.erase(geEnd, geEvents.end());

    // Simple spectra (single crystals) -- NOTE WE CAN MAKE PERMANENT CHANGES TO EVENTS
    //   Necessary in order to set corrected time for use in other processors
    for (vector<ChanEvent*>::iterator it = geEvents.begin(); 
	 it != geEvents.end(); it++) {
        ChanEvent *chan = *it;
        
        double gEnergy = chan->GetCalEnergy();	
        double gTime   = chan->GetTime() - WalkCorrection(gEnergy);	
        chan->SetCorrectedTime(gTime);

        plot(D_ENERGY, gEnergy);
	if (hasImplant) {
	    plot(D_ENERGY_WITH_IMPLANT, gEnergy);
	}
	if (hasExt0) {
	    plot(D_ENERGY_WITH_EXT0, gEnergy);
	}
	if (hasExt1) {
	    plot(D_ENERGY_WITH_EXT1, gEnergy);
	}

        if (hasBeta) {
            plot(betaGated::D_ENERGY, gEnergy);

            int dtime=(int)(gTime - betaTime + 100);
            plot(betaGated::DD_TDIFF__GAMMA_ENERGY, dtime, gEnergy);
	    plot(betaGated::DD_TDIFF__BETA_ENERGY, dtime, betaEnergy);

            // individual beta gamma coinc spectra for each beta detector
            if (betaLoc == 0) {
                plot(betaGated::D_ENERGY_BETA0, gEnergy);
	    } else if (betaLoc == 1) {
                plot(betaGated::D_ENERGY_BETA1, gEnergy);
	    }
	    if (hasExt0) {
                plot(betaGated::D_ENERGY_WITH_EXT0, gEnergy);
	    }
	    if (hasExt1) {
		plot(betaGated::D_ENERGY_WITH_EXT1, gEnergy);
	    }
        }
        
        for (vector<ChanEvent*>::const_iterator it2 = it + 1;
                it2 != geEvents.end(); it2++) {
            ChanEvent *chan2 = *it2;

            double gEnergy2 = chan2->GetCalEnergy();            
	    // double gTime2 = chan2->GetTime() - WalkCorrection(gEnergy2);
	    //? perhaps do a time cut here

            symplot(DD_ENERGY, gEnergy, gEnergy2);
            if (hasBeta)
                symplot(betaGated::DD_ENERGY, gEnergy, gEnergy2);            
        } // iteration over other gammas
    } 
    
    // now we sort the germanium events according to their corrected time
    sort(geEvents.begin(), geEvents.end(), CompareCorrectedTime);

    // addbackEvents vector is arranged as:
    vector<CloverHit> addbackEvents[numClovers];
    // tas vector for total energy absorbed
    vector<CloverHit> tas;
    
    double subEventWindow = 10.0; // pixie units
    // guarantee the first event will be greater than the subevent window delayed from reference
    double refTime = -2.0 * subEventWindow; 

    // Do not take into account events with too low energy (avoid summing of noise with real gammas) 
    const double addbackEnergyCut = 25;
    
    for (vector<ChanEvent*>::iterator it = geEvents.begin(); it != geEvents.end(); it++) {
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
                addbackEvents[i].push_back(CloverHit());
            }
            tas.push_back(CloverHit());
        }
        // Total addback energy
        addbackEvents[clover].back().energy += energy;
        // We store latest time only
        addbackEvents[clover].back().time   = time;
        tas.back().energy += energy;
        tas.back().time   = time;
        refTime = time;
    }

    // Plot 'tas' spectra
    unsigned nTas = tas.size();
    for (unsigned i = 0; i < nTas; ++i) {
        double gEnergy = tas[i].energy;
        //double gTime = tas[i].second;

        if (gEnergy < 1) 
            continue;

        plot(D_ADD_ENERGY_TOTAL, gEnergy);
        if(hasBeta)
            plot(betaGated::D_ADD_ENERGY_TOTAL, gEnergy);
    }

    // Plot addback spectra 
    //   all vectors should have the same size
    for (unsigned int ev = 0; ev < tas.size(); ev++) {
	for (unsigned int det = 0; det < numClovers; ++det) {
            double gEnergy = addbackEvents[det][ev].energy;
            double gTime = addbackEvents[det][ev].time;
            if (gEnergy < 1) 
                continue;

            plot(D_ADD_ENERGY, gEnergy);
            plot(D_ADD_ENERGY_CLOVERX + det, gEnergy);

            if(hasBeta) {
                plot(betaGated::D_ADD_ENERGY, gEnergy);
                plot(betaGated::D_ADD_ENERGY_CLOVERX + det, gEnergy);
            }

        for (unsigned int det2 = det + 1; det2 < numClovers; ++det2) {
		double gEnergy2 = addbackEvents[det2][ev].energy;
		if (gEnergy2 < 1) 
		    continue;

		symplot(DD_ADD_ENERGY, gEnergy, gEnergy2);
        /*
         * Early and late coincidences
         * Only decay part of cycle can be taken
         * Early coin. are between decay cycle start and some arb. point
         * Late are between arb. point and end of cycle
         */
        if (hasDecay) {
            double decayCycleStart = 1.0;
            double decayCycleEarly = 1.5;
            double decayCycleEnd   = 2.0;
            double decayTime = event.GetCorrelator().GetDecayTime() * pixie::clockInSeconds;
            if (decayTime > decayCycleStart) {
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

    // Putting this here so we can later move it to a proper CORRELATED process function
    if (hasDecay) {
	double decayTime = event.GetCorrelator().GetDecayTime() * pixie::clockInSeconds;

	for (vector<ChanEvent*>::const_iterator it=geEvents.begin();
	     it != geEvents.end(); it++) {
	    double gEnergy = (*it)->GetCalEnergy();
	    plot(D_ENERGY_WITH_DECAY, gEnergy);
	    for (vector<ChanEvent*>::const_iterator it2 = it+1;
		 it2 != geEvents.end(); ++it2) {
		plot(DD_ENERGY_WITH_DECAY, gEnergy, (*it2)->GetCalEnergy());
	    }
	
	    granploty(DD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
	    if (hasBeta) {
		granploty(betaGated::DD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
	    }
	}

	// same for addback spectra
	for (unsigned int ev=0; ev < tas.size(); ev++) {
	    for (unsigned int det=0; det < numClovers; det++) {
		double gEnergy = addbackEvents[det][ev].energy;
	    
		if (gEnergy < 1)
		    continue;

		granploty(DD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
		if (hasBeta) {
		    granploty(betaGated::DD_ENERGY__TIMEX, gEnergy, decayTime, timeResolution);
		}
	    }
	}
    }

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
