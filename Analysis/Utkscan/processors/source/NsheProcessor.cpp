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
    beforelast_reset = 0;
    last_reset = 0;
    associatedTypes.insert("SHE");
}

NsheProcessor::NsheProcessor(int calib, double timeWindow, double tofWindow, double vetoWindow,  double deltaEnergy, double highEnergyCut,
 double lowEnergyCut, double zero_suppress, double fissionEnergyCut, double minImpTime,
  double corrTime, double fastTime, double khs_limit_cor,int nshe_exp):EventProcessor(OFFSET, RANGE, "NsheProcessor"){ //, correlator_(numBackStrips, numFrontStrips) {
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
    khs_limit_cor_ = khs_limit_cor;
    nshe_exp_ = nshe_exp;
    beforelast_reset = 0;
    last_reset = 0;
    associatedTypes.insert("SHE");
}


void NsheProcessor::DeclarePlots(void) {
    using namespace dammIds::dssd4she;

    const int energyBins = SB;
    const int xBins = S6;
    const int yBins = S5;

    /** 1D graph Timming and energy **/
    DeclareHistogram1D(D_ENERGY_X, energyBins, "Energy/10 dssd X strips");
    DeclareHistogram1D(D_ENERGY_Y, energyBins, "Energy/10 dssd Y strips");
    DeclareHistogram1D(D_DTIME, SA, "Pairs time diff in 10 ns (+ 1 bin)");
    DeclareHistogram1D(D_DTIME_SIDE, SD, "Pairs time diff DSSD Side");
    DeclareHistogram1D(D_DTIME_SIDE_RAW, SD, "Pairs time diff DSSD Side");
    DeclareHistogram1D(D_DTIMETOF, SD, "Pairs time diff in 10 ns (+ 1 bin) for toff");
    DeclareHistogram1D(D_DTIMEVETO, SA, "time diff in 10 ns (+ 1 bin) for veto");
    DeclareHistogram1D(D_TOF_RAW, SD, "Pairs time diff in 10 ns (+ 1 bin) for toff raw");

    /** 2D graph for FB correlation **/
    DeclareHistogram2D(DD_EVENT_POSITION, xBins, yBins, "DSSD all events positions");
    DeclareHistogram2D(DD_EVENT_POSITION_FROM_E, xBins, yBins, "DSSD all events positions");
    DeclareHistogram2D(DD_MAXEVENT_ENERGY__X_POSITION, energyBins, xBins, "DSSD X strips E vs. position RAW");
    DeclareHistogram2D(DD_MAXEVENT_ENERGY__Y_POSITION, energyBins, yBins, "DSSD Y strips E vs. position RAW");
    DeclareHistogram2D(DD_EVENT_ENERGY__X_POSITION, energyBins, xBins, "DSSD X strips E vs. position");
    DeclareHistogram2D(DD_EVENT_ENERGY__Y_POSITION, energyBins, yBins, "DSSD Y strips E vs. position");
    DeclareHistogram2D(DD_FRONTE__BACKE, SB, SB, "Front vs Back energy");
    DeclareHistogram2D(DD_ENERGY__POSX_T_MISSING, energyBins, xBins, "DSSD T missing X strips E vs. position");
    DeclareHistogram2D(DD_ENERGY__POSY_T_MISSING, energyBins, yBins, "DSSD T missing Y strips E vs. position");
    DeclareHistogram2D(DD_TOF_ENERGY, SA , SD, "TOF vs DSSD");
    DeclareHistogram2D(DD_VETO_ENERGY, SB , SB, "DSSD dE dY correlated events");
    DeclareHistogram2D(DD_DENERGY_SIDE_NUM, S3 , SD, "DSSD dE dY correlated events");
    DeclareHistogram2D(DD_DENERGY_SIDE_NUM_RAW, S3 , energyBins, "Elastic timming vs DSSD Back energy/10");

    /** Elastic DSSD **/
    DeclareHistogram2D(DD_ELA_ENERGY_RAW, SC , SB, "Timming (10ns) vs Energy DSSD Y (10keV)");

    /** Control spectrum from the correlator **/
    DeclareHistogram2D(DD_KHS, energyBins , S9, "DSSD E vs log2(Decay Time) (/100)");
    DeclareHistogram2D(DD_ALPHA_ALPHA, SB , SB, "Decay vs Decay Energy");
    DeclareHistogram2D(DD_ALPHA_IMPLANT, SB , SB, " Decay Energy vs Implant Energy for first gen");
    DeclareHistogram2D(DD_KHS_GATE, energyBins , S9, "DSSD E vs log2(Decay Time) (/100) for short decay");
    DeclareHistogram2D(DD_ALPHA_ALPHA_GATE, SB , SB, "Decay vs Decay Energy for short decay");
    DeclareHistogram2D(DD_DSSD_TUNNEL, SB , S9, "Side detector vs DSSD energy");

    DeclareHistogram1D(D_GEN, S4, "number of max generation");

    DeclareHistogram2D(DD_QDC1_DSSD, SB , SD, "QDC1 vs Energy DSSD");
    DeclareHistogram2D(DD_QDC1_TOF, SA , SE, "QDC1 vs TOF");
    DeclareHistogram2D(DD_QDC2_DSSD, SB , SD, "QDC2 vs Energy DSSD");
    DeclareHistogram2D(DD_QDC2_TOF, SA , SD, "QDC2 vs TOF");
    DeclareHistogram2D(DD_TOF1_DSSD, SB , SD, "TOF1 vs DSSD");
    DeclareHistogram2D(DD_TOF2_DSSD, SB , SD, "TOF2 vs DSSD");
    DeclareHistogram2D(DD_MULTI, S4 , S6, "TOF2 vs DSSD");
    DeclareHistogram2D(DD_SIDE_N, S3 , SB, "TOF2 vs DSSD");


    /** Eleastic Histo **/
    DeclareHistogram1D(D_ELA_TOT, SE, "Energy elastic tot");
    DeclareHistogram2D(DD_ELA_TARGET, SC , SB, "Elastic vs Time target (10ns)");
}

