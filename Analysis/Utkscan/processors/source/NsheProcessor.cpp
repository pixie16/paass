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

NsheProcessor::NsheProcessor(int calib, double timeWindow, double tofWindow, double vetoWindow,  double deltaEnergy, double highEnergyCut,
 double lowEnergyCut, double zero_suppress, double fissionEnergyCut, double minImpTime,
  double corrTime, double fastTime):EventProcessor(OFFSET, RANGE, "NsheProcessor"){ //, correlator_(numBackStrips, numFrontStrips) {
    calib_ = calib;
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
    DeclareHistogram1D(D_DTIME, SA, "Pairs time diff in 10 ns (+ 1 bin)");
    DeclareHistogram1D(D_DTIMETOF, SD, "Pairs time diff in 10 ns (+ 1 bin) for toff");
    DeclareHistogram1D(D_DTIMEVETO, SA, "time diff in 10 ns (+ 1 bin) for veto");
    DeclareHistogram1D(D_TOF_RAW, SD, "Pairs time diff in 10 ns (+ 1 bin) for toff raw");

    /** 2D graph for FB correlation **/
    DeclareHistogram2D(DD_EVENT_POSITION, xBins, yBins, "DSSD all events positions");
    DeclareHistogram2D(DD_MAXEVENT_ENERGY__X_POSITION, energyBins, xBins, "DSSD X strips E vs. position RAW");
    DeclareHistogram2D(DD_MAXEVENT_ENERGY__Y_POSITION, energyBins, yBins, "DSSD Y strips E vs. position RAW");
    DeclareHistogram2D(DD_EVENT_ENERGY__X_POSITION, energyBins, xBins, "DSSD X strips E vs. position");
    DeclareHistogram2D(DD_EVENT_ENERGY__Y_POSITION, energyBins, yBins, "DSSD Y strips E vs. position");
    DeclareHistogram2D(DD_FRONTE__BACKE, SB, SB, "Front vs Back energy");
    DeclareHistogram2D(DD_ENERGY__POSX_T_MISSING, energyBins, xBins, "DSSD T missing X strips E vs. position");
    DeclareHistogram2D(DD_ENERGY__POSY_T_MISSING, energyBins, yBins, "DSSD T missing Y strips E vs. position");
    DeclareHistogram2D(DD_TOF_ENERGY, SA , SD, "TOF vs DSSD");
    DeclareHistogram2D(DD_VETO_ENERGY, SB , SB, "DSSD dE dY correlated events");

    /** Control spectrum from the correlator **/
    DeclareHistogram2D(DD_KHS, energyBins , S9, "DSSD E vs log2(Decay Time) (/100)");
    DeclareHistogram2D(DD_ALPHA_ALPHA, SB , SB, "Decay vs Decay Energy");
    DeclareHistogram2D(DD_ALPHA_IMPLANT, SB , SB, " Decay Energy vs Implant Energy for first gen");
    DeclareHistogram2D(DD_KHS_GATE, energyBins , S9, "DSSD E vs log2(Decay Time) (/100) for short decay");
    DeclareHistogram2D(DD_ALPHA_ALPHA_GATE, SB , SB, "Decay vs Decay Energy for short decay");
    DeclareHistogram2D(DD_DSSD_TUNNEL, SB , SB, "Side detector vs DSSD energy");

    DeclareHistogram1D(D_GEN, S4, "number of max generation");

    DeclareHistogram2D(DD_QDC1_DSSD, SB , SD, "QDC1 vs Energy DSSD");
    DeclareHistogram2D(DD_QDC1_TOF, SA , SD, "QDC1 vs TOF");
    DeclareHistogram2D(DD_QDC2_DSSD, SB , SE, "QDC2 vs Energy DSSD");
    DeclareHistogram2D(DD_QDC2_TOF, SA , SE, "QDC2 vs TOF");
    DeclareHistogram2D(DD_TOF1_DSSD, SA , SD, "TOF1 vs DSSD");
    DeclareHistogram2D(DD_TOF2_DSSD, SA , SD, "TOF2 vs DSSD");
    DeclareHistogram2D(DD_MULTI, S4 , S6, "TOF2 vs DSSD");

}

