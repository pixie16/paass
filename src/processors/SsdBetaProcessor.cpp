/*! \file SsdBetaProcessor.cpp
 *
 * The SSD processor handles detectors of type ssd_1,2,3  */
#include <limits>
#include <iostream>
#include <fstream>

#include "DammPlotIds.hpp"
#include "RawEvent.hpp"
#include "SsdProcessor.hpp"

using std::cout;
using std::endl;

SsdProcessor::SsdProcessor() :
  EventProcessor(), ssdSummary(NULL) {
    name = "SsdBetaProcessor";
    associatedTypes.insert("ssd");
}

void SsdProcessor::DeclarePlots(void) const {
    using namespace dammIds::ssd;

    const int EnergyBins = SE;
    const int positionBins = S4;
    const int timeBins     = S8;

       DeclareHistogram2D(SSD1_POSITION_ENERGY,
    		       EnergyBins, positionBins, "SSD1 Strip vs E - RF");
       DeclareHistogram2D(SSD2_POSITION_ENERGY,
    		       EnergyBins, positionBins, "SSD2 Strip vs E - RF");
       DeclareHistogram2D(SSD3_POSITION_ENERGY,
    		       EnergyBins, positionBins, "SSD3 Strip vs E - RF");
}

bool SsdProcessor::Process(RawEvent &event) {
    using namespace dammIds::ssd;
    if (!EventProcessor::Process(event))
        return false;
    if (ssdSummary == NULL)
        ssdSummary = event.GetSummary("ssd");

    int ssdPos = UINT_MAX;
    double ssdEnergy, ssdTime = 0.;
    string ssdSubtype="";

    bool hasSsd = ssdSummary && (ssdSummary->GetMult() > 0);

    string WhichSsd = ssdSummary-> GetName();

    if (hasSsd) {
	const ChanEvent *ch = ssdSummary->GetMaxEvent();
        ssdSubtype =  ch->GetChanID().GetSubtype();

      	ssdPos    = ch->GetChanID().GetLocation();
	ssdEnergy = ch->GetCalEnergy();
	ssdTime   = ch->GetTime();
    } else
        ssdEnergy = 0.;

    if (ssdSubtype=="ssd_1")
      plot(SSD1_POSITION_ENERGY, ssdEnergy, ssdPos);

    if (ssdSubtype=="ssd_2")
      plot(SSD2_POSITION_ENERGY, ssdEnergy, ssdPos-8);

    if (ssdSubtype=="ssd_3")
      plot(SSD3_POSITION_ENERGY, ssdEnergy, ssdPos-16);

    EndProcess(); // update the processing time
    return true;
}


