/** \file GeProcessor.cpp
 *
 * implementation for germanium processor
 * David Miller, August 2009
 */

//? Make a clover specific processor

#include <fstream>
#include <iostream>

#include <cstdlib>

#include "damm_plotids.h"

#include "Correlator.h"
#include "DetectorDriver.h"
#include "GeProcessor.h"
#include "RawEvent.h"

using namespace std;

GeProcessor::GeProcessor() : EventProcessor()
{
    name = "ge";
    associatedTypes.insert("ge"); // associate with germanium detectors
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
    extern vector<Identifier> modChan;
    unsigned int cloverChans = 0;
    
    for ( vector<Identifier>::const_iterator it = modChan.begin();
	  it != modChan.end(); it++) {
	if ((*it).GetSubtype() == "clover_high")
	    cloverChans++;
    }

    if (cloverChans % chansPerClover != 0) {
	cout << " There does not appear to be the proper number of channels "
	     << " per clover.\n Program terminating." << endl;
	exit(EXIT_FAILURE);
    }
    
    unsigned int clovers = cloverChans / chansPerClover;
    //initialize vector to contain energy for each gamma detector
    cloverEnergy.assign(clovers, 0);

    //print statement
    cout << "A total of " << cloverChans << " clover channels (" 
	 << clovers << " clovers) were detected " << endl;    

    if (clovers > dammIds::ge::MAX_CLOVERS) {
	cout << "This is greater than MAX_CLOVERS for spectra definition."
	     << "  Check the spectrum definition file and try again." << endl;
	exit(EXIT_FAILURE);
    }
    
    return true;
}

