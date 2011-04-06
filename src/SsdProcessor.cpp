/*! \file SsdProcessor.cpp
 *
 * The SSD processor handles detectors of type ssd_1,2,3  */

#include "damm_plotids.h"

#include "SsdProcessor.h"
#include "RawEvent.h"

#include <iostream>
 
using std::cout;
using std::endl;

SsdProcessor::SsdProcessor() :
  EventProcessor(), ssdSummary(NULL)
{
    name = "ssd";
    associatedTypes.insert("ssd");
}

void SsdProcessor::DeclarePlots(void) const
{
    using namespace dammIds::ssd;
    
    const int energyBins    = SE; 
    const int positionBins  = S5;
    // const int timeBins     = S8;

    DeclareHistogram2D(SSD1_POSITION_ENERGY, 
    		       energyBins, positionBins1, "SSD1 Strip vs E");
    DeclareHistogram2D(SSD2_POSITION_ENERGY, 
    		       energyBins, positionBins1, "SSD2 Strip vs E");
    DeclareHistogram2D(SSD3_POSITION_ENERGY, 
    		       energyBins, positionBins1, "SSD3 Strip vs E");
    DeclareHistogram2D(SSD4_POSITION_ENERGY, 
    		       energyBins, positionBins1, "SSD4 Strip vs E");
}

bool SsdProcessor::Process(RawEvent &event)
{
    using namespace dammIds::ssd;
    if (!EventProcessor::Process(event))
	return false;
    
    bool hasSsd = (sumMap["ssd"]->GetMult() > 0);

    if (hasSsd) {
	double ssdEnergy, ssdTime = 0.;
	string ssdSubtype="";

	const ChanEvent *ch = ssdSummary->GetMaxEvent();
        ssdSubtype = ch->GetChanID().GetSubtype();
      	ssdPos     = ch->GetChanID().GetLocation();
	//	cout <<" TST " <<ssdSubtype << ssdPos <<endl;
	ssdEnergy = ch->GetCalEnergy();
	ssdTime   = ch->GetTime();
	// plot stuff
	if (ssdSubtype=="ssd_1")
	    plot(SSD1_POSITION_ENERGY, ssdEnergy, ssdPos);
	else if (ssdSubtype=="ssd_2")
	    plot(SSD2_POSITION_ENERGY, ssdEnergy, ssdPos-16);
	else if (ssdSubtype=="ssd_3")
	    plot(SSD3_POSITION_ENERGY, ssdEnergy, ssdPos-32);
	else if (ssdSubtype=="ssd_4")
	    plot(SSD4_POSITION_ENERGY, ssdEnergy, ssdPos-48);
    } else ssdEnergy = 0.;

    EndProcess(); // update the processing time
    return true;
}

