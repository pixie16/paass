///@file PspmtProcessor.cpp
///@Processes information from a Position Sensitive PMT.  No Pixel work yet. 
///@author A. Keeler, S. Go, S. V. Paulauskas 
///@date July 8, 2018

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <signal.h>
#include <limits.h>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "PspmtProcessor.hpp"
#include "Globals.hpp"
#include "Messenger.hpp"

using namespace std;
using namespace dammIds::pspmt;

namespace dammIds {
    namespace pspmt {
        const int DD_DYNODE_QDC = 0;
        const int DD_POS_LOW = 1;
        const int DD_POS_HIGH = 2;
        const int DD_PLASTIC_EN = 3;
    }
}

void PspmtProcessor::DeclarePlots(void) {
    DeclareHistogram2D(DD_DYNODE_QDC, SD, S2, "Dynode QDC- Low gain 0, High gain 1");
    DeclareHistogram2D(DD_POS_LOW, SB, SB, "Low-gain Positions");
    DeclareHistogram2D(DD_POS_HIGH, SB, SB, "High-gain Positions");
    DeclareHistogram2D(DD_PLASTIC_EN,SD,S4, "Plastic Energy, 0-3 = VETO, 5-8 = Ion Trigger");
}

PspmtProcessor::PspmtProcessor(const std::string &vd, const double &scale, const unsigned int &offset,
                               const double &threshold) :EventProcessor(OFFSET, RANGE, "PspmtProcessor"){


    if(vd == "SIB064_1018" || vd == "SIB064_1730")
        vdtype_ = corners;
    else if(vd == "SIB064_0926")
        vdtype_ = sides;
    else
        vdtype_ = UNKNOWN;

    VDtypeStr = vd;
    positionScale_ = scale;
    positionOffset_ = offset;
    threshold_ = threshold;
    ThreshStr = threshold;
    associatedTypes.insert("pspmt");
}

