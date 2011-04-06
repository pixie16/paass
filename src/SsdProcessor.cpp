/*! \file SsdProcessor.cpp
 *
 * The SSD processor handles detectors of type ssd_1,2,3  */

#include "damm_plotids.h"

#include "SsdProcessor.h"
#include "RawEvent.h"
#include <limits.h>
#include <iostream>
#include <fstream>
 
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

    const int EnergyBins = SE; 
    const int positionBins1 = S5;
    const int positionBins = S4;
    const int timeBins     = S8;

    DeclareHistogram2D(SSD1_POSITION_ENERGY, 
    		       EnergyBins, positionBins1, "SSD1 Strip vs E - RF");
    DeclareHistogram2D(SSD2_POSITION_ENERGY, 
    		       EnergyBins, positionBins1, "SSD2 Strip vs E - RF");
    DeclareHistogram2D(SSD3_POSITION_ENERGY, 
    		       EnergyBins, positionBins1, "SSD3 Strip vs E - RF");
    DeclareHistogram2D(SSD4_POSITION_ENERGY, 
    		       EnergyBins, positionBins1, "SSD4 Strip vs E - RF");
}

bool SsdProcessor::Process(RawEvent &event)
{
    using namespace dammIds::ssd;
    if (!EventProcessor::Process(event))
	return false;
    // first time through, grab the according detector summaries
    if (ssdSummary == NULL)
	ssdSummary = event.GetSummary("ssd");

    //    static DetectorSummary *mcpSummary = event.GetSummary("mcp");
    //   static Correlator &corr = event.GetCorrelator();

    int ssdPos = UINT_MAX;
    double ssdEnergy, ssdTime = 0.;
    string ssdSubtype="";

    bool hasSsd = ssdSummary && (ssdSummary->GetMult() > 0);
    // bool hasMcp   = mcpSummary && (mcpSummary->GetMult() > 0);

    string WhichSsd = ssdSummary-> GetName();

    //    cout <<" TST " <<WhichSsd <<endl;

    if (hasSsd) {
	const ChanEvent *ch = ssdSummary->GetMaxEvent();
        ssdSubtype =  ch->GetChanID().GetSubtype();

      	ssdPos    = ch->GetChanID().GetLocation();
	//	cout <<" TST " <<ssdSubtype << ssdPos <<endl;
	ssdEnergy = ch->GetCalEnergy();
	ssdTime   = ch->GetTime();
    } else ssdEnergy = 0.;

    // plot stuff

    if (ssdSubtype=="ssd_1")
      plot(SSD1_POSITION_ENERGY, ssdEnergy, ssdPos);

    if (ssdSubtype=="ssd_2")
      plot(SSD2_POSITION_ENERGY, ssdEnergy, ssdPos-16);

    if (ssdSubtype=="ssd_3")
      plot(SSD3_POSITION_ENERGY, ssdEnergy, ssdPos-32);

    if (ssdSubtype=="ssd_4")
      plot(SSD4_POSITION_ENERGY, ssdEnergy, ssdPos-48);

    EndProcess(); // update the processing time
    return true;
}


