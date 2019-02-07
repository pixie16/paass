/*! \file DssdProcessor.cpp
 * @authors P. Brionnet , T. T. King
 * @date February 5, 2019
 *
 * Based on the older DssdProcessor and Dssd4SHEProcessor. But this processor
 * will include things specific to the NSHE implemntaion like the Tof and Veto
 * 
 */

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

#include "NsheProcessor.hpp"
#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "Messenger.hpp"
#include "Notebook.hpp"
#include "RawEvent.hpp"
#include "DetectorDriver.hpp"
#include "Correlator.hpp"

using namespace dammIds::dssd4she;
using namespace std;

NsheProcessor::NsheProcessor():EventProcessor(OFFSET, RANGE, "NsheProcessor"){ //, correlator_(numBackStrips, numFrontStrips) {
    associatedTypes.insert("SHE");
}

NsheProcessor::NsheProcessor(double timeWindow, double tofWindow, double vetoWindow,  double deltaEnergy, double highEnergyCut,
 double lowEnergyCut, double zero_suppress, double fissionEnergyCut, double minImpTime,
  double corrTime, double fastTime):EventProcessor(OFFSET, RANGE, "NsheProcessor"){ //, correlator_(numBackStrips, numFrontStrips) {
	timeWindow_ = timeWindow;
    tofWindow_ = tofWindow;
    vetoWindow_ = vetoWindow;
    deltaEnergy_ = deltaEnergy;
    highEnergyCut_ = highEnergyCut;
    lowEnergyCut_ = lowEnergyCut;
    zero_suppress_ = zero_suppress;
    fissionEnergyCut_ = fissionEnergyCut;
    minImpTime_ = minImpTime;
    corrTime_ =  corrTime;
    fastTime_ = fastTime;
    associatedTypes.insert("SHE");
    
/*
    stringstream ss;
    ss << fixed
       << "#T"
       << " " << setw(12) << "E (keV)"
       << " " << setw(12) << "t (ms)"
       << " M" << " "
       << "B" << " "
       << "V" << " "
       << "E" << " "
       << endl;
    Notebook::get()->report(ss.str());
*/
}