bool PspmtProcessor::PreProcess(RawEvent &event){

    if (!EventProcessor::PreProcess(event))
        return false;

    if (DetectorDriver::get()->GetSysRootOutput()) {
        PSstruct = processor_struct::PSPMT_DEFAULT_STRUCT;
    }

    //read in anode & dynode signals
    static const vector<ChanEvent *> &hiDynode = event.GetSummary("pspmt:dynode_high")->GetList();
    static const vector<ChanEvent *> &lowDynode = event.GetSummary("pspmt:dynode_low")->GetList();
    static const vector<ChanEvent *> &hiAnode = event.GetSummary("pspmt:anode_high")->GetList();
    static const vector<ChanEvent *> &lowAnode =  event.GetSummary("pspmt:anode_low")->GetList();

    static const vector<ChanEvent *> &veto =  event.GetSummary("pspmt:veto")->GetList();
    static const vector<ChanEvent *> &ionTrig=  event.GetSummary("pspmt:ion")->GetList();

    //Plot Dynode QDCs
    for(vector<ChanEvent *>::const_iterator it = lowDynode.begin(); it != lowDynode.end(); it++){
        plot(DD_DYNODE_QDC, (*it)->GetTrace().GetQdc(), 0);
    }
    for(vector<ChanEvent *>::const_iterator it = hiDynode.begin(); it != hiDynode.end(); it++){
        plot(DD_DYNODE_QDC, (*it)->GetTrace().GetQdc(), 1);
    }

    int numOfVetoChans = (int)(DetectorLibrary::get()->GetLocations("pspmt","veto")).size();
    std::map<int,double> vetoEnergys,IonTrigEnergies;
    for (auto it = veto.begin(); it != veto.end(); it++ ){
        int loc =  (*it)->GetChanID().GetLocation();

        plot(DD_PLASTIC_EN,(*it)->GetCalibratedEnergy(),loc);

        if (vetoEnergys.find(loc) != vetoEnergys.end()) {
            vetoEnergys.find(loc)->second = (*it)->GetCalibratedEnergy();
        }else {
            vetoEnergys.emplace(loc,(*it)->GetCalibratedEnergy());
        }

    }
    for (auto it = ionTrig.begin(); it != ionTrig.end(); it++) {
        int loc =  (*it)->GetChanID().GetLocation();
        plot(DD_PLASTIC_EN, (*it)->GetCalibratedEnergy(), loc + numOfVetoChans + 1); //max veto chan +1 for readablility

        if (IonTrigEnergies.find(loc) != IonTrigEnergies.end()) {
            IonTrigEnergies.find(loc)->second = (*it)->GetCalibratedEnergy();
        }else {
            IonTrigEnergies.emplace(loc,(*it)->GetCalibratedEnergy());
        }
    }
    //set up position calculation for low and high gain signals
    position_low.first = 0, position_low.second = 0;
    position_high.first = 0, position_high.second = 0;
    double energy = 0;
    double xa_l = 0, ya_l = 0, xb_l = 0, yb_l = 0;
    double xa_h = 0, ya_h = 0, xb_h = 0, yb_h = 0;

    for(vector<ChanEvent *>::const_iterator it = lowAnode.begin(); it != lowAnode.end(); it++){
        //check signals energy vs threshold
        energy = (*it)->GetCalibratedEnergy();
        if (energy < threshold_)
            continue;
        //parcel out position signals by tag
        if ((*it)->GetChanID().HasTag("xa") && xa_l == 0 )
            xa_l = energy;
        if ((*it)->GetChanID().HasTag("xb") && xb_l == 0 )
            xb_l = energy;
        if ((*it)->GetChanID().HasTag("ya") && ya_l == 0 )
            ya_l = energy;
        if ((*it)->GetChanID().HasTag("yb") && yb_l == 0 )
            yb_l = energy;
    }

    for(vector<ChanEvent *>::const_iterator it = hiAnode.begin();
        it != hiAnode.end(); it++){
        //check signals energy vs threshold
        energy = (*it)->GetCalibratedEnergy();
        if (energy < threshold_ || energy > 63000)
            continue;
        //parcel out position signals by tag
        if ((*it)->GetChanID().HasTag("xa") && xa_h == 0)
            xa_h = energy;
        if ((*it)->GetChanID().HasTag("xb") && xb_h == 0)
            xb_h = energy;
        if ((*it)->GetChanID().HasTag("ya") && ya_h == 0)
            ya_h = energy;
        if ((*it)->GetChanID().HasTag("yb") && yb_h == 0)
            yb_h = energy;
    }

    if (xa_l > 0 && xb_l > 0 && ya_l > 0 && yb_l > 0){
        position_low.first = CalculatePosition(xa_l, xb_l, ya_l, yb_l, vdtype_).first;
        position_low.second  = CalculatePosition(xa_l, xb_l, ya_l, yb_l, vdtype_).second;
        plot(DD_POS_LOW, position_low.first * positionScale_ + positionOffset_,
             position_low.second * positionScale_ + positionOffset_);
    }

    if (xa_h > 0 && xb_h > 0 && ya_h > 0 && yb_h > 0){
        position_high.first = CalculatePosition(xa_h, xb_h, ya_h, yb_h, vdtype_).first;
        position_high.second = CalculatePosition(xa_h, xb_h, ya_h, yb_h, vdtype_).second;
        plot(DD_POS_HIGH, position_high.first * positionScale_ + positionOffset_,
             position_high.second * positionScale_ + positionOffset_);
    }

    if (DetectorDriver::get()->GetSysRootOutput()) {

        PSstruct.xa_l = xa_l;
        PSstruct.xb_l = xb_l;
        PSstruct.ya_l = ya_l;
        PSstruct.yb_l = yb_l;
        PSstruct.xa_h = xa_h;
        PSstruct.xb_h = xb_h;
        PSstruct.ya_h = ya_h;
        PSstruct.yb_h = yb_h;
        if (!lowDynode.empty()) {
            PSstruct.dy_l = lowDynode.front()->GetCalibratedEnergy();
            PSstruct.dyL_time = lowDynode.front()->GetTimeSansCfd();
        }
        if (!hiDynode.empty()) {
            PSstruct.dy_h = hiDynode.front()->GetCalibratedEnergy();
            PSstruct.dyH_time = hiDynode.front()->GetTimeSansCfd();
        }
        PSstruct.anodeLmulti = (int) lowAnode.size();
        PSstruct.anodeHmulti = (int) hiAnode.size();
        PSstruct.dyLmulti = (int) lowDynode.size();
        PSstruct.dyHmulti = (int) hiDynode.size();
        PSstruct.xposL = position_low.first;
        PSstruct.yposL = position_low.second;
        PSstruct.xposH = position_high.first;
        PSstruct.yposH = position_high.second;

        PSstruct.vetoEn0 = vetoEnergys.find(0)->second;
        PSstruct.vetoEn1 = vetoEnergys.find(1)->second;

        PSstruct.ionTrigEn0 = IonTrigEnergies.find(0)->second;
        PSstruct.ionTrigEn1 = IonTrigEnergies.find(1)->second;
        PSstruct.ionTrigEn2 = IonTrigEnergies.find(2)->second;
        PSstruct.ionTrigEn3 = IonTrigEnergies.find(3)->second;

        pixie_tree_event_->pspmt_vec_.emplace_back(PSstruct);
        PSstruct = processor_struct::PSPMT_DEFAULT_STRUCT;
    }
    EndProcess();
    return (true);

}

pair<double, double> PspmtProcessor::CalculatePosition(double &xa, double &xb, double &ya, double &yb, const VDTYPES &vdtype){

    double x = 0, y = 0;

    switch(vdtype){
        case corners:
            x = (0.5 * (yb + xa)) / (xa + xb + ya + yb);
            y = (0.5 * (xa + xb)) / (xa + xb + ya + yb);
            break;
        case sides:
            x = (xa - xb) / (xa + xb);
            y = (ya - yb) / (ya + yb);
            break;
        case UNKNOWN:
        default:
            cerr<<"We recieved a VD_TYPE we didn't recognize " << vdtype << endl;

    }
    return make_pair(x, y);
}