bool NsheProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    if (DetectorDriver::get()->GetSysRootOutput())
        dssdstruc = processor_struct::DSSD_DEFAULT_STRUCT;
    
        xyEventsTMatch_.clear(); // part of the NsheProcessor class
        xyEventsEMatch_.clear(); 
        mcp1EventsTMatch.clear();
        mcp2EventsTMatch.clear();
        vetoEventsTMatch.clear();   
        xEventsTMatch.clear();
        yEventsTMatch.clear();
        sideEventsTMatch.clear();

        vector<ChanEvent *> xEvents = event.GetSummary("SHE:dssd_front", true)->GetList();
        vector<ChanEvent *> yEvents = event.GetSummary("SHE:dssd_back", true)->GetList();
        vector<ChanEvent *> mcp1Events = event.GetSummary("SHE:mcp1", true)->GetList();
        vector<ChanEvent *> mcp2Events = event.GetSummary("SHE:mcp2", true)->GetList();
	    vector<ChanEvent *> vetoEvents = event.GetSummary("SHE:veto", true)->GetList();
        vector<ChanEvent *> sideEvents = event.GetSummary("SHE:side", true)->GetList();
        /**
         * Matching the front-back by the time correlations
         */
        
        double Time_mcp1 = 0;
        double Time_mcp2 = 0;

        for (vector<ChanEvent *>::iterator itx = xEvents.begin();
            itx != xEvents.end(); ++itx) {
            // if((*itx)->GetCalibratedEnergy() < zero_suppress_ && !(*itx)->IsPileup()) 
            //     continue;	

            StripEvent ev((*itx)->GetCalibratedEnergy(), (*itx)->GetTime() * Globals::get()->GetAdcClockInSeconds(), (*itx)->GetChanID().GetLocation(),
            (*itx)->IsSaturated(), (*itx)->GetTrace(), (*itx)->IsPileup());
            
            pair<StripEvent, bool> match(ev, false);
            xEventsTMatch.push_back(match);
            
            plot(DD_MAXEVENT_ENERGY__X_POSITION, (*itx)->GetCalibratedEnergy()/10, (*itx)->GetChanID().GetLocation());
            plot(DD_MULTI, 0, xEvents.size());
        }

        for (vector<ChanEvent *>::iterator ity = yEvents.begin(); ity != yEvents.end(); ++ity) {
            // if((*ity)->GetCalibratedEnergy() < zero_suppress_ && !(*ity)->IsPileup()) 
            //     continue;

            StripEvent ev((*ity)->GetCalibratedEnergy(), (*ity)->GetTime() * Globals::get()->GetAdcClockInSeconds(), (*ity)->GetChanID().GetLocation(), 
            (*ity)->IsSaturated(),(*ity)->GetTrace(), (*ity)->IsPileup());

            pair<StripEvent, bool> match(ev, false);
            yEventsTMatch.push_back(match);

            plot(DD_MAXEVENT_ENERGY__Y_POSITION, (*ity)->GetCalibratedEnergy()/10, (*ity)->GetChanID().GetLocation());
            plot(DD_MULTI, 1, yEvents.size());
        }
        for (vector<ChanEvent *>::iterator itx = mcp1Events.begin();
            itx != mcp1Events.end(); ++itx) {	

            StripEvent ev((*itx)->GetCalibratedEnergy(), (*itx)->GetTime() * Globals::get()->GetAdcClockInSeconds(), (*itx)->GetChanID().GetLocation(),
            (*itx)->IsSaturated(), (*itx)->GetTrace(), (*itx)->IsPileup(), (*itx)->GetQdc());
            pair<StripEvent, bool> match(ev, false);
            mcp1EventsTMatch.push_back(match);
            plot(DD_MULTI, 2, mcp1Events.size());
        }

        for (vector<ChanEvent *>::iterator itx = mcp2Events.begin();
            itx != mcp2Events.end(); ++itx) {	

            StripEvent ev((*itx)->GetCalibratedEnergy(), (*itx)->GetTime() * Globals::get()->GetAdcClockInSeconds(), (*itx)->GetChanID().GetLocation(),
            (*itx)->IsSaturated(), (*itx)->GetTrace(), (*itx)->IsPileup(), (*itx)->GetQdc());
            pair<StripEvent, bool> match(ev, false);
            mcp2EventsTMatch.push_back(match);
            plot(DD_MULTI, 3, mcp2Events.size());
        }

        for (vector<ChanEvent *>::iterator itx = vetoEvents.begin();
            itx != vetoEvents.end(); ++itx) {	

            StripEvent ev((*itx)->GetCalibratedEnergy(), (*itx)->GetTime() * Globals::get()->GetAdcClockInSeconds(), (*itx)->GetChanID().GetLocation(),
            (*itx)->IsSaturated(), (*itx)->GetTrace(), (*itx)->IsPileup());
            pair<StripEvent, bool> match(ev, false);
            vetoEventsTMatch.push_back(match);
            plot(DD_MULTI, 4, vetoEvents.size());
        }

        for (vector<ChanEvent *>::iterator itx = sideEvents.begin();
            itx != sideEvents.end(); ++itx) {	

            StripEvent ev((*itx)->GetCalibratedEnergy(), (*itx)->GetTime() * Globals::get()->GetAdcClockInSeconds(), (*itx)->GetChanID().GetLocation(),
            (*itx)->IsSaturated(), (*itx)->GetTrace(), (*itx)->IsPileup());
            pair<StripEvent, bool> match(ev, false);
            sideEventsTMatch.push_back(match);
            plot(DD_MULTI, 5, sideEvents.size());
        }

        if(!mcp1Events.empty() && !mcp2Events.empty()){
            Time_mcp1 = mcp1Events.at(0)->GetTime() * Globals::get()->GetAdcClockInSeconds();
            Time_mcp2 = mcp2Events.at(0)->GetTime() * Globals::get()->GetAdcClockInSeconds();
            plot(D_TOF_RAW, (Time_mcp2 - Time_mcp1)*(1e9)+500);
        }

    if(!calib_){
        for (vector<pair<StripEvent, bool>>::iterator ity = yEventsTMatch.begin();
            ity != yEventsTMatch.end(); ++ity) {

            double bestDtime = numeric_limits<double>::max();
            vector<pair<StripEvent, bool> >::iterator bestMatch = xEventsTMatch.end();
            vector<pair<StripEvent, bool> >::iterator bestMatch_sat = xEventsTMatch.end();

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

                double dTime = (*itx).first.t - (*ity).first.t;
                double dEnergy = abs(energyX - energyY);
                if (abs(dTime) < bestDtime) {
                    bestDtime = dTime; 
                    bestMatch = itx;
                }
            }
            if (bestDtime < timeWindow_) {
                xyEventsTMatch_.push_back(pair<StripEvent, StripEvent>((*bestMatch).first, (*ity).first));
                (*ity).second = true;
                (*bestMatch).second = true;
                plot(D_DTIME, int(bestDtime*1e+9) + 500);
            } else {
                bestDtime = int(bestDtime);
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
        }
        for (vector<pair<StripEvent, bool> > ::iterator ity = yEventsTMatch.begin();
            ity != yEventsTMatch.end(); ++ity) {
            if ((*ity).second)
                continue;
            int position = (*ity).first.pos;
            double energy = (*ity).first.E;
        }
    }
    
    return true;
}

