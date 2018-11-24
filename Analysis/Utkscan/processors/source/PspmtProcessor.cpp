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
#include <cmath>

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
        const int DD_MULTI = 4;
        const int DD_DY_SUM_LG =5;
        const int DD_DY_SUM_HG =6;

        const int D_TRANS_EFF_YSO = 10;
        const int DD_SEPAR_GATED_LOW = 11;
        const int DD_DESI_GATED_LOW = 12;
        const int D_DESI_ENERGY = 15;
        const int D_DESI_YSO_GATED = 16;
        const int DD_SEPAR_ENERGY = 17;
        const int DD_SEPAR_YSO_GATED = 18;

        const int DD_POS_ION = 20;
        const int DD_SEPAR_GATED_ION = 21;
        const int DD_DESI_GATED_ION = 22;
    }
}

void PspmtProcessor::DeclarePlots(void) {
    DeclareHistogram2D(DD_DYNODE_QDC, SD, S2, "Dynode QDC- Low gain 0, High gain 1");
    DeclareHistogram2D(DD_POS_LOW, SB, SB, "Low-gain Positions");
    DeclareHistogram2D(DD_POS_HIGH, SB, SB, "High-gain Positions");
    DeclareHistogram2D(DD_PLASTIC_EN,SD,S4, "Plastic Energy, 0-3 = VETO, 5-8 = Ion Trigger");
    DeclareHistogram2D(DD_MULTI,S3,S3, "Dynode:Anode(+2) Multi Low gain 0, High gain 1");
    DeclareHistogram2D(DD_DY_SUM_LG,SA,SA,"Low Gain Dynode vs Anode Sum");
    DeclareHistogram2D(DD_DY_SUM_HG,SA,SA,"High Gain Dynode vs Anode Sum");
    DeclareHistogram1D(D_TRANS_EFF_YSO, S3, "Separator events (0) in ion scint (1), YSO (2), and veto (3)");
    DeclareHistogram2D(DD_SEPAR_GATED_LOW, SB, SB, "Separator-gated low-gain positions");
    DeclareHistogram2D(DD_DESI_GATED_LOW, SB, SB, "Silicon dE-gated low-gain positions");
    DeclareHistogram1D(D_DESI_ENERGY, SC, "Separator-gated dE-silicon events");
    DeclareHistogram1D(D_DESI_YSO_GATED, SC, "YSO-gated dE-silicon");
    DeclareHistogram2D(DD_SEPAR_ENERGY, S2, SC, "dE-silicon-gated separator events");
    DeclareHistogram2D(DD_SEPAR_YSO_GATED, S2, SC, "YSO-gated separator events");

    DeclareHistogram2D(DD_POS_ION, SB, SB, "Ion-scint positions - ungated");
    DeclareHistogram2D(DD_SEPAR_GATED_ION, SB, SB, "Ion-scint positions - separator-gated");
    DeclareHistogram2D(DD_DESI_GATED_ION, SB, SB, "Ion-scint positions - silicon dE-gated");
}

PspmtProcessor::PspmtProcessor(const std::string &vd, const double &yso_scale, const unsigned int &yso_offset,
			       const double &yso_threshold, const double &front_scale,
			       const unsigned int &front_offset, const double &front_threshold, const double &rotation)
				:EventProcessor(OFFSET, RANGE, "PspmtProcessor"){


    if(vd == "SIB064_1018" || vd == "SIB064_1730")
        vdtype_ = corners;
    else if(vd == "SIB064_0926")
        vdtype_ = sides;
    else
        vdtype_ = UNKNOWN;

    VDtypeStr = vd;
    positionScale_ = yso_scale;
    positionOffset_ = yso_offset;
    threshold_ = yso_threshold;
    front_positionScale_ = front_scale;
    front_positionOffset_ = front_offset;
    front_threshold_ = front_threshold;
    ThreshStr = yso_threshold;
    rotation_ = rotation * 3.1415926 / 180;     // convert from degrees to radians
    associatedTypes.insert("pspmt");
    associatedTypes.insert("generic");
}

