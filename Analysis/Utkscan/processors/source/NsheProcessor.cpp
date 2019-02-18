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
}


void NsheProcessor::DeclarePlots(void) {
    using namespace dammIds::dssd4she;

    const int energyBins = SE;
    const int xBins = S5;
    const int yBins = S4;

    /** 1D graph Timming and energy **/
    DeclareHistogram1D(D_ENERGY_X, energyBins, "Energy/10 dssd X strips");
    DeclareHistogram1D(D_ENERGY_Y, energyBins, "Energy/10 dssd Y strips");
    DeclareHistogram1D(D_DTIME, S8, "Pairs time diff in 10 ns (+ 1 bin)");
    DeclareHistogram1D(D_DTIMETOF, SA, "Pairs time diff in 10 ns (+ 1 bin) for toff");

    /** 2D graph for FB correlation **/
    DeclareHistogram2D(DD_EVENT_POSITION, xBins, yBins, "DSSD all events positions");
    DeclareHistogram2D(DD_EVENT_ENERGY__X_POSITION, energyBins, xBins, "DSSD X strips E vs. position");
    DeclareHistogram2D(DD_EVENT_ENERGY__Y_POSITION, energyBins, yBins, "DSSD Y strips E vs. position");
    DeclareHistogram2D(DD_FRONTE__BACKE, SB, SD, "Front vs Back energy");
    DeclareHistogram2D(DD_ENERGY__POSX_T_MISSING, energyBins, xBins, "DSSD T missing X strips E vs. position");
    DeclareHistogram2D(DD_ENERGY__POSY_T_MISSING, energyBins, yBins, "DSSD T missing Y strips E vs. position");
    DeclareHistogram2D(DD_TOF_ENERGY, SA , energyBins, "DSSD dE dY correlated events");
    DeclareHistogram2D(DD_VETO_ENERGY, SB , SB, "DSSD dE dY correlated events");

    /** Control spectrum from the correlator **/
    DeclareHistogram2D(DD_KHS, energyBins , S9, "DSSD E vs log2(Decay Time) (/100)");
    DeclareHistogram2D(DD_ALPHA_ALPHA, SB , SB, "Decay vs Decay Energy");
    DeclareHistogram2D(DD_ALPHA_IMPLANT, SB , SB, " Decay Energy vs Implant Energy for first gen");
    DeclareHistogram2D(DD_KHS_GATE, energyBins , S9, "DSSD E vs log2(Decay Time) (/100) for short decay");
    DeclareHistogram2D(DD_ALPHA_ALPHA_GATE, SB , SB, "Decay vs Decay Energy for short decay");
    DeclareHistogram1D(D_GEN, S4, "number of max generation");
}