void NsheProcessor::DeclarePlots(void) {
    using namespace dammIds::dssd4she;

    const int energyBins = SE;
    const int energyBins2 = SC;
    const int energyBins3 = SA;
    const int xBins = S5;
    const int yBins = S4;

    DeclareHistogram1D(D_ENERGY_X, energyBins, "Energy/10 dssd X strips");
    DeclareHistogram1D(D_ENERGY_Y, energyBins, "Energy/10 dssd Y strips");

    DeclareHistogram1D(D_DTIME, S8, "Pairs time diff in 10 ns (+ 1 bin)");
    DeclareHistogram1D(D_DTIMEE, S8, "Pairs time diff in 10 ns (+ 1 bin) for max");

    DeclareHistogram1D(D_DTIMETOF, SA, "Pairs time diff in 10 ns (+ 1 bin) for toff");

    DeclareHistogram1D(D_MWPC_MULTI, S5, "MWPC multiplicity");
    DeclareHistogram1D(D_ENERGY_CORRELATED_SIDE, energyBins,        "Energy Side corr. with DSSD");
    DeclareHistogram1D(D_DTIME_SIDE, S8,        "Side det. time diff in 10 ns (+ 1 bin)");

    DeclareHistogram2D(DD_ENERGY_DT__DSSD_MWPC,        SB, S8, "DSSD energy/100 vs DT (10 ns) to MWPC");

    DeclareHistogram2D(DD_DE_E__DSSD_VETO,        SB, SB, "DSSD energy/100 vs veto/100");

    DeclareHistogram2D(DD_EVENT_POSITION,        xBins, yBins, "DSSD all events positions");
    DeclareHistogram2D(DD_EVENT_POSITION_FROM_E,        xBins, yBins, "DSSD position all max event");
    DeclareHistogram2D(DD_IMPLANT_POSITION,        xBins, yBins, "DSSD position implant");
    DeclareHistogram2D(DD_DECAY_POSITION,        xBins, yBins, "DSSD position decay");
    DeclareHistogram2D(DD_LIGHT_POSITION,        xBins, yBins, "DSSD position light ion");
    DeclareHistogram2D(DD_UNKNOWN_POSITION,        xBins, yBins, "DSSD position unknown");
    DeclareHistogram2D(DD_FISSION_POSITION,        xBins, yBins, "DSSD position fission");

    DeclareHistogram1D(D_ENERGY_IMPLANT,        energyBins2, "DSSD energy/100 implant");
    DeclareHistogram1D(D_ENERGY_DECAY,        energyBins2, "DSSD energy/100 decay");
    DeclareHistogram1D(D_ENERGY_LIGHT,        energyBins2, "DSSD energy/100 light ion");
    DeclareHistogram1D(D_ENERGY_UNKNOWN,        energyBins2, "DSSD energy/100 unknown");
    DeclareHistogram1D(D_ENERGY_FISSION,        energyBins2, "DSSD energy/100 fission");
    DeclareHistogram1D(D_ENERGY_DECAY_BEAMSTOP,        energyBins, "DSSD energy*1 alpha beam stopped");

    DeclareHistogram2D(DD_EVENT_ENERGY__X_POSITION,        energyBins, xBins, "DSSD X strips E vs. position");
    DeclareHistogram2D(DD_EVENT_ENERGY__Y_POSITION,        energyBins, yBins, "DSSD Y strips E vs. position");
    DeclareHistogram2D(DD_MAXEVENT_ENERGY__X_POSITION,        energyBins, xBins, "MAXDSSD X strips E vs. position");
    DeclareHistogram2D(DD_MAXEVENT_ENERGY__Y_POSITION,        energyBins, yBins, "MAXDSSD Y strips E vs. position");

    DeclareHistogram1D(D_ENERGY_WITH_VETO, energyBins,        "Energy dssd/10 coin. veto");
    DeclareHistogram1D(D_ENERGY_WITH_MWPC, energyBins,        "Energy dssd/10 coin. mwpc");
    DeclareHistogram1D(D_ENERGY_WITH_VETO_MWPC, energyBins,        "Energy dssd/10 coin. veto and mwpc");
    DeclareHistogram1D(D_ENERGY_NO_VETO_MWPC, energyBins,        "Energy dssd/10 coin. no veto and mwpc");

    DeclareHistogram2D(DD_FRONTE__BACKE, SB, SD,        "Front vs Back energy");
    DeclareHistogram2D(DD_ENERGY__POSX_T_MISSING,        energyBins, xBins,        "DSSD T missing X strips E vs. position");
    DeclareHistogram2D(DD_ENERGY__POSY_T_MISSING,        energyBins, yBins,        "DSSD T missing Y strips E vs. position");
    DeclareHistogram2D(DD_MAXEVENT_FRONTE__BACKE, energyBins, energyBins3,        "Front vs Back energy max event");
    /** Check how many strips and how far fired **/
    DeclareHistogram2D(DD_DENERGY__DPOS_X_CORRELATED,        energyBins, xBins, "DSSD dE dX correlated events");
    DeclareHistogram2D(DD_DENERGY__DPOS_Y_CORRELATED,        energyBins, yBins, "DSSD dE dY correlated events");

    DeclareHistogram2D(DD_TOF_ENERGY, SA , energyBins, "DSSD dE dY correlated events");
    DeclareHistogram2D(DD_VETO_ENERGY, SB , SB, "DSSD dE dY correlated events");

    DeclareHistogram2D(DD_KH_PLOT, energyBins , S9, "DSSD E vs log2(Decay Time) (/100)");
    DeclareHistogram2D(DD_ALPHA_ALPHA, SB , SB, "Decay vs Decay Energy");


}