/** Declare plots including many for decay/implant/neutron gated analysis  */
void GeProcessor::DeclarePlots(void) const
{
    const int energyBins1 = SE;
    const int energyBins2 = SB;
    const int timeBins1   = SC;
    const int timeBins2   = SA;
    const int granTimeBins = S8;

    {
	using namespace dammIds::ge;
	DeclareHistogram1D(D_CLOVER_ENERGY_ALL, energyBins1, "Ge clovers");
	
	DeclareHistogram1D(D_ENERGY, energyBins1, "GE singles cal");
	DeclareHistogram1D(D_ENERGY_NTOF1, energyBins1, "tof1 sig1 gate GE singles cal");
	DeclareHistogram1D(D_ENERGY_NTOF2, energyBins1, "tof1 sig2 gate GE singles cal");
	DeclareHistogram1D(D_ENERGY_HEN3, energyBins1, "3Hen gate GE singles cal");
	DeclareHistogram1D(D_MULT, S6, "beta gate GE singles cal");
	DeclareHistogram1D(D_ADD_ENERGY, energyBins1, "total clover add back");
	
	for (unsigned int i=0; i < MAX_CLOVERS; i++) {
	    DeclareHistogram1D(D_CLOVER_ENERGY_DETX + i, energyBins1, "Ge clovers");
	    DeclareHistogram1D(D_ADD_ENERGY_DETX + i, energyBins1, "individual clover add back");
	    DeclareHistogram1D(betaGated::D_ADD_ENERGY_DETX + i, energyBins1, "beta gated clover add back");
	    DeclareHistogram1D(decayGated::D_ADD_ENERGY_DETX + i, energyBins1, "corr beta gated addback");
	}
    } // dammIds::ge
    {
	using namespace dammIds::ge::betaGated;
	DeclareHistogram1D(D_ENERGY, energyBins1, "beta gate GE singles cal");
	DeclareHistogram1D(D_TDIFF, timeBins1, "beta-gamma time");
	DeclareHistogram1D(D_ENERGY_BETA0, energyBins1, "beta1 gate GE singles cal");
	DeclareHistogram1D(D_ENERGY_BETA1, energyBins1, "beta2 gate GE singles cal");
	DeclareHistogram1D(D_ENERGY_NTOF1, energyBins1, "tof1 sig 1 gate GE singles cal");
	DeclareHistogram1D(D_ENERGY_NTOF2, energyBins1, "tof1 sig 2 gate GE singles cal");
	DeclareHistogram1D(D_ENERGY_HEN3, energyBins1, "3Hen gate GE singles cal");
	DeclareHistogram1D(D_ADD_ENERGY, energyBins1, "beta gated total clover add back");

	DeclareHistogram2D(DD_TDIFF__GAMMA_ENERGY, timeBins2, energyBins2, 
			   "beta gamma time vs gamma energy", 2, timeBins2, 0, 0, energyBins2, 0, 0);
	DeclareHistogram2D(DD_TDIFF__BETA_ENERGY, timeBins2, energyBins2,
			   "beta gamma time vs beta energy", 2, timeBins2, 0, 0, energyBins2, 0, 0);
    } // using dammIds::ge::betaGated
    {
	using namespace dammIds::ge::decayGated;
	DeclareHistogram1D(D_ENERGY, energyBins1, "corr beta geta GE singles cal");	
	DeclareHistogram1D(D_ADD_ENERGY, energyBins1, "corr beta total clover add back");

	DeclareHistogram2D(DD_ENERGY__DECAY_TIME_GRANX + 0,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (1us/ch)(xkeV)", 2,
			   energyBins2, 0, 0, granTimeBins, 0, 0);
	DeclareHistogram2D(DD_ENERGY__DECAY_TIME_GRANX + 1,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (10us/ch)(xkeV)", 2,
			   energyBins2, 0, 0, granTimeBins, 0, 0);
	DeclareHistogram2D(DD_ENERGY__DECAY_TIME_GRANX + 2,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (100us/ch)(xkeV)", 2,
			   energyBins2, 0, 0, granTimeBins, 0, 0);
	DeclareHistogram2D(DD_ENERGY__DECAY_TIME_GRANX + 3,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (1ms/ch)(xkeV)", 2,
			   energyBins2, 0, 0, granTimeBins, 0, 0);
	DeclareHistogram2D(DD_ENERGY__DECAY_TIME_GRANX + 4,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (10ms/ch)(xkeV)", 2,
			   energyBins2, 0, 0, granTimeBins, 0, 0);
	DeclareHistogram2D(DD_ENERGY__DECAY_TIME_GRANX + 5,
			   energyBins2, granTimeBins, "DSSD Ty,Ex (100ms/ch)(xkeV)", 2,
			   energyBins2, 0, 0, granTimeBins, 0, 0);
    } // decayGated
    {
	using namespace dammIds::ge::implantGated;
	DeclareHistogram1D(D_ENERGY, energyBins1, "implant gate GE singles cal");
    }
    {
	using namespace dammIds::ge::matrix;
	DeclareHistogram1D(D_TDIFF, timeBins1, "gamma-gamma time");
	DeclareHistogram2D(DD_ENERGY_PROMPT, energyBins2, energyBins2, "gamma-gamma", 2,			   
			   energyBins2, 0, 0, energyBins2, 0, 0);
	DeclareHistogram2D(DD_ADD_ENERGY_PROMPT, energyBins2, energyBins2, "addback gamma-gamma",
			   2, energyBins2, 0, 0, energyBins2, 0, 0);
    }
    {
	using namespace dammIds::ge::matrix::betaGated;
	DeclareHistogram2D(DD_ENERGY_PROMPT, energyBins2, energyBins2, "beta gated gamma-gamma",
			   2, energyBins2, 0, 0, energyBins2, 0, 0);
	DeclareHistogram2D(DD_ADD_ENERGY_PROMPT, energyBins2, energyBins2, "beta gated addback gamma-gamma",
			   2, energyBins2, 0, 0, energyBins2, 0, 0);
    }
    {
	using namespace dammIds::ge::matrix::decayGated;
	DeclareHistogram2D(DD_ENERGY_PROMPT, energyBins2, energyBins2, "correlated beta gamma-gamma",
			   2, energyBins2, 0, 0, energyBins2, 0, 0);
	DeclareHistogram2D(DD_ADD_ENERGY_PROMPT, energyBins2, energyBins2, "correlated beta addback gamma-gamma",
			   2, energyBins2, 0, 0, energyBins2, 0, 0);
    }      
}