bool NsheProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    if (DetectorDriver::get()->GetSysRootOutput())
        dssdstruc = processor_struct::DSSD_DEFAULT_STRUCT;

    xyEventsTMatch_.clear(); // part of the NsheProcessor class
    xyEventsEMatch_.clear();    

    vector<ChanEvent *> xEvents = event.GetSummary("SHE:dssd_front", true)->GetList();
    vector<ChanEvent *> yEvents = event.GetSummary("SHE:dssd_back", true)->GetList();

    /**
     * Matching the front-back by the time correlations
     */
    vector<pair<StripEvent, bool>> xEventsTMatch;
    vector<pair<StripEvent, bool>> yEventsTMatch;

    for (vector<ChanEvent *>::iterator itx = xEvents.begin();
        itx != xEvents.end(); ++itx) {
	    if((*itx)->GetCalibratedEnergy() < zero_suppress_ && !(*itx)->IsPileup()) 
            continue;	

	    StripEvent ev((*itx)->GetCalibratedEnergy()/0.3, (*itx)->GetTime(), (*itx)->GetChanID().GetLocation(),
        (*itx)->IsSaturated(), (*itx)->GetTrace(), (*itx)->IsPileup());
        
        pair<StripEvent, bool> match(ev, false);
        xEventsTMatch.push_back(match);
        
        plot(DD_MAXEVENT_ENERGY__X_POSITION, (*itx)->GetCalibratedEnergy()/0.3, (*itx)->GetChanID().GetLocation());
    }

    for (vector<ChanEvent *>::iterator ity = yEvents.begin(); ity != yEvents.end(); ++ity) {
	    if((*ity)->GetCalibratedEnergy() < zero_suppress_ && !(*ity)->IsPileup()) 
            continue;

	    StripEvent ev((*ity)->GetCalibratedEnergy(), (*ity)->GetTime(), (*ity)->GetChanID().GetLocation(), 
        (*ity)->IsSaturated(),(*ity)->GetTrace(), (*ity)->IsPileup());

        pair<StripEvent, bool> match(ev, false);
        yEventsTMatch.push_back(match);

        plot(DD_MAXEVENT_ENERGY__Y_POSITION, (*ity)->GetCalibratedEnergy(), (*ity)->GetChanID().GetLocation());
    }

    for (vector<pair<StripEvent, bool>>::iterator ity = yEventsTMatch.begin();
        ity != yEventsTMatch.end(); ++ity) {

        double bestDtime = numeric_limits<double>::max();
        vector<pair<StripEvent, bool> >::iterator bestMatch = xEventsTMatch.end();

        for (vector<pair<StripEvent, bool> >::iterator itx = xEventsTMatch.begin();
            itx != xEventsTMatch.end(); ++itx) {
            // If already matched, skip
            if ((*itx).second) continue;

            double energyX = (*itx).first.E;
            double energyY = (*ity).first.E;
            /** If energies are in lower range and not satured
             *  check if delta energy condition is not met, 
             *  if not, skip this event
             **/

            if ((abs(energyX - energyY) > deltaEnergy_ && !(*itx).first.pileup 
            && !(*ity).first.pileup && !(*itx).first.sat)) 
	    	    continue;
            double dTime = abs((*itx).first.t - (*ity).first.t) * Globals::get()->GetAdcClockInSeconds();
            if (dTime < bestDtime)
                bestDtime = dTime; bestMatch = itx;
        }
        if (bestDtime < timeWindow_ && (*bestMatch).first.pileup == (*ity).first.pileup) {
            xyEventsTMatch_.push_back(pair<StripEvent, StripEvent>((*bestMatch).first, (*ity).first));
            (*ity).second = true;
            (*bestMatch).second = true;
    	    plot(D_DTIME, int(bestDtime / 1.0e-8) + 1);
        } else {
            bestDtime = int(bestDtime / 1.0e-8);
            if (bestDtime > S8) 
                bestDtime = S8 - 1;
            else if (bestDtime < 0) 
	            bestDtime = 0;
        }
    }
    /**  Checking on the none paired event **/
    for (vector<pair<StripEvent, bool> >::iterator itx = xEventsTMatch.begin();
        itx != xEventsTMatch.end(); ++itx) {
        if ((*itx).second)
            continue;
        int position = (*itx).first.pos;
        double energy = (*itx).first.E;
        plot(DD_ENERGY__POSX_T_MISSING, energy, position);
    }
    for (vector<pair<StripEvent, bool> > ::iterator ity = yEventsTMatch.begin();
         ity != yEventsTMatch.end(); ++ity) {
        if ((*ity).second)
            continue;
        int position = (*ity).first.pos;
        double energy = (*ity).first.E;

        plot(DD_ENERGY__POSY_T_MISSING, energy, position);
    }
    return true;
}

