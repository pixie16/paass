/*! \file SsdProcessor.cpp
 *
 * The SSD processor handles detectors of type ssd */

#include "damm_plotids.h"

#include "SsdProcessor.h"
#include "RawEvent.h"

#include <iostream>
 
using std::cout;
using std::endl;

using namespace dammIds::ssd;

namespace dammIds {
    namespace ssd {
        const int NUM_DETECTORS = 4;
        const int DD_POSITION__ENERGY_DETX = 1; // for x detectors
    }
} // ssd namespace

SsdProcessor::SsdProcessor() : EventProcessor(OFFSET, RANGE)
{
    name = "ssd";
    associatedTypes.insert("ssd");
}

void SsdProcessor::DeclarePlots(void)
{
    using namespace dammIds::ssd;
    
    const int energyBins    = SE; 
    const int positionBins  = S5;
    // const int timeBins     = S8;

    for (int i=0; i < NUM_DETECTORS; i++) {
        DeclareHistogram2D(DD_POSITION__ENERGY_DETX + i,
                energyBins, positionBins, "SSD Strip vs E");
    }
}

bool SsdProcessor::Process(RawEvent &event)
{
    using namespace dammIds::ssd;
    if (!EventProcessor::Process(event))
	return false;

    int position;    
    double energy, time;

    static bool firstTime = true;
    static const DetectorSummary *ssdSummary[NUM_DETECTORS];

    if (firstTime) {
        ssdSummary[0] = event.GetSummary("ssd:implant");
        ssdSummary[1] = event.GetSummary("ssd:box");
        ssdSummary[2] = event.GetSummary("ssd:digisum");
        ssdSummary[3] = event.GetSummary("ssd:ssd_4");
        firstTime = false;
    }
    
    for (int i = 0; i < NUM_DETECTORS; i++) {
        if (ssdSummary[i]->GetMult() == 0)
            continue;
        const ChanEvent *ch = ssdSummary[i]->GetMaxEvent();
        position = ch->GetChanID().GetLocation();
        energy   = ch->GetCalEnergy();
        time     = ch->GetTime();

        plot(DD_POSITION__ENERGY_DETX + i, energy, position);
    }

    EndProcess(); // update the processing time
    return true;
}