bool NsheProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    if (DetectorDriver::get()->GetSysRootOutput()){
        dssdstruc = processor_struct::DSSD_DEFAULT_STRUCT;
    }
    xyEventsTMatch_.clear(); // part of the NsheProcessor class
    xyEventsEMatch_.clear();
    

    vector<ChanEvent *> xEvents =
            event.GetSummary("SHE:dssd_front", true)->GetList();
    vector<ChanEvent *> yEvents =
            event.GetSummary("SHE:dssd_back", true)->GetList();

    /**
     * Matching the front-back by the time correlations
     */
    vector<pair<StripEvent, bool>> xEventsTMatch;
    vector<pair<StripEvent, bool>> yEventsTMatch;

    for (vector<ChanEvent *>::iterator itx = xEvents.begin();
         itx != xEvents.end();
         ++itx) {
	    if((*itx)->GetCalibratedEnergy() < zero_suppress_) continue;	
        // if(xEvents.size()>1 && itx > xEvents.begin()) cout<<xEvents.at(1)->GetTime() - xEvents.at(0)->GetTime()<<endl;
	    StripEvent ev((*itx)->GetCalibratedEnergy()/0.3, (*itx)->GetTime(), (*itx)->GetChanID().GetLocation(),
        (*itx)->IsSaturated(), (*itx)->GetTrace(), (*itx)->IsPileup());
        pair<StripEvent, bool> match(ev, false);
        xEventsTMatch.push_back(match);
	    //if (mcp1Event.size() != 0 || mcp2Event.size() != 0 )
	    // cout<<mcp1Event.size()<<" "<<mcp2Event.size()<<" "<<vetoEvent.size()<<endl;
        plot(DD_MAXEVENT_ENERGY__X_POSITION, (*itx)->GetCalibratedEnergy()/0.3, (*itx)->GetChanID().GetLocation());

//        /** Handle additional pulses (no. 2, 3, ...) */
//        int pulses = trace.GetValue("numPulses");
//        for (int i = 1; i < pulses; ++i) {
//            stringstream energyCalName;
//            energyCalName << "filterEnergy" << i + 1 << "Cal";
//            stringstream timeName;
//            timeName << "filterTime" << i + 1;
//
//            ev.pileup = true;
//
//            StripEvent ev2;
//            ev2.E = trace.GetValue(energyCalName.str());
//            ev2.t = (trace.GetValue(timeName.str()) -
//                     trace.GetValue("filterTime") + ev.t);
//            ev2.pos = ev.pos;
//            ev2.sat = false;
//            ev2.pileup = true;
//            pair<StripEvent, bool> match2(ev2, false);
//            xEventsTMatch.push_back(match2);
//
//            if (i > 1) {
//                stringstream ss;
//                ss << "DSSD X, " << i + 1 << " pulse"
//                   << ", E = " << ev2.E
//                   << ", dt = " << ev2.t - ev.t;
//                Messenger m;
//                m.run_message(ss.str());
//            }
//        }
//
//        for (vector<ChanEvent *>::iterator itx2 = itx;
//             itx2 != xEvents.end();
//             ++itx2) {
//            int dx = abs(ev.pos -
//                         (*itx2)->GetChanID().GetLocation());
//            double dE = abs(ev.E -
//                            (*itx2)->GetCalibratedEnergy());
//            plot(DD_DENERGY__DPOS_X_CORRELATED, dE, dx);
//        }
    }

    for (vector<ChanEvent *>::iterator ity = yEvents.begin(); ity != yEvents.end(); ++ity) {
        
	    if((*ity)->GetCalibratedEnergy() < zero_suppress_) continue;
	    StripEvent ev((*ity)->GetCalibratedEnergy(), (*ity)->GetTime(), (*ity)->GetChanID().GetLocation(), 
        (*ity)->IsSaturated(),(*ity)->GetTrace(), (*ity)->IsPileup());
        pair<StripEvent, bool> match(ev, false);
        yEventsTMatch.push_back(match);

        plot(DD_MAXEVENT_ENERGY__Y_POSITION, (*ity)->GetCalibratedEnergy(), (*ity)->GetChanID().GetLocation());
//        int pulses = trace.GetValue("numPulses");
//        for (int i = 1; i < pulses; ++i) {
//            stringstream energyCalName;
//            energyCalName << "filterEnergy" << i + 1 << "Cal";
//            stringstream timeName;
//            timeName << "filterTime" << i + 1;
//
//            ev.pileup = true;
//
//            StripEvent ev2;
//            ev2.E = trace.GetValue(energyCalName.str());
//            ev2.t = (trace.GetValue(timeName.str()) -
//                     trace.GetValue("filterTime") + ev.t);
//            ev2.pos = ev.pos;
//            ev2.sat = false;
//            ev2.pileup = true;
//            pair<StripEvent, bool> match2(ev2, false);
//            yEventsTMatch.push_back(match2);
//
//            if (i > 1) {
//                stringstream ss;
//                ss << "DSSD Y, " << i + 1 << " pulse"
//                   << ", E = " << ev2.E
//                   << ", dt = " << ev2.t - ev.t;
//                Messenger m;
//                m.run_message(ss.str());
//            }
//        }
//
//        for (vector<ChanEvent *>::iterator ity2 = ity;
//             ity2 != yEvents.end();
//             ++ity2) {
//            int dy = abs(ev.pos -
//                         (*ity2)->GetChanID().GetLocation());
//            double dE = abs(ev.E -
//                            (*ity2)->GetCalibratedEnergy());
//            plot(DD_DENERGY__DPOS_Y_CORRELATED, dE, dy);
//        }
    }

    for (vector<pair<StripEvent, bool> >
         ::iterator ity = yEventsTMatch.begin();
         ity != yEventsTMatch.end();
         ++ity) {
        double bestDtime = numeric_limits<double>::max();
        vector<pair<StripEvent, bool> >::iterator bestMatch = xEventsTMatch.end();
        for (vector<pair<StripEvent, bool> >::iterator itx = xEventsTMatch.begin();
             itx != xEventsTMatch.end();
             ++itx) {
            // If already matched, skip
            if ((*itx).second) continue;

            double energyX = (*itx).first.E;
            double energyY = (*ity).first.E;

            /** If energies are in lower range and/or not satured
             *  check if delta energy condition is not met, 
             *  if not, skip this event
             *
             *  For high energy events and satured set 20 MeV
             *  energy for difference check. The calibration in this
             *  range is most likely imprecise, so one cannot correlate
             *  by energy difference.
             **/
	    //if ((*ity).first.sat || energyY > highEnergyCut_) energyY = 10000.0;
            //if ((*itx).first.sat || energyX > highEnergyCut_) energyX = 10000.0;
            if ((abs(energyX - energyY) > deltaEnergy_ && !(*itx).first.pileup && !(*ity).first.pileup && !(*itx).first.sat)) 
            //if ((*itx).first.pileup !=(*ity).first.pileup) 
            {
	    	continue;
	    }

            double dTime = abs((*itx).first.t - (*ity).first.t) * Globals::get()->GetClockInSeconds();
            if (dTime < bestDtime) { bestDtime = dTime; bestMatch = itx;
	    //if (bestDtime < timeWindow_) cout<<bestDtime<<endl;
            }
        }
        if (bestDtime < timeWindow_ && (*bestMatch).first.pileup == (*ity).first.pileup) {
            xyEventsTMatch_.push_back( pair<StripEvent, StripEvent>((*bestMatch).first, (*ity).first));
            (*ity).second = true;
            (*bestMatch).second = true;
            //plot(D_DTIME, int(((*bestMatch).first.t - (*ity).first.t) * Globals::get()->GetClockInSeconds()/ 1.0e-8) + 500);
	    plot(D_DTIME, int(bestDtime / 1.0e-8) + 1);
        } else {
            bestDtime = int(bestDtime / 1.0e-8);
            if (bestDtime > S8) bestDtime = S8 - 1;
            else if (bestDtime < 0) {
	    //cout<<bestDtime<<endl;
	    bestDtime = 0;
	    }
            //plot(D_DTIME, bestDtime);
        }
    }

    for (vector<pair<StripEvent, bool> >
         ::iterator itx = xEventsTMatch.begin();
         itx != xEventsTMatch.end();
         ++itx) {
        if ((*itx).second)
            continue;
        int position = (*itx).first.pos;
        double energy = (*itx).first.E;
        plot(DD_ENERGY__POSX_T_MISSING, energy, position);
    }

    for (vector<pair<StripEvent, bool> >
         ::iterator ity = yEventsTMatch.begin();
         ity != yEventsTMatch.end();
         ++ity) {
        if ((*ity).second)
            continue;
        int position = (*ity).first.pos;
        double energy = (*ity).first.E;
        plot(DD_ENERGY__POSY_T_MISSING, energy, position);
    }

    /**
     * Matching the front-back by the Energy of the event
     * Using the old style GetMaxEvent for comparison
     */
    if (xEvents.size() > 0 && yEvents.size() > 0) {
        ChanEvent *maxFront = event.GetSummary("SHE:dssd_front")->GetMaxEvent(true);
        ChanEvent *maxBack = event.GetSummary("SHE:dssd_back")->GetMaxEvent(true);
        StripEvent evf(maxFront->GetCalibratedEnergy()/0.3, maxFront->GetTime(), maxFront->GetChanID().GetLocation(),
        maxFront->IsSaturated(), maxFront->GetTrace(), maxFront->IsPileup());
        StripEvent evb(maxBack->GetCalibratedEnergy(), maxBack->GetTime(), maxBack->GetChanID().GetLocation(),
        maxBack->IsSaturated(),maxBack->GetTrace(), maxBack->IsPileup());
        xyEventsEMatch_.push_back(pair<StripEvent, StripEvent>(evf, evb));
	plot(D_DTIMEE, int((abs(maxFront->GetTime() - maxBack->GetTime()) * Globals::get()->GetClockInSeconds() / 1.0e-8) + 1));
    }

    return true;
}


