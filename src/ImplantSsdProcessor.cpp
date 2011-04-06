/*! \file ImplantSsdProcessor.cpp
 *
 * The implantSSD processor handles detectors of type ssd:implant and
 *   determines whether the events are implants or decays and informs the
 *   correlator accordingly
 */

#include <climits>

#include "damm_plotids.h"

#include "ImplantSsdProcessor.h"
#include "RawEvent.h"

/*! ecutoff for 108Xe experiment where each bin is roughly 4 keV
 *  ... implants deposit above 18 MeV
 */
const double ImplantSsdProcessor::cutoffEnergy = 4500;

ImplantSsdProcessor::ImplantSsdProcessor() : EventProcessor()
{
    name = "ImplantSsd";

    associatedTypes.insert("ssd:implant");
}

void ImplantSsdProcessor::DeclarePlots(void) const
{
    using namespace dammIds::implantSsd;

    const int implantEnergyBins = SE; 
    const int decayEnergyBins   = SD;
    const int positionBins = S6;
    const int timeBins     = S8;

    DeclareHistogram2D(DD_IMPLANT_ENERGY__POSITION, 
		       implantEnergyBins, positionBins, "SSD Strip vs Implant E");
    DeclareHistogram2D(DD_DECAY_ENERGY__POSITION,
		       decayEnergyBins, positionBins, "SSD Strip vs Decay E");

    DeclareHistogram2D(DD_ENERGY__DECAY_TIME_GRANX + 0, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (10ns/ch)(xkeV)");
    DeclareHistogram2D(DD_ENERGY__DECAY_TIME_GRANX + 1, decayEnergyBins, timeBins, 
		       "DSSD Ty,Ex (100ns/ch)(xkeV)");
    DeclareHistogram2D(DD_ENERGY__DECAY_TIME_GRANX + 2, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (400ns/ch)(xkeV)");
    DeclareHistogram2D(DD_ENERGY__DECAY_TIME_GRANX + 3, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (1us/ch)(xkeV)");
    DeclareHistogram2D(DD_ENERGY__DECAY_TIME_GRANX + 4, decayEnergyBins, timeBins, 
		       "DSSD Ty,Ex (10us/ch)(xkeV)");
    DeclareHistogram2D(DD_ENERGY__DECAY_TIME_GRANX + 5, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (100us/ch)(xkeV)");
    DeclareHistogram2D(DD_ENERGY__DECAY_TIME_GRANX + 6, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (1ms/ch)(xkeV)");
    DeclareHistogram2D(DD_ENERGY__DECAY_TIME_GRANX + 7, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (10ms/ch)(xkeV)");
    DeclareHistogram2D(DD_ENERGY__DECAY_TIME_GRANX + 8, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (100ms/ch)(xkeV)");
}

bool ImplantSsdProcessor::Process(RawEvent &event)
{
    using namespace dammIds::implantSsd;

    if (!EventProcessor::Process(event))
	return false;
    static Correlator &corr = event.GetCorrelator();

    int position;
    double energy, time;

    const ChanEvent *ch = sumMap["ssd:implant"]->GetMaxEvent();

    position = ch->GetChanID().GetLocation();
    energy   = ch->GetCalEnergy();
    time     = ch->GetTime();

    // decide whether this is an implant or a decay
    Correlator::EEventType type;

    if (energy > cutoffEnergy)
	type = Correlator::IMPLANT_EVENT;
    else 
	type = Correlator::DECAY_EVENT;

    // give a dummy back strip postion of 1
    corr.Correlate(event, type, position, 1, time); 

    // plot stuff
    if (type == Correlator::IMPLANT_EVENT) {
	plot(DD_IMPLANT_ENERGY__POSITION, energy, position);
    } else if (type == Correlator::DECAY_EVENT) {
	plot(DD_DECAY_ENERGY__POSITION, energy, position);
	if (corr.GetCondition() == Correlator::VALID_DECAY) {
	    const unsigned int NumGranularities = 8;
	    // time resolution in seconds per bin
	    const double timeResolution[NumGranularities] = 
		{10e-9, 100e-9, 400e-9, 1e-6, 100e-6, 1e-3, 10e-3, 100e-3};
	 
	    for (unsigned int i = 0; i < NumGranularities; i++) {
		int timeBin = int(corr.GetDecayTime() * pixie::clockInSeconds / 
				  timeResolution[i]);

		plot(DD_ENERGY__DECAY_TIME_GRANX + i, energy, timeBin);
	    }
	}
    }

    EndProcess(); // update the processing time
    return true;
}
