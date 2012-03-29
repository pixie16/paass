/*! \file ImplantSsdProcessor.cpp
 *
 * The implantSSD processor handles detectors of type ssd:implant and
 *   determines whether the events are implants or decays and informs the
 *   correlator accordingly
 */

#include <cfloat> // for DBL_MAX
#include <climits>
#include <iostream>
#include <sstream>
#include <vector>

#include "damm_plotids.h"

#include "Correlator.h"
#include "DetectorDriver.h"
#include "ImplantSsdProcessor.h"
#include "TriggerLogicProcessor.h"
#include "RawEvent.h"

using std::cout;
using std::endl;
using std::min;
using std::stringstream;

extern DetectorDriver driver; // need this to get the logic event processor

/*! ecutoff for 108Xe experiment where each bin is roughly 4 keV
 *  ... implants deposit above 18 MeV
 */
const double ImplantSsdProcessor::cutoffEnergy  = 1500;
const double ImplantSsdProcessor::fissionThresh = 4400;
const double ImplantSsdProcessor::goodAlphaCut  = 950;
const double ImplantSsdProcessor::implantTof    = 2800;

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
    const int vetoPositionBins  = S3;
    const int timeBins          = S8;
    const int tofBins           = SD; //! DTM -- LARGE FOR NOW 
    const int traceBins         = SC;
    const int tdiffBins         = SA;

    DeclareHistogram2D(DD_ALL_ENERGY__POSITION, 
		       implantEnergyBins, positionBins, "SSD Strip vs Implant E");
    DeclareHistogram2D(DD_IMPLANT_ENERGY__POSITION, 
		       implantEnergyBins, positionBins, "SSD Strip vs Implant E");
    DeclareHistogram2D(DD_DECAY_ENERGY__POSITION,
		       decayEnergyBins, positionBins, "SSD Strip vs Decay E");
    DeclareHistogram2D(DD_ENERGY__POSITION_BEAM,
		       unknownEnergyBins, positionBins, "SSD Strip vs E w/ beam");
    DeclareHistogram2D(DD_ENERGY__POSITION_NOBEAM,
		       unknownEnergyBins, positionBins, "SSD Strip vs E w/ no beam");
    DeclareHistogram2D(DD_ENERGY__POSITION_VETO,
		       implantEnergyBins, positionBins, "SSD Strip vs E w/ veto");
    DeclareHistogram2D(DD_ENERGY__POSITION_PROJLIKE,
		       implantEnergyBins, positionBins, "SSD Strip vs E projectile");
    DeclareHistogram2D(DD_ENERGY__POSITION_UNKNOWN,
		       unknownEnergyBins, positionBins, "SSD Strip vs E (unknown)");
    
    DeclareHistogram2D(DD_POS_VETO__POS_SSD, vetoPositionBins, positionBins,
		       "Veto pos vs SSD pos");
    DeclareHistogram2D(DD_TOTENERGY__ENERGY, implantEnergyBins, implantEnergyBins,
		       "Tot energy vs. SSD energy (/8)", 1, S3, S3);

    DeclareHistogram2D(DD_DECAY_ALL_ENERGY__TX + 0, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (10ns/ch)(xkeV)");
    DeclareHistogram2D(DD_DECAY_ALL_ENERGY__TX + 1, decayEnergyBins, timeBins, 
		       "DSSD Ty,Ex (100ns/ch)(xkeV)");
    DeclareHistogram2D(DD_DECAY_ALL_ENERGY__TX + 2, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (400ns/ch)(xkeV)");
    DeclareHistogram2D(DD_DECAY_ALL_ENERGY__TX + 3, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (1us/ch)(xkeV)");
    DeclareHistogram2D(DD_DECAY_ALL_ENERGY__TX + 4, decayEnergyBins, timeBins, 
		       "DSSD Ty,Ex (10us/ch)(xkeV)");
    DeclareHistogram2D(DD_DECAY_ALL_ENERGY__TX + 5, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (100us/ch)(xkeV)");
    DeclareHistogram2D(DD_DECAY_ALL_ENERGY__TX + 6, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (1ms/ch)(xkeV)");
    DeclareHistogram2D(DD_DECAY_ALL_ENERGY__TX + 7, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (10ms/ch)(xkeV)");
    DeclareHistogram2D(DD_DECAY_ALL_ENERGY__TX + 8, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (100ms/ch)(xkeV)");

    DeclareHistogram2D(DD_DECAY_NOBEAM_ENERGY__TX + 0, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (10ns/ch)(xkeV)");
    DeclareHistogram2D(DD_DECAY_NOBEAM_ENERGY__TX + 1, decayEnergyBins, timeBins, 
		       "DSSD Ty,Ex (100ns/ch)(xkeV)");
    DeclareHistogram2D(DD_DECAY_NOBEAM_ENERGY__TX + 2, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (400ns/ch)(xkeV)");
    DeclareHistogram2D(DD_DECAY_NOBEAM_ENERGY__TX + 3, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (1us/ch)(xkeV)");
    DeclareHistogram2D(DD_DECAY_NOBEAM_ENERGY__TX + 4, decayEnergyBins, timeBins, 
		       "DSSD Ty,Ex (10us/ch)(xkeV)");
    DeclareHistogram2D(DD_DECAY_NOBEAM_ENERGY__TX + 5, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (100us/ch)(xkeV)");
    DeclareHistogram2D(DD_DECAY_NOBEAM_ENERGY__TX + 6, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (1ms/ch)(xkeV)");
    DeclareHistogram2D(DD_DECAY_NOBEAM_ENERGY__TX + 7, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (10ms/ch)(xkeV)");
    DeclareHistogram2D(DD_DECAY_NOBEAM_ENERGY__TX + 8, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (100ms/ch)(xkeV)");

    DeclareHistogram2D(DD_VETO_ENERGY__TX + 0, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (10ns/ch)(xkeV)");
    DeclareHistogram2D(DD_VETO_ENERGY__TX + 1, decayEnergyBins, timeBins, 
		       "DSSD Ty,Ex (100ns/ch)(xkeV)");
    DeclareHistogram2D(DD_VETO_ENERGY__TX + 2, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (400ns/ch)(xkeV)");
    DeclareHistogram2D(DD_VETO_ENERGY__TX + 3, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (1us/ch)(xkeV)");
    DeclareHistogram2D(DD_VETO_ENERGY__TX + 4, decayEnergyBins, timeBins, 
		       "DSSD Ty,Ex (10us/ch)(xkeV)");
    DeclareHistogram2D(DD_VETO_ENERGY__TX + 5, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (100us/ch)(xkeV)");
    DeclareHistogram2D(DD_VETO_ENERGY__TX + 6, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (1ms/ch)(xkeV)");
    DeclareHistogram2D(DD_VETO_ENERGY__TX + 7, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (10ms/ch)(xkeV)");
    DeclareHistogram2D(DD_VETO_ENERGY__TX + 8, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (100ms/ch)(xkeV)");

    DeclareHistogram2D(DD_FIRST_DECAY_ENERGY__TX + 0, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (10ns/ch)(xkeV)");
    DeclareHistogram2D(DD_FIRST_DECAY_ENERGY__TX + 1, decayEnergyBins, timeBins, 
		       "DSSD Ty,Ex (100ns/ch)(xkeV)");
    DeclareHistogram2D(DD_FIRST_DECAY_ENERGY__TX + 2, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (400ns/ch)(xkeV)");
    DeclareHistogram2D(DD_FIRST_DECAY_ENERGY__TX + 3, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (1us/ch)(xkeV)");
    DeclareHistogram2D(DD_FIRST_DECAY_ENERGY__TX + 4, decayEnergyBins, timeBins, 
		       "DSSD Ty,Ex (10us/ch)(xkeV)");
    DeclareHistogram2D(DD_FIRST_DECAY_ENERGY__TX + 5, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (100us/ch)(xkeV)");
    DeclareHistogram2D(DD_FIRST_DECAY_ENERGY__TX + 6, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (1ms/ch)(xkeV)");
    DeclareHistogram2D(DD_FIRST_DECAY_ENERGY__TX + 7, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (10ms/ch)(xkeV)");
    DeclareHistogram2D(DD_FIRST_DECAY_ENERGY__TX + 8, decayEnergyBins, timeBins,
		       "DSSD Ty,Ex (100ms/ch)(xkeV)");

    for (int i=0; i < MAX_TOF; i++) {
      DeclareHistogram2D(DD_ALL_ENERGY__TOFX+i,
			 implantEnergyBins, tofBins, "SSD Energy vs TOF (/16)", 1, S4, S4);
      DeclareHistogram2D(DD_IMPLANT_ENERGY__TOFX+i,
			 implantEnergyBins, tofBins, "Implant Energy vs TOF (/16)", 1, S4, S4);
      DeclareHistogram2D(DD_VETO_ENERGY__TOFX+i,
			 implantEnergyBins, tofBins, "Veto Energy vs TOF (/16)", 1, S4, S4);
      DeclareHistogram2D(DD_ALL_ENERGY__TOFX_GATED+i,
			 implantEnergyBins, tofBins, "SSD Energy vs TOF (/16), gated", 1, S4, S4);
    }

    DeclareHistogram1D(D_TDIFF_FOIL_IMPLANT, tdiffBins, "DT foil to implant");
    DeclareHistogram1D(D_TDIFF_FOIL_IMPLANT_MULT1, tdiffBins, "DT foil to implant, mult. gated");

    for (unsigned int i=0; i < numTraces; i++) {
	DeclareHistogram1D(D_FAST_DECAY_TRACE + i, traceBins, "fast decay trace");
	DeclareHistogram1D(D_HIGH_ENERGY_TRACE + i, traceBins, "high energy trace");
    }
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
    static const DetectorSummary *tacSummary  = event.GetSummary("generic:tac", true);
    static const DetectorSummary *impSummary  = event.GetSummary("ssd:implant", true);
    static const DetectorSummary *mcpSummary  = event.GetSummary("logic:mcp", true);
    static const DetectorSummary *vetoSummary = event.GetSummary("ssd:veto", true);
    static const DetectorSummary *boxSummary  = event.GetSummary("ssd:box", true);

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

    EventInfo info;
    const ChanEvent *ch  = impSummary->GetMaxEvent();
    info.hasVeto = ( vetoSummary && vetoSummary->GetMult() > 0 );

    int position = ch->GetChanID().GetLocation();
    if (ch->IsSaturated()) {
	info.energy = 16000; // arbitrary large number
    } else {	
	info.energy  = ch->GetCalEnergy();
    }
    
    info.time    = ch->GetTime();
    info.beamOn  = true;

    // recect noise events
    if (info.energy < 10) {
	EndProcess();
	return true;
    }

    if (logProc) {
	info.clockCount = logProc->StartCount(2);
	if (logProc->LogicStatus(3) || logProc->LogicStatus(4) || logProc->LogicStatus(5)) {
	    info.beamOn = true;
	    info.offTime = 0;
	} else {
	    info.beamOn = false;
	    if (!logProc->LogicStatus(3))
		info.offTime = logProc->TimeOff(3, info.time);
	    else if (!logProc->LogicStatus(4))
		info.offTime = logProc->TimeOff(4, info.time) + 300e-6;
	    else if (!logProc->LogicStatus(5))
		info.offTime = logProc->TimeOff(5, info.time) + 600e-6;
	}
	for (int i=0; i < dammIds::logic::MAX_LOGIC; i++) {
	    info.logicBits[i] = (logProc->LogicStatus(i) ? '1' : '0');
	}
    }
    double digitalTof = NAN;
    if (mcpSummary) {
	info.mcpMult = mcpSummary->GetMult();
	vector<ChanEvent*> mcpEvents = mcpSummary->GetList();
	
	double dtMin = DBL_MAX;

	for (vector<ChanEvent*>::iterator it = mcpEvents.begin();
	     it != mcpEvents.end(); it++) {
	    double dt = info.time - (*it)->GetTime();

	    plot(D_TDIFF_FOIL_IMPLANT, 500 + dt);
	    if (mcpEvents.size() == 1) {
		digitalTof = -10.*dt;
		plot(D_TDIFF_FOIL_IMPLANT_MULT1, 500 + dt);
	    }
	    dtMin = min(dtMin, dt);
	}
	if (dtMin != DBL_MAX)
	    info.foilTime= dtMin;
    } else {
	info.mcpMult  = 0;
	info.foilTime = NAN;
    }
    if (impSummary) {
	info.impMult = impSummary->GetMult();	
    } else {
	info.impMult = 0;
    }
    if (boxSummary) {
	info.boxMult = boxSummary->GetMult();
	
	if (info.boxMult > 0) {
	    const ChanEvent *boxCh = boxSummary->GetMaxEvent();

	    info.energyBox = boxCh->GetCalEnergy(); //raw?
	    info.boxMax = boxCh->GetChanID().GetLocation();
	} else {
	    info.energyBox = 0;
	    info.boxMax = -1;
	}
    } else {
	info.boxMult = 0;
    }
    info.tof = NAN;
    if (tacSummary) {
	const vector<ChanEvent*> events = tacSummary->GetList();
	for (vector<ChanEvent*>::const_iterator it = events.begin(); it != events.end(); it++) {
	    int loc = (*it)->GetChanID().GetLocation();
	    if (loc == 2) {
		info.tof = (*it)->GetCalEnergy();
		info.hasTof = true;
		break;
	    }
	}
    } else {
	info.hasTof = true;
    }

    const Trace &trace = ch->GetTrace();
    if (trace.HasValue("filterEnergy2")) {
	info.pileUp = true;
    }
    
    SetType(info);
    // give a dummy back strip postion of 1
    corr.Correlate(info, position, 1);
    PlotType(info, position, corr.GetCondition());

    // TOF spectra update
    if (tacSummary) {
	const vector<ChanEvent*> events = tacSummary->GetList();
	for (vector<ChanEvent*>::const_iterator it = events.begin();
	     it != events.end(); it++) {
	    double tof  = (*it)->GetCalEnergy();
	    int ntof = (*it)->GetChanID().GetLocation();

	    plot(DD_ALL_ENERGY__TOFX + ntof - 1, info.energy, tof);
	    if (!isnan(digitalTof) && digitalTof > 1500. && digitalTof < 2000)
		plot(DD_ALL_ENERGY__TOFX_GATED + ntof - 1, info.energy, tof);
	    if (info.type == EventInfo::IMPLANT_EVENT) {
		plot(DD_IMPLANT_ENERGY__TOFX + ntof - 1, info.energy, tof);
	    } else if (info.type == EventInfo::PROTON_EVENT) {
		plot(DD_VETO_ENERGY__TOFX + ntof - 1, info.energy, tof);
	    }
	}
	// TAC channel 0 is missing, so use it for the digital tof
	if (!isnan(digitalTof)) {
	    plot(DD_ALL_ENERGY__TOFX, info.energy, digitalTof);
	    if (info.type == EventInfo::IMPLANT_EVENT) {
		plot(DD_IMPLANT_ENERGY__TOFX, info.energy, digitalTof);
	    } else if (info.type == EventInfo::PROTON_EVENT) {
		plot(DD_VETO_ENERGY__TOFX, info.energy, digitalTof);
	    }
	}	
    }

    if (info.type == EventInfo::PROTON_EVENT) {
	const ChanEvent *chVeto = vetoSummary->GetMaxEvent();
	
	unsigned int posVeto = chVeto->GetChanID().GetLocation();
	double vetoEnergy = chVeto->GetCalEnergy();

	plot(DD_POS_VETO__POS_SSD, posVeto, position);
	plot(DD_TOTENERGY__ENERGY, vetoEnergy + info.energy, info.energy);
    }

    if (info.pileUp) {
	double trigTime = info.time;

	info.energy = driver.cal.at(ch->GetID()).Calibrate(trace.GetValue("filterEnergy2"));
	info.time   = trigTime + trace.GetValue("filterTime2") - trace.GetValue("filterTime");
	
	SetType(info);
	
	corr.Correlate(info, position, 1);
	PlotType(info, position, corr.GetCondition());

	int numPulses = trace.GetValue("numPulses");

	if ( numPulses > 2 ) {
	    corr.Flag(position, 1);
	    cout << "Flagging triple event" << endl;
	    for (int i=3; i <= numPulses; i++) {
		stringstream str;
		str << "filterEnergy" << i;
		info.energy = driver.cal.at(ch->GetID()).Calibrate(trace.GetValue(str.str()));
		str.str(""); // clear it
		str << "filterTime" << i;
		info.time   = trigTime + trace.GetValue(str.str()) - trace.GetValue("filterTime");

		SetType(info);
		corr.Correlate(info, position, 1);
	    }
	}	
	// corr.Flag(position, 1);	                
#ifdef VERBOSE
	cout << "Flagging for pileup" << endl; 

	cout << "fast trace " << fastTracesWritten << " in strip " << position
	     << " : " << trace.GetValue("filterEnergy") << " " << trace.GetValue("filterTime") 
	     << " , " << trace.GetValue("filterEnergy2") << " " << trace.GetValue("filterTime2") << endl;
	cout << "  mcp mult " << info.mcpMult << endl;
#endif // VERBOSE

	if (fastTracesWritten < numTraces) {
	    trace.Plot(D_FAST_DECAY_TRACE + fastTracesWritten);
	    fastTracesWritten++;
	}
    }

    /*
    if (info.energy > 8000)
	corr.Flag(position, 1);
    */

    if (info.energy > 8000 && !trace.empty()) {
#ifdef VERBOSE
	cout << "high energy decay of " << info.energy
	     << "(raw energy " << ch->GetEnergy() << ") with beam "
	     << (info.beamOn ? "present" : "absent") << endl;

	cout << "Flagging for high energy " << info.energy << " with trace" << endl;
#endif //VERBOSE
	// corr.Flag(position, 1);

	if (highTracesWritten < numTraces) {
	    trace.Plot(D_HIGH_ENERGY_TRACE + highTracesWritten);
	    highTracesWritten++;
	}
    }

    EndProcess(); // update the processing time
    return true;
}

EventInfo::EEventTypes ImplantSsdProcessor::SetType(EventInfo &info) const
{
    if (info.hasVeto) {
	return (info.type = EventInfo::PROTON_EVENT);
    }

    bool enoughMcp = (info.mcpMult >= info.impMult);
    // high energy events
    if (info.energy > cutoffEnergy) {
	if ( info.beamOn) {
	    if (info.hasTof && enoughMcp) {
		if (info.tof > implantTof) {
		    return (info.type = EventInfo::IMPLANT_EVENT);
		}
		return (info.type = EventInfo::PROJECTILE_EVENT);
	    } else { 
		// NO TAC OR MCP
		if (info.energy > fissionThresh && !info.hasTof && !enoughMcp)

		    return (info.type = EventInfo::FISSION_EVENT);
		// Implant with lost TAC or MCP possibly?
		return (info.type = EventInfo::UNKNOWN_EVENT);
	    }
	}
	// NO BEAM
	if (info.hasTof || enoughMcp) {
	    return (info.type = EventInfo::UNKNOWN_EVENT);
	}
	if (info.energy > fissionThresh) {
	    return (info.type = EventInfo::FISSION_EVENT);
	}
	return (info.type = EventInfo::UNKNOWN_EVENT);
    }

    // low energy events
    if (info.pileUp && !enoughMcp) {
	return (info.type = EventInfo::DECAY_EVENT);
    }
    if (!info.pileUp && !enoughMcp && !info.hasTof) {
	return (info.type = EventInfo::DECAY_EVENT);
    }
    if (info.beamOn && (info.hasTof || enoughMcp) )
	return (info.type = EventInfo::PROJECTILE_EVENT);
    
    return (info.type = EventInfo::UNKNOWN_EVENT);
}

void ImplantSsdProcessor::PlotType(EventInfo &info, int pos, Correlator::EConditions cond)
{
    using namespace dammIds::implantSsd;
    
    const unsigned int numGranularities = 9;
    // time resolution in seconds per bin
    const double timeResolution[numGranularities] = 
	{10e-9, 100e-9, 400e-9, 1e-6, 10e-6, 100e-6, 1e-3, 10e-3, 100e-3};

    static double prevVeto = 0;

    plot(DD_ALL_ENERGY__POSITION, info.energy, pos);

    switch (info.type) {
	case EventInfo::IMPLANT_EVENT:
	    plot(DD_IMPLANT_ENERGY__POSITION, info.energy, pos);
	    break;
	case EventInfo::FISSION_EVENT:
	case EventInfo::ALPHA_EVENT:
	case EventInfo::DECAY_EVENT:
	    plot(DD_DECAY_ENERGY__POSITION, info.energy, pos);
	    if (info.beamOn) {
		plot(DD_ENERGY__POSITION_BEAM, info.energy, pos);
	    } else { 
		plot(DD_ENERGY__POSITION_NOBEAM, info.energy, pos);
	    }
	    if (cond == Correlator::VALID_DECAY) {	 
		for (unsigned int i = 0; i < numGranularities; i++) {
		    int timeBin = int(info.dtime * pixie::clockInSeconds / 
				      timeResolution[i]);
		
		    plot(DD_DECAY_ALL_ENERGY__TX + i, info.energy, timeBin);
		    if (!info.beamOn) {
			plot(DD_DECAY_NOBEAM_ENERGY__TX + i, info.energy, timeBin);
		    }
		    if (info.generation == 1) {
			plot(DD_FIRST_DECAY_ENERGY__TX + i, info.energy, timeBin);
		    }
		}
	    }
	    break;
	case EventInfo::PROJECTILE_EVENT:
	    plot(DD_ENERGY__POSITION_PROJLIKE, info.energy, pos);
	    break;
	case EventInfo::PROTON_EVENT:	    
	    plot(DD_ENERGY__POSITION_VETO, info.energy, pos);
	    for (unsigned int i=0; i < numGranularities; i++) {
		double dt = info.time - prevVeto; // time to previous veto
		int timeBin = int(dt * pixie::clockInSeconds / timeResolution[i]);
		plot(DD_VETO_ENERGY__TX + i, info.energy, timeBin);
	    }
	    prevVeto = info.time;
	    break;
	case EventInfo::UNKNOWN_EVENT:
	default:
	    plot(DD_ENERGY__POSITION_UNKNOWN, info.energy, pos);
	    break;
    }

}
