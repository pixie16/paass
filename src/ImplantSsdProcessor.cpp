/*! \file ImplantSsdProcessor.cpp
 *
 * The implantSSD processor handles detectors of type ssd:implant and
 *   determines whether the events are implants or decays and informs the
 *   correlator accordingly
 */

#include <climits>
#include <iostream>
#include <vector>

#include "damm_plotids.h"

#include "DetectorDriver.h"
#include "ImplantSsdProcessor.h"
#include "TriggerLogicProcessor.h"
#include "RawEvent.h"

using std::cout;
using std::endl;

extern DetectorDriver driver; // need this to get the logic event processor

/*! ecutoff for 108Xe experiment where each bin is roughly 4 keV
 *  ... implants deposit above 18 MeV
 */
const double ImplantSsdProcessor::cutoffEnergy = 1100;

ImplantSsdProcessor::ImplantSsdProcessor() : EventProcessor()
{
    name = "ImplantSsd";

    associatedTypes.insert("ssd");
}

void ImplantSsdProcessor::DeclarePlots(void) const
{
    using namespace dammIds::implantSsd;

    const int unknownEnergyBins = SE;
    const int implantEnergyBins = SD; 
    const int decayEnergyBins   = SA;
    const int positionBins      = S5;
    const int timeBins          = S8;
    const int tofBins           = SD; //! DTM -- LARGE FOR NOW 
    
    DeclareHistogram2D(DD_IMPLANT_ENERGY__POSITION, 
		       implantEnergyBins, positionBins, "SSD Strip vs Implant E");
    DeclareHistogram2D(DD_DECAY_ENERGY__POSITION,
		       decayEnergyBins, positionBins, "SSD Strip vs Decay E");
    DeclareHistogram2D(DD_ENERGY__POSITION_NOBEAM,
		       unknownEnergyBins, positionBins, "SSD Strip vs E w/ no beam");

    for (int i=0; i < MAX_TOF; i++) {
      DeclareHistogram2D(DD_IMPLANT_ENERGY__TOFX+i,
			 implantEnergyBins, tofBins, "SSD Energy vs TOF (/8)", 1, SC, SC);
    }

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

    if (!EventProcessor::Process(event)) {
        EndProcess();
	return false;
    }

    static bool firstTime = true;
    static LogicProcessor *logProc = NULL;

    static Correlator &corr = event.GetCorrelator();
    static const DetectorSummary *tacSummary = event.GetSummary("generic:tac", true);
    static const DetectorSummary *impSummary = event.GetSummary("ssd:implant", true);

    if (impSummary->GetMult() == 0) {
      EndProcess();
      return false;
    }
    if (firstTime) {
      vector<EventProcessor *> vecProc = driver.GetProcessors("logic");
      for (vector< EventProcessor * >::iterator it = vecProc.begin(); it != vecProc.end(); it++) {
	if ( (*it)->GetName() == "triggerlogic" || (*it)->GetName() == "logic" ) {
	  logProc = reinterpret_cast < LogicProcessor * >(*it);
  	  cout << "Implant SSD processor grabbed logic processor" << endl;
        }
      }    
      firstTime=false;
    }

    int position;
    double energy, time;

    const ChanEvent *ch  = impSummary->GetMaxEvent();

    position = ch->GetChanID().GetLocation();
    energy   = ch->GetCalEnergy();
    time     = ch->GetTime();

    bool noBeam = (logProc && !logProc->LogicStatus(3) && !logProc->LogicStatus(4) && !logProc->LogicStatus(5));
    bool hasTAC = ( (tacSummary && tacSummary->GetMult() > 0) || !tacSummary);
    // decide whether this is an implant or a decay
    Correlator::EEventType type;
    
    if (energy > cutoffEnergy) {
        if ( hasTAC && !noBeam )
  	  type = Correlator::IMPLANT_EVENT;
        else type = Correlator::UNKNOWN_TYPE;
    } else 
	type = Correlator::DECAY_EVENT;
    // recect noise events
    if (energy < 10) {
      EndProcess();
      return true;
    }
    // give a dummy back strip postion of 1
    corr.Correlate(event, type, position, 1, time, energy); 

    // plot stuff
    if (type == Correlator::IMPLANT_EVENT) {
	// is there a fast decay?
        if (ch->GetTrace().HasValue("filterEnergy2")) {
  	    energy = ch->GetTrace().GetValue("filterEnergy");
	    // cout << " fast decay!" << endl;
	    double decayEnergy = ch->GetTrace().GetValue("filterEnergy2");
	    double dt = ch->GetTrace().GetValue("filterTime2") - 
	      ch->GetTrace().GetValue("filterTime");
	    double decayTime   = ch->GetTime() + dt;

	    corr.Correlate(event, Correlator::DECAY_EVENT, position, 1, decayTime, energy);
	    plot(DD_DECAY_ENERGY__POSITION, decayEnergy);
	    plot(DD_ENERGY__DECAY_TIME_GRANX, dt / 10e-9 * pixie::clockInSeconds);
        }
	plot(DD_IMPLANT_ENERGY__POSITION, energy, position);
	if (tacSummary) {
	  corr.SetTACValue(9999);
	  const vector<ChanEvent*> events = tacSummary->GetList();
	  for (vector<ChanEvent*>::const_iterator it = events.begin();
	       it != events.end(); it++) {
	    double tof = (*it)->GetCalEnergy();
	    int    loc = (*it)->GetChanID().GetLocation();
	    if (loc == 2)
	      corr.SetTACValue(tof);
	    plot(DD_IMPLANT_ENERGY__TOFX+loc-1, energy, tof); // we started tof locations at 1
	  }	  
	}	
    } else if (type == Correlator::DECAY_EVENT) {
	plot(DD_DECAY_ENERGY__POSITION, energy, position);
	if (noBeam) {
	    plot(DD_ENERGY__POSITION_NOBEAM, energy, position);
	}

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
    } else if (type == Correlator::UNKNOWN_TYPE) {
      // plot this only if the beam is off
      if (noBeam) {
	// dump high energy events
	if (energy > 2000 ) {	  
	  corr.Correlate(event, Correlator::DECAY_EVENT, position, 1, time, energy); 
	  cout << "High energy decay of " << energy
	       << " (raw energy = " << ch->GetEnergy() << (ch->IsSaturated() ? " saturated " : "")
	       << ") with beam absent!" <<endl;
	  corr.PrintDecayList(position, 1);
	}      
	plot(DD_ENERGY__POSITION_NOBEAM, energy, position);
      }
    }
    EndProcess(); // update the processing time
    return true;
}
