/** \file GeProcessor.cpp
 *
 * implementation for germanium processor
 * David Miller, August 2009
 */

//? Make a clover specific processor

#include <algorithm>
#include <fstream>
#include <iostream>

#include <climits>
#include <cmath>
#include <cstdlib>

#include "damm_plotids.h"

#include "Correlator.h"
#include "DetectorDriver.h"
#include "DetectorLibrary.hpp"
#include "GeProcessor.h"
#include "RawEvent.h"

using namespace std;
using std::remove;

// useful function for symmetrically incrementing 2D plots
void symplot(int dammID, double bin1, double bin2)
{
    plot(dammID, bin1, bin2);
    plot(dammID, bin2, bin1);
}

GeProcessor::GeProcessor() : EventProcessor()
{
    name = "ge";
    associatedTypes.insert("ge"); // associate with germanium detectors

    numClovers = 0;
    minCloverLoc = INT_MAX;
}

/** Initialize processor and determine number of clovers 
 * from global Identifiers
 */
bool GeProcessor::Init(DetectorDriver &driver)
{
    if (!EventProcessor::Init(driver))
	return false;

    /* clover specific routine, determine the number of clover detector
       channels and divide by four to find the total number of clovers
    */
    extern DetectorLibrary modChan;
    unsigned int cloverChans = 0;
    int maxCloverLoc = INT_MIN;

    for ( DetectorLibrary::const_iterator it = modChan.begin();
	  it != modChan.end(); it++) {
	if (it->GetSubtype() == "clover_high") {
	    cloverChans++;
	    if (it->GetLocation() < minCloverLoc)
		minCloverLoc = it->GetLocation();
	    if (it->GetLocation() > maxCloverLoc)
		maxCloverLoc = it->GetLocation();
	}
    }
    size_t spread;
    if (cloverChans == 0) {
	spread = 0;
	numClovers = 0;
    } else {
	spread = maxCloverLoc - minCloverLoc + 1;
	
	if (cloverChans % chansPerClover != 0 || 
	    spread % chansPerClover != 0 ) {
	    
	    cout << " There does not appear to be the proper number of"
		 << " channels per clover.\n Program terminating." << endl;
	    exit(EXIT_FAILURE);
	}
    
	if (cloverChans != 0) {
	    numClovers = spread / chansPerClover;
	    //print statement
	    cout << "A total of " << cloverChans << " clover channels (" 
		 << minCloverLoc << " - " << maxCloverLoc << ") for "
		 << numClovers << " clovers were detected." << endl;    
	    ClearClovers();
	}
    }

    if (numClovers > dammIds::ge::MAX_CLOVERS) {
	cout << "This is greater than MAX_CLOVERS for spectra definition."
	     << "  Check the spectrum definition file and try again." << endl;
	exit(EXIT_FAILURE);
    }
    
    return true;
}

void GeProcessor::ClearClovers(void)
{
    cloverEnergy.assign(numClovers, 0);
    cloverMultiplicity.assign(numClovers, 0);
}