bool NsheProcessor::Process(RawEvent &event) {
    using namespace dammIds::dssd4she;

    if (!EventProcessor::Process(event))
        return false;

    static Correlator corr(minImpTime_,corrTime_,fastTime_);
    // static Correlator corr_gate(minImpTime_,corrTime_,fastTime_);
    vector<ChanEvent *> mcp1Event = event.GetSummary("SHE:mcp1", true)->GetList();
    vector<ChanEvent *> mcp2Event = event.GetSummary("SHE:mcp2", true)->GetList();
	vector<ChanEvent *> vetoEvent = event.GetSummary("SHE:veto", true)->GetList();
    EventInfo corEvent;

    for (vector<pair<StripEvent, StripEvent> >::iterator it = xyEventsTMatch_.begin();
        it != xyEventsTMatch_.end(); ++it) {
        double xEnergy = (*it).first.E;
        double yEnergy = (*it).second.E;

	    if ((*it).first.sat && !(*it).second.sat) 
            xEnergy = yEnergy;
        /** If saturated set to 200 MeV **/
        if ((*it).first.sat && (*it).second.sat) {
            xEnergy = 10000.0;
            yEnergy = 10000.0;
        }

        int xPosition = (*it).first.pos;
        int yPosition = (*it).second.pos;
	    double time = min((*it).first.t, (*it).second.t);
	    double bestDtime_mcp1 = numeric_limits<double>::max();
	    auto bestMatch_mcp1 = mcp1Event.end();	
	    double bestDtime_mcp2 = numeric_limits<double>::max();
	    auto bestMatch_mcp2 = mcp2Event.end();	
	    double bestDtime_veto = numeric_limits<double>::max();
	    auto bestMatch_veto = vetoEvent.end();
	    double tof = 0;
        double veto = 0;
        int max_gen = 0;
        /** Correlation with MCP1, MPC2 and Veto detector **/
	    for (vector<ChanEvent *>::iterator itm = mcp1Event.begin(); itm != mcp1Event.end(); itm++){
	        double dTime = abs(time - (*itm)->GetTime()) * Globals::get()->GetAdcClockInSeconds();
	        if (dTime < bestDtime_mcp1) 
                bestDtime_mcp1 = dTime; bestMatch_mcp1 = itm;
	    }
	    for (vector<ChanEvent *>::iterator itm = mcp2Event.begin(); itm != mcp2Event.end(); itm++){
	        double dTime = abs(time - (*itm)->GetTime()) * Globals::get()->GetAdcClockInSeconds();
	        if (dTime < bestDtime_mcp2)
                bestDtime_mcp2 = dTime; bestMatch_mcp2 = itm;
	    }
        for (vector<ChanEvent *>::iterator itv = vetoEvent.begin(); itv != vetoEvent.end(); itv++){
	        double dTime = abs(time - (*itv)->GetTime()) * Globals::get()->GetAdcClockInSeconds();
	        if (dTime < bestDtime_veto)
                bestDtime_veto = dTime; bestMatch_veto = itv;
	    }
        if (bestDtime_mcp1 < tofWindow_ &&  bestDtime_mcp2 < tofWindow_) {
	    	tof = (((*bestMatch_mcp2)->GetTime() - (*bestMatch_mcp1)->GetTime()) 
            * Globals::get()->GetAdcClockInSeconds())/(1e-9);
            corEvent.type = EventInfo::IMPLANT_EVENT;
            corEvent.energy = yEnergy;
            max_gen = corr.GetGen(xPosition, yPosition);
    	    plot(D_GEN,max_gen);
	    }	 	
	    else if(bestDtime_mcp1 < tofWindow_) {
	    	tof = bestDtime_mcp1;
            corEvent.type = EventInfo::IMPLANT_EVENT;
            corEvent.energy = yEnergy;
            max_gen = corr.GetGen(xPosition, yPosition);
	    } 
        else if(bestDtime_mcp2 < tofWindow_) {
	    	tof = bestDtime_mcp2;
            corEvent.type = EventInfo::IMPLANT_EVENT;
            corEvent.energy = yEnergy;
            max_gen = corr.GetGen(xPosition, yPosition);
            plot(D_GEN,max_gen);
	    }
        else {
            tof = 0;
            corEvent.type = EventInfo::DECAY_EVENT;
            corEvent.energy = xEnergy;
        }
        if (bestDtime_veto < vetoWindow_) {
	    	plot(DD_VETO_ENERGY, (*bestMatch_veto)->GetCalibratedEnergy(),yEnergy/3);
            veto = (*bestMatch_veto)->GetCalibratedEnergy();
        }

        corEvent.time = time;
        corr.Correlate(corEvent, xPosition, yPosition);
        if (corr.GetCondition() == Correlator::VALID_DECAY) {
            double Tlog = log2(((corr.GetDecayGenTime(xPosition, yPosition)) 
            * Globals::get()->GetAdcClockInSeconds())/(1e-9))*10;
        	plot(DD_KHS, xEnergy, Tlog);
            if (Tlog > 250 )
                corr.Flag(xPosition,yPosition);
            if(corr.GetGen(xPosition, yPosition) > 1) 
                plot(DD_ALPHA_ALPHA, xEnergy/5, corr.GetPreviousDecayEnergy(xPosition, yPosition)/5);
            if(corr.GetGen(xPosition, yPosition) == 1)
                plot(DD_ALPHA_IMPLANT, xEnergy/5, corr.GetImplantEnergy(xPosition, yPosition)/5);
            if(!corr.IsFlagged(xPosition,yPosition) ){
            	plot(DD_KHS_GATE, xEnergy, Tlog);
                if(corr.GetGen(xPosition, yPosition) > 1)
                    plot(DD_ALPHA_ALPHA_GATE, xEnergy/5, corr.GetPreviousDecayEnergy(xPosition, yPosition)/5);
            }   
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
	    	dssdstruc.Tof = tof;
	    	dssdstruc.Veto = veto;
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
    }
    EndProcess();
    return true;
}