bool NsheProcessor::Process(RawEvent &event) {
    using namespace dammIds::dssd4she;

    if (!EventProcessor::Process(event))
        return false;
    
    if(!calib_){
        static Correlator corr(minImpTime_,corrTime_,fastTime_);
        EventInfo corEvent;
        for (vector<pair<StripEvent, StripEvent> >::iterator it = xyEventsTMatch_.begin();
            it != xyEventsTMatch_.end(); ++it) {
            double xEnergy = (*it).first.E;
            double yEnergy = (*it).second.E;

            if (xEnergy>lowEnergyCut_ && yEnergy<highEnergyCut_ ) 
                xEnergy = yEnergy;
            /** If saturated set to 200 MeV **/
            else if (yEnergy>highEnergyCut_){
                xEnergy = highEnergyCut_;
                yEnergy = highEnergyCut_;
            }

            int xPosition = (*it).first.pos;
            int yPosition = (*it).second.pos;
            double time = max((*it).first.t, (*it).second.t);
            double bestDtime_mcp1 = numeric_limits<double>::max();
            auto bestMatch_mcp1 = mcp1EventsTMatch.end();
            vector<unsigned> bestQDC_mcp1 ;	
            double bestDtime_mcp2 = numeric_limits<double>::max();
            auto bestMatch_mcp2 = mcp2EventsTMatch.end();
            vector<unsigned> bestQDC_mcp2 ;
            double bestDtime_veto = numeric_limits<double>::max();
            auto bestMatch_veto = vetoEventsTMatch.end();
            double bestDtime_side = numeric_limits<double>::max();
            auto bestMatch_side = vetoEventsTMatch.end();
            double tof = 0;
            double veto = 0;
            double side = 0;
            int max_gen = 0;
            /** Correlation with MCP1, MPC2 and Veto detector **/
            for (vector<pair<StripEvent, bool>>::iterator itm = mcp1EventsTMatch.begin();
                itm != mcp1EventsTMatch.end(); ++itm) {
                if((*itm).second)
                    continue;
                double dTime = abs(time - (*itm).first.t);
                if (dTime < abs(bestDtime_mcp1) ){
                    bestDtime_mcp1 = time - (*itm).first.t; 
                    bestMatch_mcp1 = itm; 
                    bestQDC_mcp1 = (*itm).first.Qdc;
                }
            }
            for (vector<pair<StripEvent, bool>>::iterator itm = mcp2EventsTMatch.begin();
                itm != mcp2EventsTMatch.end(); ++itm){
                if((*itm).second)
                    continue;
                double dTime = abs(time - (*itm).first.t);
                if (dTime < abs(bestDtime_mcp2)){
                    bestDtime_mcp2 = time - (*itm).first.t;
                    bestMatch_mcp2 = itm;                   
                    bestQDC_mcp2 = (*itm).first.Qdc;
                }
            }
            for (vector<pair<StripEvent, bool>>::iterator itv = vetoEventsTMatch.begin();
                itv != vetoEventsTMatch.end(); ++itv){
                if((*itv).second)
                    continue;
                double dTime = (time - (*itv).first.t);
                if (dTime < bestDtime_veto)
                    bestDtime_veto = dTime; bestMatch_veto = itv;
            }
            for (vector<pair<StripEvent, bool>>::iterator its = sideEventsTMatch.begin();
                its != sideEventsTMatch.end(); ++its){
                if((*its).second)
                    continue;
                double dTime = (time - (*its).first.t);
                if (dTime < bestDtime_side)
                    bestDtime_side = dTime; bestMatch_side = its;
            }

            if (abs(bestDtime_mcp1) < tofWindow_ &&  abs(bestDtime_mcp2) < tofWindow_ && (bestDtime_mcp1) > 0 && (bestDtime_mcp2) > 0){
                // tof = ((*bestMatch_mcp2).first.t - (*bestMatch_mcp1).first.t)/(1e-9) + 400;
                tof = (bestDtime_mcp1-bestDtime_mcp2)/(1e-9) + 400;
                corEvent.type = EventInfo::IMPLANT_EVENT;
                corEvent.energy = yEnergy;
                max_gen = corr.GetGen(xPosition, yPosition);
                plot(D_GEN,max_gen);
                (*bestMatch_mcp1).second = true;
                (*bestMatch_mcp2).second = true;
                if(!bestQDC_mcp1.empty()){
                    // plot(DD_QDC1_DSSD,xEnergy/10,bestQDC_mcp1.at(1));
                    // plot(DD_QDC1_TOF,tof,bestQDC_mcp1.at(1) );
                    plot(DD_QDC1_DSSD,xEnergy/10,bestQDC_mcp1.at(6) - 13*bestQDC_mcp1.at(7)/16);
                    plot(DD_QDC1_TOF,tof,bestQDC_mcp1.at(6) - 13*bestQDC_mcp1.at(7)/16);
                }
                if(!bestQDC_mcp2.empty()){
                    // plot(DD_QDC2_DSSD,xEnergy/10,bestQDC_mcp2.at(1) );
                    // plot(DD_QDC2_TOF,tof,bestQDC_mcp2.at(1) );
                    plot(DD_QDC2_DSSD,xEnergy/10,bestQDC_mcp2.at(6) - 13*bestQDC_mcp2.at(7)/16);
                    plot(DD_QDC2_TOF,tof,bestQDC_mcp2.at(6) - 13*bestQDC_mcp2.at(7)/16);
                }
                plot(DD_TOF_ENERGY, tof,yEnergy/10);
                plot(D_DTIMETOF, tof);
            }	 	
            else{
                if(abs(bestDtime_mcp1) < tofWindow_ && (bestDtime_mcp1) > 0) {
                    tof = bestDtime_mcp1/(1e-9) + 400;
                    corEvent.type = EventInfo::IMPLANT_EVENT;
                    corEvent.energy = yEnergy;
                    max_gen = corr.GetGen(xPosition, yPosition);
                    (*bestMatch_mcp1).second = true;
                    if(!bestQDC_mcp1.empty()){
                        // plot(DD_QDC1_DSSD,xEnergy/10,bestQDC_mcp1.at(1));
                        // plot(DD_QDC1_TOF,tof,bestQDC_mcp1.at(1) );
                        plot(DD_QDC1_DSSD,xEnergy/10,bestQDC_mcp1.at(6) - 13*bestQDC_mcp1.at(7)/16);
                        plot(DD_QDC1_TOF,tof,bestQDC_mcp1.at(6) - 13*bestQDC_mcp1.at(7)/16);
                    }
                    plot(DD_TOF1_DSSD, tof,yEnergy/10);
                } 
                if(abs(bestDtime_mcp2) < tofWindow_ && (bestDtime_mcp2) > 0) {
                    tof = bestDtime_mcp2/(1e-9) + 400;
                    corEvent.type = EventInfo::IMPLANT_EVENT;
                    corEvent.energy = yEnergy;
                    max_gen = corr.GetGen(xPosition, yPosition);
                    plot(D_GEN,max_gen);
                    (*bestMatch_mcp2).second = true;
                    if(!bestQDC_mcp2.empty()){
                        // plot(DD_QDC2_DSSD,xEnergy/10,bestQDC_mcp2.at(1));
                        // plot(DD_QDC2_TOF,tof,bestQDC_mcp2.at(1) );
                        plot(DD_QDC2_DSSD,xEnergy/10,bestQDC_mcp2.at(6) - 13*bestQDC_mcp2.at(7)/16);
                        plot(DD_QDC2_TOF,tof,bestQDC_mcp2.at(6) - 13*bestQDC_mcp2.at(7)/16);
                    }
                    plot(DD_TOF2_DSSD, tof,yEnergy/10);
                }
                else {
                    tof = 0;
                    corEvent.type = EventInfo::DECAY_EVENT;
                    corEvent.energy = xEnergy;
                }
            } 
            if (bestDtime_veto < vetoWindow_) {
                plot(DD_VETO_ENERGY, (*bestMatch_veto).first.E,yEnergy/100);
                plot(D_DTIMEVETO, bestDtime_veto/(1e-8));
                veto = (*bestMatch_veto).first.E;
                (*bestMatch_veto).second = true;
            }
            if (bestDtime_side < vetoWindow_) {
                plot(DD_DSSD_TUNNEL, xEnergy/10, (*bestMatch_side).first.E);
                side = (*bestMatch_side).first.E;
                (*bestMatch_side).second = true;
            }

            corEvent.time = time;
            corr.Correlate(corEvent, xPosition, yPosition);
            if (corr.GetCondition() == Correlator::VALID_DECAY) {
                double Tlog = log2(((corr.GetDecayGenTime(xPosition, yPosition)))/(1e-9))*10;
                plot(DD_KHS, xEnergy/10, Tlog);
                if (Tlog > 330 )
                    corr.Flag(xPosition,yPosition);
                if(corr.GetGen(xPosition, yPosition) > 1) 
                    plot(DD_ALPHA_ALPHA, xEnergy/10, corr.GetPreviousDecayEnergy(xPosition, yPosition)/10);
                if(corr.GetGen(xPosition, yPosition) == 1)
                    plot(DD_ALPHA_IMPLANT, xEnergy/10, corr.GetImplantEnergy(xPosition, yPosition)/10);
                if(!corr.IsFlagged(xPosition,yPosition)){
                    plot(DD_KHS_GATE, xEnergy/10, Tlog);
                    if(corr.GetGen(xPosition, yPosition) > 1)
                        plot(DD_ALPHA_ALPHA_GATE, xEnergy/10, corr.GetPreviousDecayEnergy(xPosition, yPosition)/10);
                }   
            }

            if (DetectorDriver::get()->GetSysRootOutput()){
            //Fill Root struct
                dssdstruc.x = xPosition;
                dssdstruc.y = yPosition;
                dssdstruc.Energy_front = xEnergy;
                dssdstruc.Energy_back = yEnergy;
                dssdstruc.Time = time;
                dssdstruc.Tof = tof;
                if(!bestQDC_mcp1.empty()) dssdstruc.QDC1 = bestQDC_mcp1.at(6) - 13*bestQDC_mcp1.at(7)/16;
                if(!bestQDC_mcp2.empty()) dssdstruc.QDC2 = bestQDC_mcp2.at(6) - 13*bestQDC_mcp2.at(7)/16;
                // if(bestDtime_veto < vetoWindow_) {
                //     dssdstruc.Trace_Veto = (*bestMatch_veto).first.trace;
                //     dssdstruc.Veto = (*bestMatch_veto).first.E;
                // }
                dssdstruc.Trace_Front = (*it).first.trace;
                dssdstruc.Trace_Back = (*it).second.trace;
                dssdstruc.pile_up = (*it).second.pileup; 
                if(bestDtime_mcp1 < tofWindow_) dssdstruc.Trace_TOF1 = (*bestMatch_mcp1).first.trace;
                if(bestDtime_mcp2 < tofWindow_) dssdstruc.Trace_TOF2 = (*bestMatch_mcp2).first.trace;
                // if(bestDtime_side < vetoWindow_) {
                //     dssdstruc.Trace_Side = (*bestMatch_side).first.trace;
                //     dssdstruc.Side = (*bestMatch_side).first.E;
                // }
                pixie_tree_event_->dssd_vec_.emplace_back(dssdstruc);
                dssdstruc = processor_struct::DSSD_DEFAULT_STRUCT;
            }
            plot(D_ENERGY_X, xEnergy/10);
            plot(D_ENERGY_Y, yEnergy/10);
            plot(DD_FRONTE__BACKE, xEnergy/100, yEnergy/100);
            plot(DD_EVENT_ENERGY__X_POSITION, xEnergy/10, xPosition);
            plot(DD_EVENT_ENERGY__Y_POSITION, yEnergy/10, yPosition);
            plot(DD_EVENT_POSITION, xPosition, yPosition);
        }
    }
    else{
        for (vector<pair<StripEvent, bool>>::iterator it = xEventsTMatch.begin();
            it != xEventsTMatch.end(); ++it) {
            if (DetectorDriver::get()->GetSysRootOutput()){
            //Fill Root struct
                dssdstruc.x = (*it).first.pos;
                dssdstruc.y = -1;
                dssdstruc.Energy_front = (*it).first.E;
                dssdstruc.Energy_back = 0;
                dssdstruc.Time = (*it).first.t;
                dssdstruc.Tof = -4000;
                dssdstruc.Veto = -4000;
                dssdstruc.Trace_Front= (*it).first.trace;
                dssdstruc.Trace_Back.clear() ;
                dssdstruc.pile_up = (*it).first.pileup; 
                dssdstruc.Trace_TOF1.clear();
                dssdstruc.Trace_TOF2.clear();
                pixie_tree_event_->dssd_vec_.emplace_back(dssdstruc);
                dssdstruc = processor_struct::DSSD_DEFAULT_STRUCT;
            }
        }
        for (vector<pair<StripEvent, bool>>::iterator it = yEventsTMatch.begin();
            it != yEventsTMatch.end(); ++it) {
            if (DetectorDriver::get()->GetSysRootOutput()){
            //Fill Root struct
                dssdstruc.x = -1;
                dssdstruc.y = (*it).first.pos;
                dssdstruc.Energy_front = 0;
                dssdstruc.Energy_back = (*it).first.E;
                dssdstruc.Time = (*it).first.t;
                dssdstruc.Tof = -4000;
                dssdstruc.Veto = -4000;
                dssdstruc.Trace_Front.clear();
                dssdstruc.Trace_Back = (*it).first.trace;
                dssdstruc.pile_up = (*it).first.pileup; 
                dssdstruc.Trace_TOF1.clear();
                dssdstruc.Trace_TOF2.clear();
                pixie_tree_event_->dssd_vec_.emplace_back(dssdstruc);
                dssdstruc = processor_struct::DSSD_DEFAULT_STRUCT;
            }
        }
       
        for (vector<pair<StripEvent, bool>>::iterator it = mcp1EventsTMatch.begin();
            it != mcp1EventsTMatch.end(); ++it) {
            double bestDtime_mcp2 = numeric_limits<double>::max();
            auto bestMatch_mcp2 = mcp2EventsTMatch.end();
                for (vector<pair<StripEvent, bool>>::iterator itm = mcp2EventsTMatch.begin();
                itm != mcp2EventsTMatch.end(); ++itm) {
                    if((*itm).second)
                        continue;
                double dTime =((*it).first.t - (*itm).first.t);
                if (abs(dTime) < bestDtime_mcp2)
                    bestDtime_mcp2 = dTime; bestMatch_mcp2 = itm;
                }
                if (DetectorDriver::get()->GetSysRootOutput()){
            //Fill Root struct
                dssdstruc.x = -1;
                dssdstruc.y = -1;
                dssdstruc.Energy_front = 0;
                dssdstruc.Energy_back = 0;
                dssdstruc.Time = (*it).first.t;
                dssdstruc.Tof = bestDtime_mcp2/1e-9;
                dssdstruc.Veto = -4000;
                // dssdstruc.Trace_Front.clear();
                // dssdstruc.Trace_Back.clear() ;
                dssdstruc.pile_up = (*it).first.pileup; 
                // dssdstruc.Trace_TOF1= (*it).first.trace;
                // dssdstruc.Trace_TOF2= (*bestMatch_mcp2).first.trace;
                pixie_tree_event_->dssd_vec_.emplace_back(dssdstruc);
                dssdstruc = processor_struct::DSSD_DEFAULT_STRUCT;
            }
            (*bestMatch_mcp2).second = true;
        }
        
        for (vector<pair<StripEvent, bool>>::iterator it = vetoEventsTMatch.begin();
            it != vetoEventsTMatch.end(); ++it) {
            if (DetectorDriver::get()->GetSysRootOutput()){
            //Fill Root struct
                dssdstruc.x = -1;
                dssdstruc.y = -1;
                dssdstruc.Energy_front = 0;
                dssdstruc.Energy_back = 0;
                dssdstruc.Time = (*it).first.t;
                dssdstruc.Tof = -4000;
                dssdstruc.Veto = (*it).first.E;
                dssdstruc.Trace_Front.clear();
                dssdstruc.Trace_Back.clear();
                dssdstruc.pile_up = (*it).first.pileup; 
                dssdstruc.Trace_TOF1.clear();
                dssdstruc.Trace_TOF2.clear();
                pixie_tree_event_->dssd_vec_.emplace_back(dssdstruc);
                dssdstruc = processor_struct::DSSD_DEFAULT_STRUCT;
            }
        }
    }
    EndProcess();
    return true;
}