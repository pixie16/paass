/*! \file ImplantSsdProcessor.cpp
 * \brief Handles ssd:implant detectors
 *
 * The implantSSD processor handles detectors of type ssd:implant and
 *   determines whether the events are implants or decays and informs the
 *   correlator accordingly
 */
#include <cfloat>
#include <climits>
#include <iostream>
#include <sstream>
#include <vector>

#include "DammPlotIds.hpp"

#include "DetectorDriver.hpp"
#include "ImplantSsdProcessor.hpp"
#include "TriggerLogicProcessor.hpp"
#include "RawEvent.hpp"

using std::cout;
using std::endl;
using std::min;
using std::stringstream;
using std::vector;

/*! ecutoff for 108Xe experiment where each bin is roughly 4 keV
 *  ... implants deposit above 18 MeV
 */
const double ImplantSsdProcessor::cutoffEnergy  = 1500;
const double ImplantSsdProcessor::fissionThresh = 4400;//!< fission threshold
const double ImplantSsdProcessor::goodAlphaCut  = 950;//!< good alpha cut
const double ImplantSsdProcessor::implantTof    = 2800;//!< implant time of flight

using namespace dammIds::implantSsd;

namespace dammIds {
    namespace implantSsd {
        const int MAX_TOF = 5;//!< maximum tof

        const int DD_IMPLANT_ENERGY__LOCATION  = 0;//!< Implant Energy vs. Loc
        const int DD_DECAY_ENERGY__LOCATION    = 1;//!< SSD Strip vs. Decay E
        const int DD_ENERGY__LOCATION_BEAM     = 2;//!< SSD Strip vs. E w/ beam
        const int DD_ENERGY__LOCATION_NOBEAM   = 3;//!< SSD Strip vs. E no beam
        const int DD_ENERGY__LOCATION_VETO     = 4;//!< SSD Strip vs. E Veto
        const int DD_ENERGY__LOCATION_PROJLIKE = 5;//!< SSD Strip vs. E Projectile Like
        const int DD_ENERGY__LOCATION_UNKNOWN  = 6;//!< SSD Strip vs. E (unknown)
        const int DD_LOC_VETO__LOC_SSD         = 7;//!< Veto pos vs. SSD Pos
        const int DD_TOTENERGY__ENERGY         = 8;//!< Tot Energy vs. SSD Energy (/8)
        const int DD_ALL_ENERGY__LOCATION      = 9;//!< All energy vs. location
        const int DD_FISSION_ENERGY__LOCATION  = 10;//!< Fission energy vs. location

        const int DD_DECAY_ALL_ENERGY__TX     = 20;//!< SSD Ty,Ex (various times)
        const int DD_DECAY_NOBEAM_ENERGY__TX  = 30;//!< SSD Ty,Ex no beam
        const int DD_VETO_ENERGY__TX          = 40;//!< SSD Veto vs Energy
        const int DD_FIRST_DECAY_ENERGY__TX   = 50;//!< SSD First decay vs. Energy

        const int DD_ALL_ENERGY__TOFX         = 60;//!< SSD All energy vs. ToF
        const int DD_IMPLANT_ENERGY__TOFX     = 70;//!< Implant energy vs. ToF
        const int DD_VETO_ENERGY__TOFX        = 80;//!< Veto Energy vs. ToF
        const int DD_ALL_ENERGY__TOFX_GATED   = 90;//!< All Energy vs. ToF - gated

        const int D_TDIFF_FOIL_IMPLANT = 11;//!< Tdiff between Foil and Implant
        const int D_TDIFF_FOIL_IMPLANT_MULT1 = 12;//!< Tdiff between Foil and Implant - Multiplicity 1

        const int D_FAST_DECAY_TRACE  = 100;//!< Fast decay traces
        const int D_HIGH_ENERGY_TRACE = 200;//!< High energy traces
    }
}

ImplantSsdProcessor::ImplantSsdProcessor() : 
    EventProcessor(OFFSET, RANGE, "ImplantSsdProcessor") {
    associatedTypes.insert("ssd");
}