bool PspmtProcessor::PreProcess(RawEvent &event) {

    if (!EventProcessor::PreProcess(event))
        return false;

    if (DetectorDriver::get()->GetSysRootOutput()) {
        PSstruct = processor_struct::PSPMT_DEFAULT_STRUCT;
    }

    //read in anode & dynode signals
    static const vector<ChanEvent *> &hiDynode = event.GetSummary("pspmt:dynode_high")->GetList();
    static const vector<ChanEvent *> &lowDynode = event.GetSummary("pspmt:dynode_low")->GetList();
    static const vector<ChanEvent *> &hiAnode = event.GetSummary("pspmt:anode_high")->GetList();
    static const vector<ChanEvent *> &lowAnode = event.GetSummary("pspmt:anode_low")->GetList();

    static const vector<ChanEvent *> &veto = event.GetSummary("pspmt:veto")->GetList();
    static const vector<ChanEvent *> &ionTrig = event.GetSummary("pspmt:ion")->GetList();
    static const vector<ChanEvent *> &desi = event.GetSummary("generic:desi")->GetList();
    static const vector<ChanEvent *> &separatorScint = event.GetSummary("generic:f11")->GetList();

    //Plot Dynode QDCs
    for (vector<ChanEvent *>::const_iterator it = lowDynode.begin(); it != lowDynode.end(); it++) {
        plot(DD_DYNODE_QDC, (*it)->GetTrace().GetQdc(), 0);
    }
    for (vector<ChanEvent *>::const_iterator it = hiDynode.begin(); it != hiDynode.end(); it++) {
        plot(DD_DYNODE_QDC, (*it)->GetTrace().GetQdc(), 1);
    }

    int numOfVetoChans = (int) (DetectorLibrary::get()->GetLocations("pspmt", "veto")).size();
    std::map<int, double> vetoEnergys, IonTrigEnergies;
    for (auto it = veto.begin(); it != veto.end(); it++) {
        int loc = (*it)->GetChanID().GetLocation();

        plot(DD_PLASTIC_EN, (*it)->GetCalibratedEnergy(), loc);

        if (vetoEnergys.find(loc) != vetoEnergys.end()) {
            vetoEnergys.find(loc)->second = (*it)->GetCalibratedEnergy();
        } else {
            vetoEnergys.emplace(loc, (*it)->GetCalibratedEnergy());
        }

    }
    for (auto it = ionTrig.begin(); it != ionTrig.end(); it++) {
        int loc = (*it)->GetChanID().GetLocation();
        plot(DD_PLASTIC_EN, (*it)->GetCalibratedEnergy(), loc + numOfVetoChans + 1); //max veto chan +1 for readablility

        if (IonTrigEnergies.find(loc) != IonTrigEnergies.end()) {
            IonTrigEnergies.find(loc)->second = (*it)->GetCalibratedEnergy();
        } else {
            IonTrigEnergies.emplace(loc, (*it)->GetCalibratedEnergy());
        }
    }
    //set up position calculation for low / high gain yso signals and ion scint
    position_low.first = 0, position_low.second = 0;
    position_high.first = 0, position_high.second = 0;
    double energy = 0;
    double xa_l = 0, ya_l = 0, xb_l = 0, yb_l = 0;
    double xa_h = 0, ya_h = 0, xb_h = 0, yb_h = 0;
    double top_l = 0, top_r = 0, bottom_l = 0, bottom_r = 0;
    bool hasPosition_low = false, hasPosition_high = false;
    bool hasPosition_ion = false;

    plot(DD_MULTI, lowDynode.size(), 0);
    plot(DD_MULTI, hiDynode.size(), 1);

    plot(DD_MULTI, lowAnode.size(), 2);
    plot(DD_MULTI, hiAnode.size(), 3);


    double lowAnodeSum = 0;
    for (vector<ChanEvent *>::const_iterator it = lowAnode.begin(); it != lowAnode.end(); it++) {
        //check signals energy vs threshold
        energy = (*it)->GetCalibratedEnergy();
        if (energy < threshold_)
            continue;
        //parcel out position signals by tag
        if ((*it)->GetChanID().HasTag("xa") && xa_l == 0) {
            xa_l = energy;
            lowAnodeSum += energy;
        }
        if ((*it)->GetChanID().HasTag("xb") && xb_l == 0) {
            xb_l = energy;
            lowAnodeSum += energy;
        }
        if ((*it)->GetChanID().HasTag("ya") && ya_l == 0) {
            ya_l = energy;
            lowAnodeSum += energy;
        }
        if ((*it)->GetChanID().HasTag("yb") && yb_l == 0) {
            yb_l = energy;
            lowAnodeSum += energy;
        }
    }


    double highAnodeSum = 0;
    for (vector<ChanEvent *>::const_iterator it = hiAnode.begin();
         it != hiAnode.end(); it++) {
        //check signals energy vs threshold
        energy = (*it)->GetCalibratedEnergy();
        if (energy < threshold_ || energy > 63000)
            continue;
        //parcel out position signals by tag
        if ((*it)->GetChanID().HasTag("xa") && xa_h == 0){
            xa_h = energy;
            highAnodeSum += energy;
        }
        if ((*it)->GetChanID().HasTag("xb") && xb_h == 0){
            xb_h = energy;
            highAnodeSum += energy;
        }
        if ((*it)->GetChanID().HasTag("ya") && ya_h == 0){
            ya_h = energy;
            highAnodeSum += energy;
        }
        if ((*it)->GetChanID().HasTag("yb") && yb_h == 0){
            yb_h = energy;
            highAnodeSum += energy;
        }
    }
    //compute position only if all 4 signals are present
    if (xa_l > 0 && xb_l > 0 && ya_l > 0 && yb_l > 0){
        hasPosition_low = true;
        position_low.first = CalculatePosition(xa_l, xb_l, ya_l, yb_l, vdtype_, rotation_).first;
        position_low.second  = CalculatePosition(xa_l, xb_l, ya_l, yb_l, vdtype_, rotation_).second;
        plot(DD_POS_LOW, position_low.first * positionScale_ + positionOffset_,
             position_low.second * positionScale_ + positionOffset_);
    }

    if (xa_h > 0 && xb_h > 0 && ya_h > 0 && yb_h > 0){
        hasPosition_high = true;
        position_high.first = CalculatePosition(xa_h, xb_h, ya_h, yb_h, vdtype_, rotation_).first;
        position_high.second = CalculatePosition(xa_h, xb_h, ya_h, yb_h, vdtype_, rotation_).second;
        plot(DD_POS_HIGH, position_high.first * positionScale_ + positionOffset_,
             position_high.second * positionScale_ + positionOffset_ );
    }

    //------------Positions from ion scintillator---------------------------------
        //using top - bottom and left - right computation scheme

    for(vector<ChanEvent *>::const_iterator it = ionTrig.begin(); it != ionTrig.end(); it++){
        //check signals energy vs threshold
        energy = (*it)->GetCalibratedEnergy();
        if (energy < 10)
            continue;
        //parcel out position signals by tag
        if ((*it)->GetChanID().HasTag("black") && top_l == 0 )
            top_l = energy;
        if ((*it)->GetChanID().HasTag("blue") && top_r == 0 )
            top_r = energy;
        if ((*it)->GetChanID().HasTag("white") && bottom_l == 0 )
            bottom_l = energy;
        if ((*it)->GetChanID().HasTag("green") && bottom_r == 0 )
            bottom_r = energy;
    }

    if (top_l > 0 && top_r > 0 && bottom_l > 0 && bottom_r > 0){
        hasPosition_ion = true;
        position_ion.first = (top_l + bottom_l - top_r - bottom_r) / (top_l + top_r + bottom_l + bottom_r);
        position_ion.second  = (top_l + top_r - bottom_l - bottom_r) / (top_l + top_r + bottom_l + bottom_r);
        plot(DD_POS_ION, position_ion.first * front_positionScale_ + front_positionOffset_,
             position_ion.second * front_positionScale_ + front_positionOffset_);
    }




    //----------------------------------------------------------------------------
    //------------Check Transmission efficiencies---------------------------------

        //check for valid upstream events, dE silicon events, and vetos for gating
    bool hasUpstream = false, hasDeSi = false, hasVeto = false;
    for(auto it = separatorScint.begin(); it != separatorScint.end(); it++){
        if ((*it)->GetCalibratedEnergy() > 1 && (*it)->GetCalibratedEnergy() < 10000){
            hasUpstream = true;
            break;
        }
    }

    for (auto it = desi.begin(); it != desi.end(); it++){
        if((*it)->GetCalibratedEnergy() > 1 && (*it)->GetCalibratedEnergy() < 10000){
            hasDeSi = true;
            break;
        }
    }

    for(auto it = veto.begin(); it != veto.end(); it++){
        if ((*it)->GetCalibratedEnergy() > 1 && (*it)->GetCalibratedEnergy() < 10000){
            hasVeto = true;
            break;
        }
    }


        //plot valid YSO positions and dE silicon events gated on upstream events
        //plot upstream events gated on dE silicon
        //plot transmission efficiency from upstream to YSO and veto

    if(hasUpstream) {
        for(auto de_it = desi.begin(); de_it != desi.end(); de_it++) {
            plot(D_DESI_ENERGY,(*de_it)->GetCalibratedEnergy());
        }
        if(hasPosition_low){
            plot(DD_SEPAR_GATED_LOW, position_low.first * positionScale_ + positionOffset_,
                    position_low.second * positionScale_ + positionOffset_);
        }
        if(hasPosition_ion){
            plot(DD_SEPAR_GATED_ION, position_ion.first * positionScale_ + positionOffset_,
                    position_ion.second * positionScale_ + positionOffset_);
        }
    }

    if(hasDeSi) {
        for (auto it_sep = separatorScint.begin(); it_sep != separatorScint.end(); it_sep++) {
            if ((*it_sep)->GetChanID().HasTag("left")) {
                plot(DD_SEPAR_ENERGY, (*it_sep)->GetCalibratedEnergy(), 0);
            } else if ((*it_sep)->GetChanID().HasTag("right")) {
                plot(DD_SEPAR_ENERGY, (*it_sep)->GetCalibratedEnergy(), 1);
            }
        }

        if (hasPosition_low) {
            plot(DD_DESI_GATED_LOW, position_low.first * positionScale_ + positionOffset_,
                 position_low.second * positionScale_ + positionOffset_);
        }
        if (hasPosition_ion) {
            plot(DD_DESI_GATED_ION, position_ion.first * positionScale_ + positionOffset_,
                 position_ion.second * positionScale_ + positionOffset_);
        }
    }

    if(hasPosition_low) {
        for (auto de_it = desi.begin(); de_it != desi.end(); de_it++) {
            plot(D_DESI_YSO_GATED, (*de_it)->GetCalibratedEnergy());
        }
        for (auto it_sep = separatorScint.begin(); it_sep != separatorScint.end(); it_sep++) {
            if ((*it_sep)->GetChanID().HasTag("left")) {
                plot(DD_SEPAR_YSO_GATED, (*it_sep)->GetCalibratedEnergy(), 0);
            } else if ((*it_sep)->GetChanID().HasTag("right")) {
                plot(DD_SEPAR_YSO_GATED, (*it_sep)->GetCalibratedEnergy(), 1);
            }
        }
    }

    if(hasUpstream)
        plot(D_TRANS_EFF_YSO, 0);
    if(hasUpstream && hasPosition_ion)
        plot(D_TRANS_EFF_YSO, 1);
    if(hasUpstream && hasPosition_low)
        plot(D_TRANS_EFF_YSO, 2);
    if (hasUpstream && hasVeto)
        plot(D_TRANS_EFF_YSO, 3);




    //----------------------------------------------------------------------------


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

    if (!lowDynode.empty())
        plot(DD_DY_SUM_LG,lowDynode.front()->GetCalibratedEnergy(),lowAnodeSum);

    if (!hiDynode.empty())
        plot(DD_DY_SUM_HG,hiDynode.front()->GetCalibratedEnergy(),highAnodeSum);

    EndProcess();
    return (true);

}

pair<double, double> PspmtProcessor::CalculatePosition(double &xa, double &xb, double &ya, double &yb, const VDTYPES &vdtype, double &rot){

    double x = 0, y = 0, x_tmp = 0, y_tmp = 0, center = 0;

    switch(vdtype){
        case corners:
            x_tmp = (0.5 * (yb + xa)) / (xa + xb + ya + yb);
            y_tmp = (0.5 * (xa + xb)) / (xa + xb + ya + yb);
            center = 0.2;
            break;
        case sides:
            x_tmp = (xa - xb) / (xa + xb);
            y_tmp = (ya - yb) / (ya + yb);
            center = 0;
            break;
        case UNKNOWN:
        default:
            cerr<<"We recieved a VD_TYPE we didn't recognize " << vdtype << endl;

    }
    x = (x_tmp - center) * cos(rot) - (y_tmp - center) * sin(rot) + center;  //rotate positions about center of image by angle rot    
    y = (x_tmp - center) * sin(rot) + (y_tmp - center) * cos(rot) + center;
    return make_pair(x, y);
}