bool NsheProcessor::Process(RawEvent &event) {
    using namespace dammIds::dssd4she;

    if (!EventProcessor::Process(event))
        return false;

//    vector<ChanEvent *> vetoEvents =
//            event.GetSummary("si:veto", true)->GetList();
//    vector<ChanEvent *> sideEvents =
//            event.GetSummary("si:si", true)->GetList();
//    vector<ChanEvent *> mwpcEvents =
//            event.GetSummary("si:si", true)->GetList();
//    int mwpc = event.GetSummary("mcp", true)->GetMult();
//
//    bool hasBeam = TreeCorrelator::get()->place("Beam")->status();
//
//    plot(D_MWPC_MULTI, mwpc);

    static Correlator corr(minImpTime_,corrTime_,fastTime_);

    vector<ChanEvent *> mcp1Event = event.GetSummary("SHE:mcp1", true)->GetList();
    vector<ChanEvent *> mcp2Event = event.GetSummary("SHE:mcp2", true)->GetList();
	vector<ChanEvent *> vetoEvent = event.GetSummary("SHE:veto", true)->GetList();
  
    EventInfo corEvent;

//	cout<<mcp1Event.size()<<endl;
    for (vector<pair<StripEvent, StripEvent> >::iterator it =
            xyEventsTMatch_.begin();
         it != xyEventsTMatch_.end();
         ++it) {

        double xEnergy = (*it).first.E;
        double yEnergy = (*it).second.E;

	if ((*it).first.sat && !(*it).second.sat) xEnergy = yEnergy;
	
        /** If saturated set to 200 MeV **/
        if ((*it).first.sat && (*it).second.sat) {
            xEnergy = 10000.0;
            yEnergy = 10000.0;
        }


    int xPosition = (*it).first.pos;
    int yPosition = (*it).second.pos;
	double time = min((*it).first.t, (*it).second.t);

	double bestDtime = numeric_limits<double>::max();
	auto bestMatch = mcp1Event.end();	
	double bestDtime1 = numeric_limits<double>::max();
	auto bestMatch1 = mcp2Event.end();	
	double bestDtime_veto = numeric_limits<double>::max();
	auto bestMatch_veto = vetoEvent.end();

	double tof = 0;

    /** Correlation with MCP1, MPC2 and Veto detector **/
	for (vector<ChanEvent *>::iterator itm = mcp1Event.begin(); itm != mcp1Event.end(); itm++)
	{
	    double dTime = abs(time - (*itm)->GetTime()) * Globals::get()->GetClockInSeconds();
//            cout<<(*itm)->GetCfdFractionalTime() * Globals::get()->GetClockInSeconds()<<endl;  
	  if (dTime < bestDtime) { bestDtime = dTime; bestMatch = itm;
	    //if (bestDtime < timeWindow_) cout<<bestDtime<<endl;
            }
	}
	for (vector<ChanEvent *>::iterator itm = mcp2Event.begin(); itm != mcp2Event.end(); itm++)
	{
	    double dTime = abs(time - (*itm)->GetTime()) * Globals::get()->GetClockInSeconds();
//           cout<<(*itm)->GetCfdFractionalTime() * Globals::get()->GetClockInSeconds()<<endl;  
	  if (dTime < bestDtime1) { bestDtime1 = dTime; bestMatch1 = itm;
	    //if (bestDtime < timeWindow_) cout<<bestDtime<<endl;
            }
	}
    for (vector<ChanEvent *>::iterator itv = vetoEvent.begin(); itv != vetoEvent.end(); itv++)
	{
	    double dTime = abs(time - (*itv)->GetTime()) * Globals::get()->GetClockInSeconds();
//            cout<<(*itm)->GetCfdFractionalTime() * Globals::get()->GetClockInSeconds()<<endl;  
	  if (dTime < bestDtime_veto) { bestDtime_veto = dTime; bestMatch_veto = itv;
	    //if (bestDtime < timeWindow_) cout<<bestDtime<<endl;
            }
	}


	//cout<<bestDtime<<endl;
    if (bestDtime < tofWindow_ &&  bestDtime1 < tofWindow_)
	{
		tof = (((*bestMatch1)->GetTime() - (*bestMatch)->GetTime()) * Globals::get()->GetClockInSeconds())/(1e-9);
        corEvent.type = EventInfo::IMPLANT_EVENT;
        corEvent.energy = yEnergy;
	}	 	
	else if(bestDtime < tofWindow_)
	{
		tof = bestDtime;
        corEvent.type = EventInfo::IMPLANT_EVENT;
        corEvent.energy = yEnergy;
	} 
       	else if(bestDtime1 < tofWindow_)
	{
		tof = bestDtime1;
        corEvent.type = EventInfo::IMPLANT_EVENT;
        corEvent.energy = yEnergy;
	}
    else 
    {
        tof = 0;
        corEvent.type = EventInfo::DECAY_EVENT;
        corEvent.energy = xEnergy;
    }
    if (bestDtime_veto < vetoWindow_)
	{
		plot(DD_VETO_ENERGY, (*bestMatch_veto)->GetCalibratedEnergy(),yEnergy/3);
	}

    corEvent.time = time;

    corr.Correlate(corEvent, xPosition, yPosition);
    if (corr.GetCondition() == Correlator::VALID_DECAY) {
        double Tlog = log2((corr.GetDecayGenTime(xPosition, yPosition) * Globals::get()->GetClockInSeconds())/(1e-9))*10;
    	plot(DD_KH_PLOT, xEnergy, Tlog);
        if(corr.GetGen(xPosition, yPosition) > 1 ) 
        plot(DD_ALPHA_ALPHA, xEnergy/5, corr.GetPreviousDecayEnergy(xPosition, yPosition)/5);
    }
	plot(D_DTIMETOF, tof);
	plot(DD_TOF_ENERGY, tof,yEnergy);

		if (DetectorDriver::get()->GetSysRootOutput()){
			//Fill Root struct
			dssdstruc.x = xPosition;
			dssdstruc.y = yPosition;
			dssdstruc.Energy_front = xEnergy;
			dssdstruc.Energy_back = yEnergy;
			dssdstruc.Time = time;
			dssdstruc.Trace_Front = (*it).first.trace;
			dssdstruc.Trace_Back = (*it).second.trace;
			dssdstruc.pile_up = (*it).first.pileup; 
			pixie_tree_event_->dssd_vec_.emplace_back(dssdstruc);
			dssdstruc = processor_struct::DSSD_DEFAULT_STRUCT;
		}

        plot(D_ENERGY_X, xEnergy);
        plot(D_ENERGY_Y, yEnergy);

        plot(DD_FRONTE__BACKE, xEnergy/5, yEnergy/5);
        plot(DD_EVENT_ENERGY__X_POSITION, xEnergy, xPosition);
        plot(DD_EVENT_ENERGY__Y_POSITION, yEnergy, yPosition);

        plot(DD_EVENT_POSITION, xPosition, yPosition);
		/*
        double mwpcTime = numeric_limits<double>::max();
        for (vector<ChanEvent *>::iterator itm = mwpcEvents.begin();
             itm != mwpcEvents.end();
             ++itm) {
            double dt = abs(time - (*itm)->GetTime()) * Globals::get()->clockInSeconds();
            if (dt < mwpcTime) { mwpcTime = dt;
            }
        }
        // Plot up to 3 us only
        if (mwpcTime < 3.0e-6) {
            int timeBin = int(mwpcTime / 1.0e-8);
            int energyBin = xEnergy / 100.0;
            plot(DD_ENERGY_DT__DSSD_MWPC, energyBin, timeBin);
        }

        if (vetoEvents.size() > 0) {
            for (vector<ChanEvent *>::iterator itv = vetoEvents.begin(); itv != vetoEvents.end(); ++itv) {
                double vetoEnergy = (*itv)->GetCalibratedEnergy(); plot(DD_DE_E__DSSD_VETO,
                (vetoEnergy + xEnergy) / 100.0, xEnergy / 100.0);
            }
        }

        double bestSiTime = numeric_limits<double>::max();
        ChanEvent *correlatedSide = 0;
        bool hasEscape = false;
        double escapeEnergy = 0.0;
        for (vector<ChanEvent *>::iterator its = sideEvents.begin();
             its != sideEvents.end();
             ++its) {
            double dt = abs(time - (*its)->GetTime()) * Globals::get()->clockInSeconds();
            if (dt < bestSiTime) { bestSiTime = dt; correlatedSide = *its;
            }
        }

        if (correlatedSide != 0) {
            int siTime = int(bestSiTime / 1.0e-8) + 1;
            if (siTime > S8) siTime = S8 - 1;
            else if (siTime < 0) siTime = 0;
            plot(D_DTIME_SIDE, siTime);
            if (bestSiTime < timeWindow_) { plot(D_ENERGY_CORRELATED_SIDE, correlatedSide->GetCalibratedEnergy());
            hasEscape = true; escapeEnergy = correlatedSide->GetCalibratedEnergy();
            }
        }

        bool hasVeto = false;
        if (vetoEvents.size() > 0)
            hasVeto = true;

        if (hasVeto)
            plot(D_ENERGY_WITH_VETO, xEnergy / 10.0);
        if (mwpc > 0)
            plot(D_ENERGY_WITH_MWPC, xEnergy / 10.0);
        if (hasVeto && mwpc > 0)
            plot(D_ENERGY_WITH_VETO_MWPC, xEnergy / 10.0);
        if (!hasVeto && mwpc == 0)
            plot(D_ENERGY_NO_VETO_MWPC, xEnergy / 10.0);

        SheEvent event = SheEvent(xEnergy + escapeEnergy, time, mwpc, hasBeam, hasVeto, hasEscape, unknown);
        pickEventType(event);

        if (!event.get_beam())
            plot(D_ENERGY_DECAY_BEAMSTOP, event.get_energy());

        if (event.get_type() == heavyIon) {
            plot(DD_IMPLANT_POSITION, xPosition, yPosition);
            plot(D_ENERGY_IMPLANT, event.get_energy());
        } else if (event.get_type() == alpha) {
            plot(DD_DECAY_POSITION, xPosition, yPosition);
            plot(D_ENERGY_DECAY, event.get_energy() / 100.0);
        } else if (event.get_type() == lightIon) {
            plot(DD_LIGHT_POSITION, xPosition, yPosition);
            plot(D_ENERGY_LIGHT, event.get_energy() / 100.0);
        } else if (event.get_type() == unknown) {
            plot(DD_UNKNOWN_POSITION, xPosition, yPosition);
            plot(D_ENERGY_UNKNOWN, event.get_energy() / 100.0);
        } else if (event.get_type() == fission) {
            plot(DD_FISSION_POSITION, xPosition, yPosition);
            plot(D_ENERGY_FISSION, event.get_energy() / 100.0);
        }

        correlator_.add_event(event, xPosition, yPosition);
*/
    }

    /** Old style max event for comparison */
    for (vector<pair<StripEvent, StripEvent> >::iterator it =
            xyEventsEMatch_.begin();
         it != xyEventsEMatch_.end();
         ++it) {
        double xEnergy = (*it).first.E;
        double yEnergy = (*it).second.E;

        int xPosition = (*it).first.pos;
        int yPosition = (*it).second.pos;

        plot(DD_EVENT_POSITION_FROM_E, xPosition, yPosition);

	plot(DD_MAXEVENT_FRONTE__BACKE, xEnergy/5, yEnergy/5);
    }

    EndProcess();
    return true;
}