void ImplantSsdProcessor::DeclarePlots(void)
{
    using namespace dammIds::implantSsd;

    const int unknownEnergyBins = SE;
    const int implantEnergyBins = SD;
    const int decayEnergyBins   = SA;
    const int fissionEnergyBins = SC;
    const int locationBins      = S5;
    const int vetoLocationBins  = S3;
    const int timeBins          = S8;
    const int tofBins           = SD; //! DTM -- LARGE FOR NOW
    const int traceBins         = SC;
    const int tdiffBins         = SA;

    DeclareHistogram2D(DD_ALL_ENERGY__LOCATION,
		       implantEnergyBins, locationBins, "SSD Strip vs Implant E");
    DeclareHistogram2D(DD_IMPLANT_ENERGY__LOCATION,
		       implantEnergyBins, locationBins, "SSD Strip vs Implant E");
    DeclareHistogram2D(DD_DECAY_ENERGY__LOCATION,
		       decayEnergyBins, locationBins, "SSD Strip vs Decay E");
    DeclareHistogram2D(DD_FISSION_ENERGY__LOCATION,
		       fissionEnergyBins, locationBins, "SSD Strip vs Fission E");
    DeclareHistogram2D(DD_ENERGY__LOCATION_BEAM,
		       unknownEnergyBins, locationBins, "SSD Strip vs E w/ beam");
    DeclareHistogram2D(DD_ENERGY__LOCATION_NOBEAM,
		       unknownEnergyBins, locationBins, "SSD Strip vs E w/ no beam");
    DeclareHistogram2D(DD_ENERGY__LOCATION_VETO, implantEnergyBins, locationBins, "SSD Strip vs E w/ veto"); DeclareHistogram2D(DD_ENERGY__LOCATION_PROJLIKE,
		       implantEnergyBins, locationBins, "SSD Strip vs E projectile");
    DeclareHistogram2D(DD_ENERGY__LOCATION_UNKNOWN,
		       unknownEnergyBins, locationBins, "SSD Strip vs E (unknown)");

    DeclareHistogram2D(DD_LOC_VETO__LOC_SSD, vetoLocationBins, locationBins,
		       "Veto pos vs SSD pos");
    histo.DeclareHistogram2D(DD_TOTENERGY__ENERGY, implantEnergyBins, implantEnergyBins,
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
      histo.DeclareHistogram2D(DD_ALL_ENERGY__TOFX+i,
			 implantEnergyBins, tofBins, "SSD Energy vs TOF (/16)", 1, S4, S4);
      histo.DeclareHistogram2D(DD_ALL_ENERGY__TOFX+i,
			 implantEnergyBins, tofBins, "Implant Energy vs TOF (/16)", 1, S4, S4);
      histo.DeclareHistogram2D(DD_VETO_ENERGY__TOFX+i,
			 implantEnergyBins, tofBins, "Veto Energy vs TOF (/16)", 1, S4, S4);
      histo.DeclareHistogram2D(DD_ALL_ENERGY__TOFX_GATED+i,
			 implantEnergyBins, tofBins, "SSD Energy vs TOF (/16), gated", 1, S4, S4);
    }

    DeclareHistogram1D(D_TDIFF_FOIL_IMPLANT, tdiffBins, "DT foil to implant");
    DeclareHistogram1D(D_TDIFF_FOIL_IMPLANT_MULT1, tdiffBins, "DT foil to implant, mult. gated");

    for (unsigned int i=0; i < numTraces; i++) {
	DeclareHistogram1D(D_FAST_DECAY_TRACE + i, traceBins, "fast decay trace");
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
    static DetectorSummary *impSummary  = event.GetSummary("ssd:sum", true);
    static const DetectorSummary *mcpSummary  = event.GetSummary("logic:mcp", true);
    static const DetectorSummary *vetoSummary = event.GetSummary("ssd:veto", true);
    static const DetectorSummary *boxSummary  = event.GetSummary("ssd:box", true);
    DetectorDriver* driver = DetectorDriver::get();

    if (impSummary->GetMult() == 0) {
      EndProcess();
      return false;
    }
    if (firstTime) {
      vector<EventProcessor *> vecProc = driver->GetProcessors("logic");
      for (vector< EventProcessor * >::iterator it = vecProc.begin(); it != vecProc.end(); it++) {
	if ( (*it)->GetName() == "triggerlogic" || (*it)->GetName() == "logic" ) {
	  logProc = reinterpret_cast < LogicProcessor * >(*it);
  	  cout << "Implant SSD processor grabbed logic processor" << endl;
        }
      }
      firstTime=false;
    }

    EventInfo info;
    ChanEvent *ch  = impSummary->GetMaxEvent(true);
    info.hasVeto = ( vetoSummary && vetoSummary->GetMult() > 0 );

    int location = ch->GetChanID().GetLocation();
    if (ch->IsSaturated()) {
	info.energy = 16000; // arbitrary large number
    } else {
	info.energy  = ch->GetCalEnergy();
    }
    if (ch->GetTrace().HasValue("position")) {
	info.position = ch->GetTrace().GetValue("position");
    } // else it defaults to nan

    info.time    = ch->GetTime();
    info.beamOn  = true;

    // recect noise events
    if (info.energy < 10 || ch->GetTrace().HasValue("badqdc")) {
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

    Trace &trace = ch->GetTrace();
    if (trace.HasValue("filterEnergy2")) {
	info.pileUp = true;
    }

    SetType(info);
    Correlate(corr, info, location);

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

        plot(DD_LOC_VETO__LOC_SSD, posVeto, location);
        plot(DD_TOTENERGY__ENERGY, vetoEnergy + info.energy, info.energy);
    }

    if (info.pileUp) {
        double trigTime = info.time;

        info.energy = driver->cali.GetCalEnergy(ch->GetChanID(),
                                              trace.GetValue("filterEnergy2"));
        info.time = trigTime + trace.GetValue("filterTime2") - trace.GetValue("filterTime");

        SetType(info);
        Correlate(corr, info, location);

        int numPulses = trace.GetValue("numPulses");

        if ( numPulses > 2 ) {
            corr.Flag(location, 1);
            cout << "Flagging triple event" << endl;
            for (int i=3; i <= numPulses; i++) {
            stringstream str;
            str << "filterEnergy" << i;
            info.energy = driver->cali.GetCalEnergy(ch->GetChanID(),
                                              trace.GetValue(str.str()));
            str.str(""); // clear it
            str << "filterTime" << i;
            info.time   = trigTime + trace.GetValue(str.str()) - trace.GetValue("filterTime");

            SetType(info);
            Correlate(corr, info, location);
            }
        }
        // corr.Flag(location, 1);
#ifdef VERBOSE
        cout << "Flagging for pileup" << endl;

        cout << "fast trace " << fastTracesWritten << " in strip " << location
            << " : " << trace.GetValue("filterEnergy") << " " << trace.GetValue("filterTime")
            << " , " << trace.GetValue("filterEnergy2") << " " << trace.GetValue("filterTime2") << endl;
        cout << "  mcp mult " << info.mcpMult << endl;
#endif // VERBOSE

        if (fastTracesWritten < numTraces) {
            trace.Plot(D_FAST_DECAY_TRACE + fastTracesWritten);
            fastTracesWritten++;
        }
    }

    if (info.energy > 10000 && !ch->IsSaturated() && !isnan(info.position) ) {
	corr.Flag(location, info.position);
    }

    if (info.energy > 8000 && !trace.empty()) {
#ifdef VERBOSE
	cout << "high energy decay of " << info.energy
	     << "(raw energy " << ch->GetEnergy() << ") with beam "
	     << (info.beamOn ? "present" : "absent") << endl;

	cout << "Flagging for high energy " << info.energy << " with trace" << endl;
#endif //VERBOSE
	// corr.Flag(location, 1);

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

void ImplantSsdProcessor::PlotType(EventInfo &info, int loc, Correlator::EConditions cond)
{
    using namespace dammIds::implantSsd;

    const unsigned int numGranularities = 9;
    // time resolution in seconds per bin
    const double timeResolution[numGranularities] =
	{10e-9, 100e-9, 400e-9, 1e-6, 10e-6, 100e-6, 1e-3, 10e-3, 100e-3};

    double clockInSeconds = Globals::get()->clockInSeconds();

    static double prevVeto = 0;

    plot(DD_ALL_ENERGY__LOCATION, info.energy, loc);

    switch (info.type) {
	case EventInfo::IMPLANT_EVENT:
	    plot(DD_IMPLANT_ENERGY__LOCATION, info.energy, loc);
	    break;
	case EventInfo::FISSION_EVENT:
	    plot(DD_FISSION_ENERGY__LOCATION, info.energy / 10, loc);
	case EventInfo::ALPHA_EVENT:
	case EventInfo::DECAY_EVENT:
	    plot(DD_DECAY_ENERGY__LOCATION, info.energy, loc);
	    if (info.beamOn) {
		plot(DD_ENERGY__LOCATION_BEAM, info.energy, loc);
	    } else {
		plot(DD_ENERGY__LOCATION_NOBEAM, info.energy, loc);
	    }
	    if (cond == Correlator::VALID_DECAY) {
		for (unsigned int i = 0; i < numGranularities; i++) {
		    int timeBin = int(info.dtime * clockInSeconds /
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
	    plot(DD_ENERGY__LOCATION_PROJLIKE, info.energy, loc);
	    break;
	case EventInfo::PROTON_EVENT:
	    plot(DD_ENERGY__LOCATION_VETO, info.energy, loc);
	    for (unsigned int i=0; i < numGranularities; i++) {
		double dt = info.time - prevVeto; // time to previous veto
		int timeBin = int(dt * clockInSeconds / timeResolution[i]);
		plot(DD_VETO_ENERGY__TX + i, info.energy, timeBin);
	    }
	    prevVeto = info.time;
	    break;
	case EventInfo::UNKNOWN_EVENT:
	default:
	    plot(DD_ENERGY__LOCATION_UNKNOWN, info.energy, loc);
	    break;
    }

}

/**
 *  Handle the correlation whether or not we have position information
 */
void ImplantSsdProcessor::Correlate(Correlator &corr, EventInfo &info, int location)
{
    if ( isnan(info.position) ) {
	corr.CorrelateAllY(info, location);
    } else {
	corr.Correlate(info, location, int(info.position));
    }
    // if we want to neglect position information using a dummy variable,
    //   uncomment this line, and comment out the above
    // corr.Correlate(info, location, 1)
    PlotType(info, location, corr.GetCondition());
}