/** Declare plots including many for decay/implant/neutron gated analysis  */
void GeProcessor::DeclarePlots(void) const
{
    const int energyBins1 = SE;
    const int energyBins2 = SB;
    const int timeBins1   = SC;
    const int timeBins2   = SA;
    const int granTimeBins = SA;

    {
	using namespace dammIds::ge;
	DeclareHistogram1D(D_CLOVER_ENERGY_ALL, energyBins1, "Ge clovers");
	
	DeclareHistogram1D(D_ENERGY, energyBins1, "GE singles cal");
	DeclareHistogram1D(D_ENERGY_LOWGAIN, energyBins1, "GE LG singles cal");
	DeclareHistogram1D(D_ENERGY_HIGHGAIN, energyBins1, "GE HG singles cal");
	DeclareHistogram1D(D_ENERGY_NTOF1, energyBins1, "tof1 sig1 gate GE singles cal");
	DeclareHistogram1D(D_ENERGY_NTOF2, energyBins1, "tof1 sig2 gate GE singles cal");
	DeclareHistogram1D(D_ENERGY_HEN3, energyBins1, "3Hen gate GE singles cal");
	DeclareHistogram1D(D_MULT, S6, "gamma multiplicity");
	DeclareHistogram1D(D_ADD_ENERGY, energyBins1, "total clover add back");
	
	for (unsigned int i=0; i < MAX_CLOVERS; i++) {
	    DeclareHistogram1D(D_CLOVER_ENERGY_DETX + i, energyBins1, "Ge clovers");
	    DeclareHistogram1D(D_ADD_ENERGY_DETX + i, energyBins1, "individual clover add back");
	    DeclareHistogram1D(betaGated::D_ADD_ENERGY_DETX + i, energyBins1, "beta gated clover addback");
	    DeclareHistogram1D(decayGated::D_ADD_ENERGY_DETX + i, energyBins1, "corr beta gated addback");
	}

	DeclareHistogram2D(dammIds::ge::DD_CLOVER_ENERGY_RATIO,
			   S4, S6, "high/low energy ratio (x10)");
    } // dammIds::ge
    {
	using namespace dammIds::ge::betaGated;
	DeclareHistogram1D(D_ENERGY, energyBins1, "beta gate GE singles cal");
	DeclareHistogram1D(multiplicityGated::D_ENERGY, energyBins1, "mult1 beta gated GE singles cal");
	DeclareHistogram1D(D_TDIFF, timeBins1, "beta-gamma time");
	DeclareHistogram1D(D_ENERGY_BETA0, energyBins1, "beta1 gate GE singles cal");
	DeclareHistogram1D(D_ENERGY_BETA1, energyBins1, "beta2 gate GE singles cal");
	DeclareHistogram1D(D_ENERGY_NTOF1, energyBins1, "tof1 sig 1 gate GE singles cal");
	DeclareHistogram1D(D_ENERGY_NTOF2, energyBins1, "tof1 sig 2 gate GE singles cal");
	DeclareHistogram1D(D_ENERGY_HEN3, energyBins1, "3Hen gate GE singles cal");
	DeclareHistogram1D(D_ADD_ENERGY, energyBins1, "beta gated total clover add back");

	DeclareHistogram2D(DD_TDIFF__GAMMA_ENERGY, timeBins2, energyBins2, 
			   "beta gamma time vs gamma energy", 2);
	DeclareHistogram2D(multiplicityGated::DD_TDIFF__GAMMA_ENERGY, timeBins2, energyBins2,
			   "mult1 beta gamma time vs gamma energy", 2);
	DeclareHistogram2D(DD_TDIFF__BETA_ENERGY, timeBins2, energyBins2,
			   "beta gamma time vs beta energy", 2);
    } // using dammIds::ge::betaGated
    {
	using namespace dammIds::ge::decayGated;
	DeclareHistogram1D(D_ENERGY, energyBins1, "corr beta geta GE singles cal");	
	DeclareHistogram1D(D_ADD_ENERGY, energyBins1, "corr beta total clover add back");

	DeclareHistogram2D(withBeta::DD_ENERGY__DECAY_TIME_GRANX + 0,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (1us/ch)(xkeV)", 2);
	DeclareHistogram2D(withBeta::DD_ENERGY__DECAY_TIME_GRANX + 1,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (10us/ch)(xkeV)", 2);
	DeclareHistogram2D(withBeta::DD_ENERGY__DECAY_TIME_GRANX + 2,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (100us/ch)(xkeV)", 2);
	DeclareHistogram2D(withBeta::DD_ENERGY__DECAY_TIME_GRANX + 3,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (1ms/ch)(xkeV)", 2);
	DeclareHistogram2D(withBeta::DD_ENERGY__DECAY_TIME_GRANX + 4,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (10ms/ch)(xkeV)", 2);
	DeclareHistogram2D(withBeta::DD_ENERGY__DECAY_TIME_GRANX + 5,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (100ms/ch)(xkeV)", 2);

	DeclareHistogram2D(withBeta::multiplicityGated::DD_ENERGY__DECAY_TIME_GRANX + 0,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (1us/ch)(xkeV)", 2);
	DeclareHistogram2D(withBeta::multiplicityGated::DD_ENERGY__DECAY_TIME_GRANX + 1,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (10us/ch)(xkeV)", 2);
	DeclareHistogram2D(withBeta::multiplicityGated::DD_ENERGY__DECAY_TIME_GRANX + 2,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (100us/ch)(xkeV)", 2);
	DeclareHistogram2D(withBeta::multiplicityGated::DD_ENERGY__DECAY_TIME_GRANX + 3,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (1ms/ch)(xkeV)", 2);
	DeclareHistogram2D(withBeta::multiplicityGated::DD_ENERGY__DECAY_TIME_GRANX + 4,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (10ms/ch)(xkeV)", 2);
	DeclareHistogram2D(withBeta::multiplicityGated::DD_ENERGY__DECAY_TIME_GRANX + 5,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (100ms/ch)(xkeV)", 2);

	DeclareHistogram2D(withoutBeta::DD_ENERGY__DECAY_TIME_GRANX + 0,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (1us/ch)(xkeV)");
	DeclareHistogram2D(withoutBeta::DD_ENERGY__DECAY_TIME_GRANX + 1,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (10us/ch)(xkeV)");
	DeclareHistogram2D(withoutBeta::DD_ENERGY__DECAY_TIME_GRANX + 2,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (100us/ch)(xkeV)");
	DeclareHistogram2D(withoutBeta::DD_ENERGY__DECAY_TIME_GRANX + 3,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (1ms/ch)(xkeV)");
	DeclareHistogram2D(withoutBeta::DD_ENERGY__DECAY_TIME_GRANX + 4,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (10ms/ch)(xkeV)");
	DeclareHistogram2D(withoutBeta::DD_ENERGY__DECAY_TIME_GRANX + 5,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (100ms/ch)(xkeV)");

    } // decayGated
    {
	using namespace dammIds::ge::implantGated;
	DeclareHistogram1D(D_ENERGY, energyBins1, "implant gate GE singles cal");
	DeclareHistogram2D(DD_ENERGY__TDIFF, energyBins2, granTimeBins, 
			   "implant time v. energy (10 ms/ch) (keV");
    }
    {
	using namespace dammIds::ge::matrix;
	DeclareHistogram1D(D_TDIFF, timeBins1, "gamma-gamma time");
	DeclareHistogram2D(DD_ENERGY_PROMPT, energyBins2, energyBins2, 
			   "gamma-gamma");
	DeclareHistogram2D(DD_ADD_ENERGY_PROMPT, energyBins2, energyBins2, 
			   "addback gamma-gamma");
    }
    {
	using namespace dammIds::ge::matrix::betaGated;
	DeclareHistogram2D(DD_ENERGY_PROMPT, energyBins2, energyBins2,
			   "beta gated gamma-gamma");
	DeclareHistogram2D(DD_ADD_ENERGY_PROMPT, energyBins2, energyBins2,
			   "beta gated addback gamma-gamma");
    }
    {
	using namespace dammIds::ge::matrix::decayGated;
	DeclareHistogram2D(DD_ENERGY_PROMPT, energyBins2, energyBins2, 
			   "correlated beta gamma-gamma");
	DeclareHistogram2D(DD_ADD_ENERGY_PROMPT, energyBins2, energyBins2, 
			   "correlated beta addback gamma-gamma");
    }      
}

/** process the event */
bool GeProcessor::Process(RawEvent &event)
{
    using namespace dammIds;

    if (!EventProcessor::Process(event))
	return false;
    
    // makes a copy so we can remove (or rearrange) bad events 
    //   based on poorly matched high-low gain energies
    vector<ChanEvent*> geEvents = sumMap["ge"]->GetList();
    vector<ChanEvent*>::iterator geEnd = geEvents.end();

    static const DetectorSummary *scintSummary = event.GetSummary("scint");
    
    bool hasDecay = 
	(event.GetCorrelator().GetCondition() == Correlator::VALID_DECAY);
    bool hasImplant =
	(event.GetCorrelator().GetCondition() == Correlator::VALID_IMPLANT);
    double betaEnergy = NAN;
    double betaTime = NAN;
    double betaLoc = NAN;
    int ntof1 = 0;
    bool hen3 = false;

    // find all beta and neutron signals for ge gating
    if (scintSummary) {	
	for (vector<ChanEvent*>::const_iterator it = scintSummary->GetList().begin();
	     it != scintSummary->GetList().end(); it++) {
	    ChanEvent *chan = *it;
	    string subtype = chan->GetChanID().GetSubtype();
	    
	    if (subtype == "beta") {
		betaEnergy = chan->GetCalEnergy();
		betaLoc    = chan->GetChanID().GetLocation();
		betaTime   = chan->GetTime();
	    } else if (subtype == "neutr"){
		int neutronLoc = chan->GetChanID().GetLocation();  
		// some magic channel numbers here
		if(neutronLoc == 2) ntof1 = 1;
		if(neutronLoc == 3) ntof1 = 2;
		if(neutronLoc == 4)
		    hen3 = true;
	    }
	}
    } // scint summary present

    // reject events for clover where raw high gain energy does not
    //   correspond properly to raw low gain energy
    const double lowRatio = 1.5, highRatio = 3.0;
    static const vector<ChanEvent*> &highEvents = event.GetSummary("ge:clover_high", true)->GetList();
    static const vector<ChanEvent*> &lowEvents  = event.GetSummary("ge:clover_low", true)->GetList();

    // first plot low gain energies
    for (vector<ChanEvent*>::const_iterator itLow  = lowEvents.begin();
	 itLow != lowEvents.end(); itLow++) {
	plot(ge::D_ENERGY_LOWGAIN, (*itLow)->GetCalEnergy());
    }
    for (vector<ChanEvent*>::const_iterator itHigh = highEvents.begin();
	 itHigh != highEvents.end(); itHigh++) {
	// find the matching low gain event
	int location = (*itHigh)->GetChanID().GetLocation();
	plot(ge::D_ENERGY_HIGHGAIN, (*itHigh)->GetCalEnergy());
	vector <ChanEvent*>::const_iterator itLow = lowEvents.begin();
	for (; itLow != lowEvents.end(); itLow++) {
	    if ( (*itLow)->GetChanID().GetLocation() == location ) {
		break;
	    }
	}
	if ( itLow != lowEvents.end() ) {
	    double ratio = (*itHigh)->GetEnergy() / (*itLow)->GetEnergy();
	    plot(ge::DD_CLOVER_ENERGY_RATIO, location, ratio * 10.);
	    if (ratio < lowRatio || ratio > highRatio) {
		// put these bad events at the end of the vector
		geEnd = remove(geEvents.begin(), geEnd, *itHigh);
		geEnd = remove(geEvents.begin(), geEnd, *itLow);
	    }
	}
    }
    // this purges the bad events for good from this processor which "remove"
    //   has moved to the end
    geEvents.erase(geEnd, geEvents.end());

    // ge multiplicity spectrum
    plot(ge::D_MULT,geEvents.size());
        
    // determine clover multiplicity
    ClearClovers();
    for (vector<ChanEvent*>::const_iterator it = geEvents.begin();
	 it != geEvents.end(); it++) {
	ConstructAddback(*it);
    }

    // create Ge spectrum with and without various gates
    for (vector<ChanEvent*>::const_iterator it = geEvents.begin(); 
	 it != geEvents.end(); it++) {
	ChanEvent *chan = *it;
	
	// For some experiments two outputs per clover were used, one was set
	// to high gain on pixie16 and one was set to low gain on pixie16. 
	// Only perform analysis for high gain clover signals. 
	string subtype = chan->GetChanID().GetSubtype();
	if (subtype == "clover_low")
	    continue;

	double gEnergy = chan->GetCalEnergy();
	double gTime= chan->GetTime();	
	unsigned int detNum = GetCloverNum(chan->GetChanID().GetLocation());

	plot(ge::D_ENERGY, gEnergy);

	if (subtype == "clover_high") {
            //plot a singles total spectrum and inidividual clover spectra
	    plot(ge::D_CLOVER_ENERGY_DETX + detNum, gEnergy);
	    plot(ge::D_CLOVER_ENERGY_ALL, gEnergy);
	}
	
	if(betaEnergy > 0){ // beta gamma coincidence
	    using namespace dammIds::ge::betaGated;
	    int dtime=(int)(gTime - betaTime + 100);
	    
	    plot(D_ENERGY, gEnergy);
	    plot(D_TDIFF, dtime);
	    plot(DD_TDIFF__GAMMA_ENERGY, dtime, gEnergy);  
	    plot(DD_TDIFF__BETA_ENERGY, dtime, betaEnergy);  
	    
	    // individual beta gamma coinc spectra for each beta detector
	    if (betaLoc == 0)
		plot(D_ENERGY_BETA0, gEnergy);
	    if (betaLoc == 1)
		plot(D_ENERGY_BETA1, gEnergy);
	    //beta-neutron gated ge spectra
	    if (ntof1 == 1) 
		plot(D_ENERGY_NTOF1, gEnergy);
	    if (ntof1 == 2) 
		plot(D_ENERGY_NTOF2, gEnergy);
	    if (hen3) 
		plot(D_ENERGY_HEN3, gEnergy);
	}
	
	//plot beta gated spectra, 1501 - clover total, 505-510 correlation
	//  time vs gamma energy
	// implant - beta correlation
	if (hasDecay) {
	    using namespace dammIds::ge::decayGated;
	    
	    const int numGranularities = 6;
	    const double timeResolution[numGranularities] =
		{1e-6, 10e-6, 100e-6, 1e-3, 10e-3, 100e-3}; // in seconds/bin
	    double decayTime = event.GetCorrelator().GetDecayTime() * pixie::clockInSeconds;
	    plot(D_ENERGY, gEnergy);
	    
	    if (betaEnergy > 0) { // added for LeRIBSS
		for (int i = 0; i < numGranularities; i++) {
		    double timeBin = decayTime / timeResolution[i];
		    plot(withBeta::DD_ENERGY__DECAY_TIME_GRANX + i, gEnergy, timeBin);
		    if (cloverMultiplicity.at(detNum) == 1) {
			plot (withBeta::multiplicityGated::DD_ENERGY__DECAY_TIME_GRANX + i,
			      gEnergy, timeBin);
		    }
		}
	    } else {
		for (int i = 0; i < numGranularities; i++) {
		    double timeBin = decayTime / timeResolution[i];
		    plot(withoutBeta::DD_ENERGY__DECAY_TIME_GRANX + i, gEnergy, timeBin);
		}
	    }
	} // decay gate
	
	//neutron gated Ge singles spectra
	if (ntof1 == 1)
	    plot(ge::D_ENERGY_NTOF1,gEnergy);
	if (ntof1 == 2)
	    plot(ge::D_ENERGY_NTOF2,gEnergy);
	if (hen3 == 1) 
	    plot(ge::D_ENERGY_HEN3,gEnergy);
	
	// implant gated ge spectrum
	if (hasImplant) {
	    plot(ge::implantGated::D_ENERGY, gEnergy);
	    double timeBin = (gTime - event.GetCorrelator().GetImplantTime()) * pixie::clockInSeconds / 10e-3;
	    plot(ge::implantGated::DD_ENERGY__TDIFF, gEnergy, timeBin);
	}
	
	//end singles plotting
	/*************************************/
	
	//perform simple gamma-gamma matrix
	/*************************************/
	
	for (vector<ChanEvent*>::const_iterator it2 = it+1;
	     it2 != geEvents.end(); it2++) {
	    ChanEvent *chan2 = *it2;
	    if ( chan2->GetChanID().GetSubtype() == "clover_low" )
		continue;
	    double gEnergy2 = chan2->GetCalEnergy();
	    double gTime2 = chan2->GetTime();
	    
	    plot(ge::matrix::D_TDIFF, fabs(gTime2 - gTime));
	    
	    //if((det2_time - det1_time) < 5){ // 50 ns gamma-gamma gate
	    // increment gamma-gamma matrix symmetrically
	    symplot(ge::matrix::DD_ENERGY_PROMPT, gEnergy, gEnergy2);
	    
	    if (betaEnergy > 0) {
		symplot(ge::matrix::betaGated::DD_ENERGY_PROMPT, gEnergy, gEnergy2);
	    }	    
	    //correlated beta gated gamma-gamma
	    if (hasDecay) {
		using namespace ge::matrix::decayGated;
		symplot(ge::matrix::decayGated::DD_ENERGY_PROMPT, gEnergy, gEnergy2);
	    } //end beta gated gammma-gamma	      
	} // end loop over all other detectors	 
    } // end loop over Ge channels

    /**********************************/
    //plot addback spectra, 
    
    for (unsigned int det1 = 0; det1 < numClovers; det1++) {
	if (cloverEnergy[det1] <= 0) 
	    continue;
	// all clover addback
	plot(ge::D_ADD_ENERGY, cloverEnergy[det1]);
	plot(ge::D_ADD_ENERGY_DETX + det1, cloverEnergy[det1]);

	// beta gated all clover addback
	if (betaEnergy > 0){
	    using namespace ge::betaGated;
	    plot(D_ADD_ENERGY, cloverEnergy[det1]);
	    plot(D_ADD_ENERGY_DETX + det1, cloverEnergy[det1]);
	}
	    
	// correlated beta all clover addback
	if (hasDecay) {
	    using namespace ge::decayGated;
	    plot(D_ADD_ENERGY, cloverEnergy[det1]);
	    plot(D_ADD_ENERGY_DETX + det1, cloverEnergy[det1]);
	}
	  
	for (unsigned int det2 = det1+1; det2 < cloverEnergy.size(); det2++) {	      
	    if (cloverEnergy[det2] <= 0) 
		continue;
	    // addback gamma-gamma
	    // symmetrically increment gamma-gamma matrix
	    symplot(ge::matrix::DD_ADD_ENERGY_PROMPT, 
		    cloverEnergy[det1], cloverEnergy[det2]);
	  
	    // beta-gated addback gamma-gamma 
	    if (betaEnergy > 0) {
		symplot(ge::matrix::betaGated::DD_ADD_ENERGY_PROMPT,
			cloverEnergy[det1], cloverEnergy[det2]);
	    } //end beta gated gammma-gamma
		    
	    //correlated beta gated addback gamma-gamma
	    if (hasDecay) {
		symplot(ge::matrix::decayGated::DD_ADD_ENERGY_PROMPT,
			cloverEnergy[det1], cloverEnergy[det2]);
	    } //end beta gated gammma-gamma
	} // end loop over second clover      
    }

    // end plotting addback spectra
    /*********************************/
    
    EndProcess(); // update the processing time
    return true;
}

/**
 * Simple clover addback, routine is not fully complete
 *   Clover channels are grouped based on their location starting from the lowest number
 *   (i.e. 0-3 is one detector and so forth)
 */
void GeProcessor::ConstructAddback(const ChanEvent *ch)
{
    // simple clover addback routine for now
    string subtype = ch->GetChanID().GetSubtype();

    if (subtype == "clover_high") {
	double energy = ch->GetCalEnergy();
	unsigned int detnum = GetCloverNum(ch->GetChanID().GetLocation());

	cloverMultiplicity.at(detnum)++;
	cloverEnergy.at(detnum) += energy;
    } else if (subtype == "sega") {
	cout << " Add back not implemented for sega detectors yet " << endl;
    }
}