bool NsheProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    if (DetectorDriver::get()->GetSysRootOutput()){
        dssdstruc = processor_struct::DSSD_DEFAULT_STRUCT;
        elastruc = processor_struct::SHE_ELA_DEFAULT_STRUCT;
    }
        xyEventsTMatch_.clear(); // part of the NsheProcessor class
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
        vector<ChanEvent *> RotationEvents = event.GetSummary("SHE:rotation", true)->GetList();

        /** Updating last reset if it is present in the event **/
        if(!RotationEvents.empty()){
            beforelast_reset = last_reset;
            last_reset = (*RotationEvents.begin())->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9;
        }

        /** Matching the front-back by the time correlations **/
        double Time_mcp1 = 0;
        double Time_mcp2 = 0;

        for (vector<ChanEvent *>::iterator itx = xEvents.begin();
            itx != xEvents.end(); ++itx) {	

            StripEvent ev((*itx)->GetCalibratedEnergy(), (*itx)->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9, (*itx)->GetChanID().GetLocation(),
            (*itx)->IsSaturated(), (*itx)->GetTrace(), (*itx)->IsPileup());
            pair<StripEvent, bool> match(ev, false);
            xEventsTMatch.push_back(match);
            
            plot(DD_MAXEVENT_ENERGY__X_POSITION, (*itx)->GetCalibratedEnergy()/10, (*itx)->GetChanID().GetLocation());
            plot(DD_MULTI, 0, xEvents.size());
        }
        for (vector<ChanEvent *>::iterator ity = yEvents.begin();
            ity != yEvents.end(); ++ity) {

            StripEvent ev((*ity)->GetCalibratedEnergy(), (*ity)->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9, (*ity)->GetChanID().GetLocation(), 
            (*ity)->IsSaturated(),(*ity)->GetTrace(), (*ity)->IsPileup());
            pair<StripEvent, bool> match(ev, false);
            yEventsTMatch.push_back(match);

            plot(DD_MAXEVENT_ENERGY__Y_POSITION, (*ity)->GetCalibratedEnergy()/10, (*ity)->GetChanID().GetLocation());
            plot(DD_MULTI, 1, yEvents.size());
            if((*ity)->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9 >= last_reset)
                plot(DD_ELA_ENERGY_RAW,((*ity)->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9 - last_reset)/10,(*ity)->GetCalibratedEnergy()/10);
            else
                plot(DD_ELA_ENERGY_RAW,((*ity)->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9 - beforelast_reset)/10,(*ity)->GetCalibratedEnergy()/10);

        }
        for (vector<ChanEvent *>::iterator itx = mcp1Events.begin();
            itx != mcp1Events.end(); ++itx) {	
            StripEvent ev((*itx)->GetCalibratedEnergy(), (*itx)->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9, (*itx)->GetChanID().GetLocation(),(*itx)->IsSaturated(), (*itx)->GetTrace(), (*itx)->IsPileup(), (*itx)->GetQdc());
            pair<StripEvent, bool> match(ev, false);
            mcp1EventsTMatch.push_back(match);
            plot(DD_MULTI, 2, mcp1Events.size());
        }
        for (vector<ChanEvent *>::iterator itx = mcp2Events.begin();
            itx != mcp2Events.end(); ++itx) {	
            StripEvent ev((*itx)->GetCalibratedEnergy(), (*itx)->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9, (*itx)->GetChanID().GetLocation(),(*itx)->IsSaturated(), (*itx)->GetTrace(), (*itx)->IsPileup(), (*itx)->GetQdc());
            pair<StripEvent, bool> match(ev, false);
            mcp2EventsTMatch.push_back(match);
            plot(DD_MULTI, 3, mcp2Events.size());
        }
        for (vector<ChanEvent *>::iterator itx = vetoEvents.begin();
            itx != vetoEvents.end(); ++itx) {	

            StripEvent ev((*itx)->GetCalibratedEnergy(), (*itx)->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9, (*itx)->GetChanID().GetLocation(),
            (*itx)->IsSaturated(), (*itx)->GetTrace(), (*itx)->IsPileup());
            pair<StripEvent, bool> match(ev, false);
            vetoEventsTMatch.push_back(match);
            plot(DD_MULTI, 4, vetoEvents.size());
        }
        for (vector<ChanEvent *>::iterator itx = sideEvents.begin();
            itx != sideEvents.end(); ++itx) {	

            StripEvent ev((*itx)->GetCalibratedEnergy(), (*itx)->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9, (*itx)->GetChanID().GetLocation(),
            (*itx)->IsSaturated(), (*itx)->GetTrace(), (*itx)->IsPileup());
            pair<StripEvent, bool> match(ev, false);
            sideEventsTMatch.push_back(match);
            plot(DD_MULTI, 5, sideEvents.size());
            plot(DD_DENERGY_SIDE_NUM_RAW,(*itx)->GetChanID().GetLocation(),(*itx)->GetCalibratedEnergy());
        }
        
        if(!mcp1Events.empty() && !mcp2Events.empty()){ /** Raw TOF 1-2**/
            Time_mcp1 = mcp1Events.at(0)->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9;
            Time_mcp2 = mcp2Events.at(0)->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9;
            plot(D_TOF_RAW, (Time_mcp2 - Time_mcp1)*10+500);
        }

        if(!sideEvents.empty() && !xEvents.empty()){ 
            Time_mcp1 = sideEvents.at(0)->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9;
            Time_mcp2 = xEvents.at(0)->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9;
            plot(D_DTIME_SIDE_RAW, (Time_mcp2 - Time_mcp1) + 4000);
        }

    if(!calib_){ //* Construct Front Back pair event only on timming
        for (vector<pair<StripEvent, bool>>::iterator ity = yEventsTMatch.begin();
            ity != yEventsTMatch.end(); ++ity) {

            double bestDtime = numeric_limits<double>::max();
            vector<pair<StripEvent, bool> >::iterator bestMatch = xEventsTMatch.end();

            for (vector<pair<StripEvent, bool> >::iterator itx = xEventsTMatch.begin();
                itx != xEventsTMatch.end(); ++itx) {
                // If already matched, skip
                if ((*itx).second) 
                    continue;

                double dTime = (*itx).first.t - (*ity).first.t;
                if (abs(dTime) < bestDtime) {
                    bestDtime = dTime; 
                    bestMatch = itx;
                }
            }
            if (bestDtime < timeWindow_) {
                xyEventsTMatch_.push_back(pair<StripEvent, StripEvent>((*bestMatch).first, (*ity).first));
                (*ity).second = true;
                (*bestMatch).second = true;
                plot(D_DTIME, int(bestDtime) + 500);
            } 
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
        /** Event based on a DSSD trigger : TOF Veto and potential Side search link to the DSSD event **/
        for (vector<pair<StripEvent, StripEvent> >::iterator it = xyEventsTMatch_.begin();
            it != xyEventsTMatch_.end(); ++it) {
            double xEnergy = (*it).first.E;
            double yEnergy = (*it).second.E;

            if (xEnergy > lowEnergyCut_ && yEnergy < highEnergyCut_ ) 
                xEnergy = yEnergy;
            /** If saturated set to 200 MeV **/
            else if (yEnergy > highEnergyCut_){
                xEnergy = highEnergyCut_;
                yEnergy = highEnergyCut_;
            }

            int xPosition = (*it).first.pos;
            int yPosition = (*it).second.pos;
            double time = max((*it).first.t,(*it).second.t);
            double time_t = 0;
            double bestDtime_mcp1 = numeric_limits<double>::max();
            auto bestMatch_mcp1 = mcp1EventsTMatch.end();
            vector<unsigned> bestQDC_mcp1 ;	
            double bestDtime_mcp2 = numeric_limits<double>::max();
            auto bestMatch_mcp2 = mcp2EventsTMatch.end();
            vector<unsigned> bestQDC_mcp2 ;
            double bestDtime_veto = numeric_limits<double>::max();
            auto bestMatch_veto = vetoEventsTMatch.end();
            double bestDtime_side = numeric_limits<double>::max();
            auto bestMatch_side = sideEventsTMatch.end();
            double tof = 0;
            double veto = 0;
            double side = 0;
            int max_gen = 0;
            int num_side = -1;
            double qdc1 = -9999;
            double qdc2 = -9999;

            /** Calculating the Timming Target**/
            if(time >= last_reset)
                time_t = time - last_reset;
            else
                time_t = time - beforelast_reset;

            /** Searching for MCP1, MPC2, Veto and Side detector event in correlation with DSSD events within th define timming gates**/
            for (vector<pair<StripEvent, bool>>::iterator itm = mcp1EventsTMatch.begin(); //*looking for the best MCP1 based on timming only
                itm != mcp1EventsTMatch.end(); ++itm) {
                if((*itm).second)
                    continue;
                double dTime = abs(time - (*itm).first.t);
                if (dTime < abs(time - bestDtime_mcp1) ){
                    bestDtime_mcp1 =(*itm).first.t; 
                    bestMatch_mcp1 = itm; 
                    bestQDC_mcp1 = (*itm).first.Qdc;
                }
            }
            for (vector<pair<StripEvent, bool>>::iterator itm = mcp2EventsTMatch.begin(); //*looking for the best MCP2 based on timming only
                itm != mcp2EventsTMatch.end(); ++itm){
                if((*itm).second)
                    continue;
                double dTime = abs(time - (*itm).first.t);
                if (dTime < abs(time - bestDtime_mcp2)){
                    bestDtime_mcp2 = (*itm).first.t;
                    bestMatch_mcp2 = itm;                   
                    bestQDC_mcp2 = (*itm).first.Qdc;
                }
            }
            for (vector<pair<StripEvent, bool>>::iterator itv = vetoEventsTMatch.begin(); //*looking for the best veto based on timming only
                itv != vetoEventsTMatch.end(); ++itv){
                if((*itv).second)
                    continue;
                double dTime = abs(time - (*itv).first.t);
                if (dTime < abs(bestDtime_veto))
                    bestDtime_veto = time - (*itv).first.t; bestMatch_veto = itv;
            }
            for (vector<pair<StripEvent, bool>>::iterator its = sideEventsTMatch.begin(); //*looking for the best Side based on timming only
                its != sideEventsTMatch.end(); ++its){
                if((*its).second)
                    continue;
                double dTime = abs(time - (*its).first.t);
                if (dTime < abs(bestDtime_side))
                    bestDtime_side = time - (*its).first.t; bestMatch_side = its;
            }
            /** Timming correlation for TOF, Veto and Side detector with the DSSD for construction of a focale plane event if possible : timming gate define in XML file**/
            if (abs(time - bestDtime_mcp1) < tofWindow_ &&  abs(time - bestDtime_mcp2) < tofWindow_){
                tof = (bestDtime_mcp1-bestDtime_mcp2);
                corEvent.type = EventInfo::IMPLANT_EVENT;
                corEvent.energy = yEnergy;
                max_gen = corr.GetGen(xPosition, yPosition);
                plot(D_GEN,max_gen);
                (*bestMatch_mcp1).second = true;
                (*bestMatch_mcp2).second = true;
                if(!bestQDC_mcp1.empty()){
                    switch (nshe_exp_){
                        case 5:
                            qdc1 = bestQDC_mcp1.at(6) - 13*bestQDC_mcp1.at(7)/16;
                            break;
                         case 8 : 
                                qdc1 = bestQDC_mcp1.at(7) - bestQDC_mcp1.at(6);
                                break;
                        default:
                            qdc1 = bestQDC_mcp1.at(6) - bestQDC_mcp1.at(7);
                            break;
                    }
                    plot(DD_QDC1_DSSD,yEnergy/20,qdc1);
                    plot(DD_QDC1_TOF,tof + 600,qdc1);
                }
                if(!bestQDC_mcp2.empty()){
                    switch (nshe_exp_){
                        case 5:
                            qdc2 = bestQDC_mcp2.at(6) - 13*bestQDC_mcp2.at(7)/16;
                            break;
                         case 8 : 
                                qdc2 = bestQDC_mcp2.at(7) - bestQDC_mcp2.at(6);
                                break;
                        default:
                            qdc2 = bestQDC_mcp2.at(6) - bestQDC_mcp2.at(7);
                            break;
                    }
                    plot(DD_QDC2_DSSD,yEnergy/20,qdc2);
                    plot(DD_QDC2_TOF,tof + 600,qdc2);
                }
                plot(DD_TOF_ENERGY, tof + 600,yEnergy/20);
                plot(D_DTIMETOF, tof);
            }	 	
            else{
                if(abs(time - bestDtime_mcp1) < tofWindow_) {
                    tof = (time - bestDtime_mcp1) + 400;
                    corEvent.type = EventInfo::IMPLANT_EVENT;
                    corEvent.energy = yEnergy;
                    max_gen = corr.GetGen(xPosition, yPosition);
                    (*bestMatch_mcp1).second = true;
                    if(!bestQDC_mcp1.empty()){
                        switch (nshe_exp_){
                            case 5:
                                qdc1 = bestQDC_mcp1.at(6) - 13*bestQDC_mcp1.at(7)/16;
                                break;
                            case 8 : 
                                qdc1 = bestQDC_mcp1.at(7) - bestQDC_mcp1.at(6);
                                break;
                            default:
                                qdc1 = bestQDC_mcp1.at(6) - bestQDC_mcp1.at(7);
                                break;
                        }
                        plot(DD_QDC1_DSSD,yEnergy/20,qdc1/20);
                        plot(DD_QDC1_TOF,tof ,qdc1/20);
                    }
                    plot(DD_TOF1_DSSD, tof ,yEnergy/20);
                } 
                else if(abs(time - bestDtime_mcp2) < tofWindow_){
                    tof = (time - bestDtime_mcp2) + 800;
                    corEvent.type = EventInfo::IMPLANT_EVENT;
                    corEvent.energy = yEnergy;
                    max_gen = corr.GetGen(xPosition, yPosition);
                    plot(D_GEN,max_gen);
                    (*bestMatch_mcp2).second = true;
                    if(!bestQDC_mcp2.empty()){
                        switch (nshe_exp_){
                            case 5:
                                qdc2 = bestQDC_mcp2.at(6) - 13*bestQDC_mcp2.at(7)/16;
                                break;
                             case 8 :
                                qdc2 = bestQDC_mcp2.at(7) - bestQDC_mcp2.at(6);
                                break;
                            default:
                                qdc2 = bestQDC_mcp2.at(6) - bestQDC_mcp2.at(7);
                                break;
                        }
                        plot(DD_QDC2_DSSD,yEnergy/20,qdc2/20);
                        plot(DD_QDC2_TOF,tof + 600,qdc2/20);
                    }
                    plot(DD_TOF2_DSSD, tof,yEnergy/20);
                }
                else {
                    tof = 0;
                    corEvent.type = EventInfo::DECAY_EVENT;
                    corEvent.energy = xEnergy;
                }
            }
            if (bestDtime_veto < vetoWindow_) {
                plot(DD_VETO_ENERGY, (*bestMatch_veto).first.E,yEnergy/100);
                plot(D_DTIMEVETO, bestDtime_veto);
                veto = (*bestMatch_veto).first.E;
                (*bestMatch_veto).second = true;
            }
            if (abs(bestDtime_side) < tofWindow_) {
                plot(DD_DSSD_TUNNEL, xEnergy/10, (*bestMatch_side).first.E/10);
                side = (*bestMatch_side).first.E;
                num_side = (*bestMatch_side).first.pos;
                (*bestMatch_side).second = true;
                plot(DD_DENERGY_SIDE_NUM,num_side,side);
                plot(D_DTIME_SIDE, int(bestDtime_side) + 4000);
                plot(DD_SIDE_N, num_side,side/10);
            }
            /** Correlator only for quick inspection of the restults online or to check that everything is good using DAMM histo : DO NOT PUT DECAY CORRELATION INTO TTREE**/
            corEvent.time = time;
            corr.Correlate(corEvent, xPosition, yPosition);

            if (corr.GetCondition() == Correlator::VALID_DECAY) {
                double Tlog = log2(((corr.GetDecayGenTime(xPosition, yPosition))))*10;
                plot(DD_KHS, xEnergy/10, Tlog);
                if (Tlog > khs_limit_cor_*10 )
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
            /** Save event into the TTree with only Front/Back, TOF, Veto, Side (if any) timming correlation (define in xml for gate) : no other correlation doen **/
            if (DetectorDriver::get()->GetSysRootOutput()){
                //Fill Root struct
                dssdstruc.x = xPosition;
                dssdstruc.y = yPosition;
                dssdstruc.Energy_front = xEnergy;
                dssdstruc.Energy_back = yEnergy;
                dssdstruc.Time = time;
                dssdstruc.Time_t = time_t;
                dssdstruc.Tof = tof;
                if(abs(time - bestDtime_mcp1) < tofWindow_) dssdstruc.TMCP1 = (time - bestDtime_mcp1) ;
                if(abs(time - bestDtime_mcp2)  < tofWindow_) dssdstruc.TMCP2 = (time - bestDtime_mcp2) ;
                if(!bestQDC_mcp1.empty()) dssdstruc.QDC1 = qdc1;
                if(!bestQDC_mcp2.empty()) dssdstruc.QDC2 = qdc2;
                if(bestDtime_veto < vetoWindow_) {
                    if((*bestMatch_veto).first.pileup) dssdstruc.Trace_Veto = (*bestMatch_veto).first.trace;
                    dssdstruc.Veto = (*bestMatch_veto).first.E;
                }
                dssdstruc.Trace_Front = (*it).first.trace;
                dssdstruc.Trace_Back = (*it).second.trace;
                dssdstruc.pile_up = (*it).second.pileup; 
                if(bestDtime_mcp1 < tofWindow_) dssdstruc.Trace_TOF1 = (*bestMatch_mcp1).first.trace;
                if(bestDtime_mcp2 < tofWindow_) dssdstruc.Trace_TOF2 = (*bestMatch_mcp2).first.trace;
                if(abs(bestDtime_side) < vetoWindow_) {
                    // if((*bestMatch_side).first.pileup) dssdstruc.Trace_Side = (*bestMatch_side).first.trace;
                    dssdstruc.Side = side;
                    dssdstruc.NSide = num_side;
                }
                pixie_tree_event_->dssd_vec_.emplace_back(dssdstruc);
                dssdstruc = processor_struct::DSSD_DEFAULT_STRUCT;
            }
            plot(D_ENERGY_X, xEnergy/10);
            plot(D_ENERGY_Y, yEnergy/10);
            plot(DD_FRONTE__BACKE, xEnergy/100, yEnergy/100);
            plot(DD_EVENT_ENERGY__X_POSITION, xEnergy/10, xPosition);
            plot(DD_EVENT_ENERGY__Y_POSITION, yEnergy/100, yPosition);
            plot(DD_EVENT_POSITION, xPosition, yPosition);
            if(xEnergy/10 > 500 && xEnergy/10 < 1200 && xPosition < 32){
                if(yPosition < 16) 
                    plot(DD_EVENT_POSITION_FROM_E, xPosition, yPosition);
                else
                    plot(DD_EVENT_POSITION_FROM_E, xPosition, yPosition-16);
            }
            else if(xEnergy/10 > 500 && xEnergy/10 < 1200 && xPosition > 31){
                if(yPosition < 16) 
                    plot(DD_EVENT_POSITION_FROM_E, xPosition, yPosition);
                else
                    plot(DD_EVENT_POSITION_FROM_E, xPosition, yPosition-16);
            }
        }
        /** Side Only Event : case of an escape particle leaving energy below threshold in the DSSD **/
        for (vector<pair<StripEvent, bool>>::iterator its = sideEventsTMatch.begin();
            its != sideEventsTMatch.end(); ++its){
            if((*its).second)
                continue;
            if (DetectorDriver::get()->GetSysRootOutput()){
            //Fill Root struct
                dssdstruc.x = -1;
                dssdstruc.y = -1;
                dssdstruc.Energy_front = 0;
                dssdstruc.Energy_back = 0;
                dssdstruc.Time = (*its).first.t;
                dssdstruc.Time_t = 0;
                dssdstruc.Tof = 0;
                dssdstruc.TMCP1 = -999 ;
                dssdstruc.TMCP2 = -999 ;
                dssdstruc.QDC1 = -999;
                dssdstruc.QDC2 = -999;
                dssdstruc.Trace_Veto.clear();
                dssdstruc.Veto = 0;
                dssdstruc.Trace_Front.clear();
                dssdstruc.Trace_Back.clear();
                dssdstruc.pile_up = (*its).first.pileup; 
                dssdstruc.Trace_TOF1.clear();
                dssdstruc.Trace_TOF2.clear();
                dssdstruc.Side = (*its).first.E;
                dssdstruc.NSide = (*its).first.pos;
                pixie_tree_event_->dssd_vec_.emplace_back(dssdstruc);
                dssdstruc = processor_struct::DSSD_DEFAULT_STRUCT;
            }
        }
    }
    else{ /** In case we want to put single even without any correlation in the root file (calibration or check efficiency selection scannor**/
        for (vector<pair<StripEvent, bool>>::iterator it = xEventsTMatch.begin();
            it != xEventsTMatch.end(); ++it) {
            if (DetectorDriver::get()->GetSysRootOutput()){
            //Fill Root struct
                dssdstruc.x = (*it).first.pos;
                dssdstruc.y = -1;
                dssdstruc.Energy_front = (*it).first.E;
                dssdstruc.Energy_back = 0;
                dssdstruc.Time = (*it).first.t;

                if((*it).first.t >= last_reset)
                    dssdstruc.Time_t = (*it).first.t - last_reset;
                else
                    dssdstruc.Time_t = (*it).first.t - beforelast_reset;

                dssdstruc.Tof = 0;
                dssdstruc.TMCP1 = -999 ;
                dssdstruc.TMCP2 = -999 ;
                dssdstruc.QDC1 = -999;
                dssdstruc.QDC2 = -999;
                dssdstruc.Trace_Veto.clear();
                dssdstruc.Veto = 0;
                dssdstruc.Trace_Front = (*it).first.trace;
                dssdstruc.Trace_Back.clear();
                dssdstruc.pile_up = (*it).first.pileup; 
                dssdstruc.Trace_TOF1.clear();
                dssdstruc.Trace_TOF2.clear();
                dssdstruc.Side = 0;
                dssdstruc.NSide = -1;
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
                if((*it).first.t >= last_reset)
                    dssdstruc.Time_t = (*it).first.t - last_reset;
                else
                    dssdstruc.Time_t = (*it).first.t - beforelast_reset;
                dssdstruc.Tof = 0;
                dssdstruc.TMCP1 = -999 ;
                dssdstruc.TMCP2 = -999 ;
                dssdstruc.QDC1 = -999;
                dssdstruc.QDC2 = -999;
                dssdstruc.Trace_Veto.clear();
                dssdstruc.Veto = 0;
                dssdstruc.Trace_Front.clear();
                dssdstruc.Trace_Back = (*it).first.trace;
                dssdstruc.pile_up = (*it).first.pileup; 
                dssdstruc.Trace_TOF1.clear();
                dssdstruc.Trace_TOF2.clear();
                dssdstruc.Side = 0;
                dssdstruc.NSide = -1;
                pixie_tree_event_->dssd_vec_.emplace_back(dssdstruc);
                dssdstruc = processor_struct::DSSD_DEFAULT_STRUCT;
            }
        }
        for (vector<pair<StripEvent, bool>>::iterator its = sideEventsTMatch.begin();
            its != sideEventsTMatch.end(); ++its){
            if (DetectorDriver::get()->GetSysRootOutput()){
            //Fill Root struct
                dssdstruc.x = -1;
                dssdstruc.y = -1;
                dssdstruc.Energy_front = 0;
                dssdstruc.Energy_back = 0;
                dssdstruc.Time = (*its).first.t;
                dssdstruc.Time_t = 0;
                dssdstruc.Tof = 0;
                dssdstruc.TMCP1 = -999 ;
                dssdstruc.TMCP2 = -999 ;
                dssdstruc.QDC1 = -999;
                dssdstruc.QDC2 = -999;
                dssdstruc.Trace_Veto.clear();
                dssdstruc.Veto = 0;
                dssdstruc.Trace_Front.clear();
                dssdstruc.Trace_Back.clear();
                dssdstruc.pile_up = (*its).first.pileup; 
                dssdstruc.Trace_TOF1.clear();
                dssdstruc.Trace_TOF2.clear();
                dssdstruc.Side = (*its).first.E;
                dssdstruc.NSide = (*its).first.pos;
                pixie_tree_event_->dssd_vec_.emplace_back(dssdstruc);
                dssdstruc = processor_struct::DSSD_DEFAULT_STRUCT;
            }
        }
        for (vector<pair<StripEvent, bool>>::iterator it = mcp1EventsTMatch.begin();
            it != mcp1EventsTMatch.end(); ++it) {
            if (DetectorDriver::get()->GetSysRootOutput()){
                dssdstruc.x = -1;
                dssdstruc.y = -1;
                dssdstruc.Energy_front = 0;
                dssdstruc.Energy_back = 0;
                dssdstruc.Time = (*it).first.t;
                dssdstruc.Time_t = 0;
                dssdstruc.Tof = -999;
                dssdstruc.TMCP1 = -999 ;
                dssdstruc.TMCP2 = -999 ;
                dssdstruc.QDC1 = -999;
                dssdstruc.QDC2 = -999;
                dssdstruc.Trace_Veto.clear();
                dssdstruc.Veto = 0;
                dssdstruc.Trace_Front.clear();
                dssdstruc.Trace_Back.clear();
                dssdstruc.pile_up = (*it).first.pileup; 
                if(!(*it).first.trace.empty()) dssdstruc.Trace_TOF1 = (*it).first.trace;
                dssdstruc.Trace_TOF2.clear();
                dssdstruc.Side = 0;
                dssdstruc.NSide = -1;
                pixie_tree_event_->dssd_vec_.emplace_back(dssdstruc);
                dssdstruc = processor_struct::DSSD_DEFAULT_STRUCT;
            }
        }
        for (vector<pair<StripEvent, bool>>::iterator it = mcp2EventsTMatch.begin();
            it != mcp2EventsTMatch.end(); ++it) {
            if (DetectorDriver::get()->GetSysRootOutput()){
                dssdstruc.x = -1;
                dssdstruc.y = -1;
                dssdstruc.Energy_front = 0;
                dssdstruc.Energy_back = 0;
                dssdstruc.Time = (*it).first.t;
                dssdstruc.Time_t = 0;
                dssdstruc.Tof = -999;
                dssdstruc.TMCP1 = -999 ;
                dssdstruc.TMCP2 = -999 ;
                dssdstruc.QDC1 = -999;
                dssdstruc.QDC2 = -999;
                dssdstruc.Trace_Veto.clear();
                dssdstruc.Veto = 0;
                dssdstruc.Trace_Front.clear();
                dssdstruc.Trace_Back.clear();
                dssdstruc.pile_up = (*it).first.pileup; 
                if(!(*it).first.trace.empty()) dssdstruc.Trace_TOF2 = (*it).first.trace;
                dssdstruc.Trace_TOF1.clear();
                dssdstruc.Side = 0;
                dssdstruc.NSide = -1;
                pixie_tree_event_->dssd_vec_.emplace_back(dssdstruc);
                dssdstruc = processor_struct::DSSD_DEFAULT_STRUCT;
            }
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
                dssdstruc.Time_t = 0;
                dssdstruc.Tof = 0;
                dssdstruc.TMCP1 = -999;
                dssdstruc.TMCP2 = -999;
                dssdstruc.QDC1 = -999;
                dssdstruc.QDC2 = -999;
                dssdstruc.Trace_Veto.clear();
                dssdstruc.Veto = (*it).first.E;
                dssdstruc.Trace_Front.clear();
                dssdstruc.Trace_Back.clear();
                dssdstruc.pile_up = (*it).first.pileup; 
                dssdstruc.Trace_TOF1.clear();
                dssdstruc.Trace_TOF2.clear();
                dssdstruc.Side = 0;
                dssdstruc.NSide = -1;
                pixie_tree_event_->dssd_vec_.emplace_back(dssdstruc);
                dssdstruc = processor_struct::DSSD_DEFAULT_STRUCT;
            }
        }
    }
    /** Elastic event stored in a another TTree ans always decoupled from the Focal plane detectors and its correlations **/
    vector<ChanEvent *> elaEvents = event.GetSummary("SHE:Elastic", true)->GetList();
    for (vector<ChanEvent *>::iterator itx = elaEvents.begin();
        itx != elaEvents.end(); ++itx) {	
        plot(D_ELA_TOT,(*itx)->GetCalibratedEnergy());
        double time_target = 0;
        if((*itx)->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9 >= last_reset)
            time_target = (*itx)->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9 - last_reset;
        else
            time_target = (*itx)->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9 - beforelast_reset;

        plot(DD_ELA_TARGET,(time_target/10),(*itx)->GetCalibratedEnergy());
        if (DetectorDriver::get()->GetSysRootOutput()){
        //Fill Root struct
            elastruc.Energy = (*itx)->GetCalibratedEnergy();
            elastruc.Time = (*itx)->GetTime() * Globals::get()->GetAdcClockInSeconds()*1e+9;
            elastruc.Time_t = time_target;
            pixie_tree_event_->she_ela_vec_.emplace_back(elastruc);
            elastruc = processor_struct::SHE_ELA_DEFAULT_STRUCT;
        }
    }

    EndProcess();
    return true;
}