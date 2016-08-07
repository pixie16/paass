/*! \file DssdProcessor.cpp
 * \brief Processes DSSD events
 *
 * The DSSD processor handles detectors of type dssd_front and dssd_back and
 *   determines whether the events are implants or decays and informs the
 *   correlator accordingly
 */

#include <climits>

#include "DammPlotIds.hpp"

#include "DssdProcessor.hpp"
#include "RawEvent.hpp"

/*! ecutoff for 108Xe experiment where each bin is roughly 4 keV
 *  ... implants deposit above 18 MeV
 */
const double DssdProcessor::cutoffEnergy = 4500;

using namespace dammIds::dssd;

namespace dammIds {
    namespace dssd {
        const int DD_IMPLANT_POSITION = 25;//!< Implant Position
        const int DD_DECAY_POSITION   = 26;//!< Decay Position
        const int DD_IMPLANT_FRONT_ENERGY__POSITION = 41;//!< Implant Front vs. Position
        const int DD_IMPLANT_BACK_ENERGY__POSITION  = 42;//!< Implant Back vs. Position
        const int DD_DECAY_FRONT_ENERGY__POSITION   = 43;//!< Decay Front vs. Position
        const int DD_DECAY_BACK_ENERGY__POSITION    = 44;//!< Decay Back vs. Position
        const int DD_ENERGY__DECAY_TIME_GRANX = 50;//!< Energy Vs. Decay Time
    }
}


DssdProcessor::DssdProcessor() :
    EventProcessor(OFFSET, RANGE, "DssdProcessor"),
    frontSummary(NULL), backSummary(NULL) {
    name = "dssd";
    
    associatedTypes.insert("dssd_front");
    associatedTypes.insert("dssd_back");
}

void DssdProcessor::DeclarePlots(void) {
    using namespace dammIds::dssd;

    const int implantEnergyBins = SE;
    const int decayEnergyBins   = SD;
    const int positionBins = S6;
    const int timeBins     = S8;

    DeclareHistogram2D(DD_IMPLANT_FRONT_ENERGY__POSITION,
		       implantEnergyBins, positionBins, "DSSD Strip vs E - RF");
    DeclareHistogram2D(DD_IMPLANT_BACK_ENERGY__POSITION,
		       implantEnergyBins, positionBins, "DSSD Strip vs E - RB");
    DeclareHistogram2D(DD_DECAY_FRONT_ENERGY__POSITION,
		       decayEnergyBins, positionBins, "DSSD Strip vs E - DF");
    DeclareHistogram2D(DD_DECAY_BACK_ENERGY__POSITION,
		       decayEnergyBins, positionBins, "DSSD Strip vs E - DB");
    DeclareHistogram2D(DD_IMPLANT_POSITION,
		       positionBins, positionBins, "DSSD Hit Pattern - R");
    DeclareHistogram2D(DD_DECAY_POSITION,
		       positionBins, positionBins, "DSSD Hit Pattern - D");

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

bool DssdProcessor::Process(RawEvent &event) {
    using namespace dammIds::dssd;

    if (!EventProcessor::Process(event))
	return false;
    // first time through, grab the according detector summaries
    if (frontSummary == NULL)
	frontSummary = event.GetSummary("dssd_front");
    if (backSummary == NULL)
	backSummary = event.GetSummary("dssd_back");
    //Removed this until it can be updated with the TreeCorrelator
    //static Correlator &corr = event.GetCorrelator();

    int frontPos = INT_MAX, backPos = INT_MAX;
    double frontEnergy, backEnergy, frontTime = 0.;

    bool hasFront = (frontSummary->GetMult() > 0);
    bool hasBack  = (backSummary->GetMult() > 0);
    bool hasMcp   = (event.GetSummary("mcp")->GetMult() > 0);

    if (hasFront) {
	const ChanEvent *ch = frontSummary->GetMaxEvent();

	frontPos    = ch->GetChanID().GetLocation();
	frontEnergy = ch->GetCalEnergy();
	frontTime   = ch->GetTime();
    } else frontEnergy = 0.;
    if (hasBack) {
	const ChanEvent *ch = backSummary->GetMaxEvent();

	backPos    = ch->GetChanID().GetLocation();
	backEnergy = ch->GetCalEnergy();
    } else backEnergy = 0.;

    // decide whether this is an implant or a decay
    EventInfo corEvent;
    corEvent.time = frontTime;

    if (hasFront && hasBack) {
	if (frontEnergy > cutoffEnergy && backEnergy > cutoffEnergy && hasMcp) {
	    corEvent.type = EventInfo::IMPLANT_EVENT;
	} else if (frontEnergy <= cutoffEnergy && backEnergy <= cutoffEnergy &&
		   !hasMcp) {
	    corEvent.type = EventInfo::DECAY_EVENT;
	} else {
	    corEvent.type = EventInfo::UNKNOWN_EVENT;
	}
	corr.Correlate(corEvent, frontPos, backPos);
    } else if (hasFront) {
  	if (frontEnergy > cutoffEnergy) {
	    corEvent.type = EventInfo::IMPLANT_EVENT;
	} else corEvent.type = EventInfo::DECAY_EVENT;

    } else if (hasBack) {
	if (backEnergy > cutoffEnergy) {
	    corEvent.type = EventInfo::IMPLANT_EVENT;
	} else corEvent.type = EventInfo::DECAY_EVENT;
    } else corEvent.type = EventInfo::UNKNOWN_EVENT;

    // plot stuff
    if (corEvent.type == EventInfo::IMPLANT_EVENT) {
	if (hasFront)
	    plot(DD_IMPLANT_FRONT_ENERGY__POSITION, frontEnergy, frontPos);
	if (hasBack)
	    plot(DD_IMPLANT_BACK_ENERGY__POSITION, backEnergy, backPos);
	if (hasFront && hasBack)
	    plot(DD_IMPLANT_POSITION, backPos, frontPos);
    } else if (corEvent.type == EventInfo::DECAY_EVENT) {
	if (hasFront)
	    plot(DD_DECAY_FRONT_ENERGY__POSITION, frontEnergy, frontPos);
	if (hasBack)
	    plot(DD_DECAY_BACK_ENERGY__POSITION, backEnergy, backPos);
	if (hasFront && hasBack)
	    plot(DD_DECAY_POSITION, backPos, frontPos);
	if (corr.GetCondition() == Correlator::VALID_DECAY) {
	    const unsigned int NumGranularities = 8;
	    // time resolution in seconds per bin
	    const double timeResolution[NumGranularities] =
		{10e-9, 100e-9, 400e-9, 1e-6, 100e-6, 1e-3, 10e-3, 100e-3};

	    for (unsigned int i = 0; i < NumGranularities; i++) {
		int timeBin = int(corr.GetDecayTime() *
                Globals::get()->clockInSeconds() / timeResolution[i]);

		plot(DD_ENERGY__DECAY_TIME_GRANX + i, frontEnergy, timeBin);
	    }
	}
    }

    EndProcess(); // update the processing time
    return true;
}