/** process the event */
bool GeProcessor::Process(RawEvent &event)
{
    using namespace dammIds;

    if (!EventProcessor::Process(event))
	return false;
    
    static const vector<ChanEvent*>& geEvents = sumMap["ge"]->GetList();
    static const DetectorSummary *scintSummary = event.GetSummary("scint");
    
    bool hasDecay = 
	(event.GetCorrelator().GetCondition() == Correlator::VALID_DECAY);
    bool hasImplant =
	(event.GetCorrelator().GetCondition() == Correlator::VALID_IMPLANT);
    double betaEnergy = -1;
    double betaTime = -1;
    double betaLoc = -1;
    int ntof1 = 0;
    int hen3 = 0;

    cloverEnergy.assign(cloverEnergy.size(), 0);

    // find all beta and neutron signals for ge gating
    if (scintSummary) {	
	for (vector<ChanEvent*>::const_iterator it = scintSummary->GetList().begin();
	     it != scintSummary->GetList().end(); it++) {
	    ChanEvent *chan = *it;
	    string subtype = chan->GetChanID().GetSubtype();
	    
	    if (subtype == "beta") {
		betaEnergy = chan->GetCalEnergy();
		betaLoc = chan->GetChanID().GetLocation();
		betaTime   = chan->GetTime();
	    } else if (subtype == "neutr"){
		int neutronLoc = chan->GetChanID().GetLocation();  
		// some magic channel numbers here
		if(neutronLoc == 2) ntof1 = 1;
		if(neutronLoc == 3) ntof1 = 2;
		if(neutronLoc == 4) hen3 = 1;
	    }
	}
    } // scint summary present

    // ge multiplicity spectrum
    plot(ge::D_MULT,geEvents.size());
    
    // creat Ge spectrum with and without various gates
    for (vector<ChanEvent*>::const_iterator it = geEvents.begin(); 
	 it != geEvents.end(); it++) {
	ChanEvent *chan = *it;
	
	string subtype = chan->GetChanID().GetSubtype();
	
	// For some experiments two outputs per clover were used, one was set
	// to high gain on pixie16 and one was set to low gain on pixie16. 
	// Only perform analysis for high gain clover signals. 

	double calEnergy = chan->GetCalEnergy();
	int detLoc = chan->GetChanID().GetLocation();
	double gammaTime= chan->GetTime();	

	plot(ge::D_ENERGY, calEnergy);

	if (subtype == "clover_high") {
	    unsigned int detNum = detLoc / chansPerClover;	    
            //plot a singles total spectrum and inidividual clover spectra
	    plot(ge::D_CLOVER_ENERGY_DETX + detNum, calEnergy);
	    plot(ge::D_CLOVER_ENERGY_ALL, calEnergy);
	}
	
	if(betaEnergy > 0){ // beta gamma coincidence
	    using namespace dammIds::ge::betaGated;
	    int dtime=(int)(gammaTime - betaTime + 100);
	    
	    plot(D_ENERGY, calEnergy);
	    plot(D_TDIFF, dtime);
	    plot(DD_TDIFF__GAMMA_ENERGY, dtime, calEnergy);  
	    plot(DD_TDIFF__BETA_ENERGY, dtime, betaEnergy);  
	    
	    // individual beta gamma coinc spectra for each beta detector
	    if(betaLoc == 0)
		plot(D_ENERGY_BETA0, calEnergy);
	    if(betaLoc == 1)
		plot(D_ENERGY_BETA1, calEnergy);
	    //beta-neutron gated ge spectra
	    if(ntof1 == 1) 
		plot(D_ENERGY_NTOF1, calEnergy);
	    if(ntof1 == 2) 
		plot(D_ENERGY_NTOF2, calEnergy);
	    if(hen3 == 1) 
		plot(D_ENERGY_HEN3, calEnergy);
	}
	
	//plot beta gated spectra, 1501 - clover total, 505-510 correlation
	//  time vs gamma energy
	// implant - beta correlation
	if (hasDecay) {
	    using namespace dammIds::ge::decayGated;
	    
	    const int numGranularities = 6;
	    const double timeResolution[numGranularities] =
		{1e-6, 10e-6, 100e-6, 1e-3, 10e-3, 100e-3}; // in seconds/bin
	    
	    plot(D_ENERGY, calEnergy);
	    for (int i = 0; i < numGranularities; i++) {
		int timeBin = int(event.GetCorrelator().GetDecayTime()
				  * pixie::clockInSeconds / timeResolution[i]);
		plot(DD_ENERGY__DECAY_TIME_GRANX + i, calEnergy, timeBin);
	    }
	} // decay gate
	
	//neutron gated Ge singles spectra
	if (ntof1 == 1)
	    plot(ge::D_ENERGY_NTOF1,calEnergy);
	if (ntof1 == 2)
	    plot(ge::D_ENERGY_NTOF2,calEnergy);
	if (hen3 == 1) 
	    plot(ge::D_ENERGY_HEN3,calEnergy);
	
	// implant gated ge spectrum
	if (hasImplant) {
	    plot(ge::implantGated::D_ENERGY, calEnergy);
	}
	
	//end singles plotting
	/*************************************/
	
	ConstructAddback(calEnergy, detLoc, subtype);
	
	//perform simple gamma-gamma matrix
	/*************************************/
	
	for (vector<ChanEvent*>::const_iterator it2 = it;
	     it2 != geEvents.end(); it2++) {
	    ChanEvent *chan2 = *it;
	    double calEnergy2 = chan2->GetCalEnergy();
	    double gammaTime2 = chan2->GetTime();
	    
	    plot(ge::matrix::D_TDIFF, fabs(gammaTime2 - gammaTime));
	    
	    //if((det2_time - det1_time) < 5){ // 50 ns gamma-gamma gate
	    // increment gamma-gamma matrix symmetrically
	    plot(ge::matrix::DD_ENERGY_PROMPT, calEnergy, calEnergy2);
	    plot(ge::matrix::DD_ENERGY_PROMPT, calEnergy2, calEnergy);
	    
	    //crg beta-gated gamma-gamma
	    if (betaEnergy > 0) {
		using namespace ge::matrix::betaGated;
		plot(DD_ENERGY_PROMPT, calEnergy, calEnergy2);
		plot(DD_ENERGY_PROMPT, calEnergy2, calEnergy);
	    } //end beta gated gammma-gamma
	    
	      //correlated beta gated gamma-gamma
	    if (hasDecay) {
		using namespace ge::matrix::decayGated;
		plot(DD_ENERGY_PROMPT, calEnergy, calEnergy2);
		plot(DD_ENERGY_PROMPT, calEnergy2, calEnergy);
	    } //end beta gated gammma-gamma	      
	} // end loop over all other detectors	 
    } // end loop over Ge channels

    /**********************************/
    //plot addback spectra, 
    
    for (unsigned int det1 = 0; det1 < cloverEnergy.size(); det1++) {
	if (cloverEnergy[det1] > 0) {	  
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
		if(cloverEnergy[det2] > 0){
		    // addback gamma-gamma
		    // symmetrically increment gamma-gamma matrix
		    plot(ge::matrix::DD_ADD_ENERGY_PROMPT, 
			 cloverEnergy[det1], cloverEnergy[det2]);
		    plot(ge::matrix::DD_ADD_ENERGY_PROMPT, 
			 cloverEnergy[det2], cloverEnergy[det1]);
	  
		    // beta-gated addback gamma-gamma 
		    if (betaEnergy > 0) {
			using namespace ge::matrix::betaGated;
			plot(DD_ADD_ENERGY_PROMPT,
			     cloverEnergy[det1], cloverEnergy[det2]);
			plot(DD_ADD_ENERGY_PROMPT,
			     cloverEnergy[det2], cloverEnergy[det1]);
		    } //end beta gated gammma-gamma
		    
		    //correlated beta gated addback gamma-gamma
		    if (hasDecay) {
			using namespace ge::matrix::decayGated;
			plot(DD_ADD_ENERGY_PROMPT,
			     cloverEnergy[det1], cloverEnergy[det2]);
			plot(DD_ADD_ENERGY_PROMPT,
			     cloverEnergy[det2], cloverEnergy[det1]);
		    } //end beta gated gammma-gamma
		} // end check on second clover energy
	    } // end loop over second clover      
	} // end check on clover energy
    } // end clover energy check
    
    // end plotting addback spectra
    /*********************************/
    
    EndProcess(); // update the processing time
    return true;
}

/**
 * Simple clover addback, routine is not fully complete
 *   Clover channels are grouped based on their location
 *   (i.e. 0-3 is one detector and so forth)
 */
void GeProcessor::ConstructAddback(double calEnergy, int loc, const string &subtype)
{
    //routine not finsihed and is currently crap, DO NOT USE  
    // but we use it anyway
    if (subtype == "clover_high") {
	int cloverNum = loc / chansPerClover;
	cloverEnergy[cloverNum] += calEnergy;
    } else if (subtype == "sega") {
	cout << " Add back not implemented for sega detectors yet " << endl;
    }
}