//bool NsheProcessor::pickEventType(SheEvent &event) {
//    /**
//     * Logic table (V - veto, M - mwpc, B - beam )
//     * Logic state is converted into a numerical value N
//     * like a binary number:
//     *
//     * V M B | N | decision
//     * --------------------
//     * 0 0 0 | 0 | unknown / alpha / fission (depending on energy)
//     * 0 0 1 | 1 | -"-
//     * 0 1 0 | 2 | unknown
//     * 0 1 1 | 3 | heavyIon
//     * 1 0 0 | 4 | unknown
//     * 1 0 1 | 5 | lightIon
//     * 1 1 0 | 6 | unknown
//     * 1 1 1 | 7 | lightIon
//     *
//     **/
//
//    int condition = 0;
//    if (event.get_beam())
//        condition += 1;
//    if (event.get_mwpc() > 0)
//        condition += 2;
//    if (event.get_veto())
//        condition += 4;
//
//    if (condition == 0) {
//        double energy = event.get_energy();
//        if (energy < highEnergyCut_)
//            event.set_type(alpha);
//        else if (energy < fissionEnergyCut_)
//            event.set_type(unknown);
//        else
//            event.set_type(fission);
//    } else if (condition == 1) {
//        double energy = event.get_energy();
//        if (energy < lowEnergyCut_)
//            event.set_type(unknown);
//        else if (energy < highEnergyCut_)
//            event.set_type(alpha);
//        else if (energy < fissionEnergyCut_)
//            event.set_type(unknown);
//        else
//            event.set_type(fission);
//    } else if (condition == 2 ||
//               condition == 4 ||
//               condition == 6) {
//        event.set_type(unknown);
//    } else if (condition == 3) {
//        event.set_type(heavyIon);
//    } else if (condition == 5 || condition == 7) {
//        event.set_type(lightIon);
//    } else
//        event.set_type(unknown);
//
//    return true;
//}


