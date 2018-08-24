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
        const int DD_POS_LEFT = 1;
        const int DD_POS_RIGHT = 2;
    }
}

void PspmtProcessor::DeclarePlots(void) {
    DeclareHistogram2D(DD_DYNODE_QDC, SD, S2, "Dynode QDC- Left 0, Right 1");
    DeclareHistogram2D(DD_POS_LEFT, SB, SB, "Left Positions");
    DeclareHistogram2D(DD_POS_RIGHT, SB, SB, "Right Positions");
}



PspmtProcessor::PspmtProcessor(const std::string &vd, const double &scale, const unsigned int &offset,
                               const double &threshold) :EventProcessor(OFFSET, RANGE, "PspmtProcessor"){


  std::cout<<"PspmtProcessor::PspmtProcessor()"<<std::endl;

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

    //if (DetectorDriver::get()->GetSysRootOutput()) {
    //    PSvec.clear();
    //    PSstruct = DefaultStruc;
    // }

    //read in anode & dynode signals
    static const vector<ChanEvent *> &rightDynode = event.GetSummary("pspmt:dynode_right")->GetList();
    static const vector<ChanEvent *> &leftDynode = event.GetSummary("pspmt:dynode_left")->GetList();
    static const vector<ChanEvent *> &rightAnode = event.GetSummary("pspmt:anode_right")->GetList();
    static const vector<ChanEvent *> &leftAnode =  event.GetSummary("pspmt:anode_left")->GetList();


    //Plot Dynode QDCs
    for(vector<ChanEvent *>::const_iterator it = leftDynode.begin(); it != leftDynode.end(); it++){
        plot(DD_DYNODE_QDC, (*it)->GetTrace().GetQdc(), 0);
    }
    for(vector<ChanEvent *>::const_iterator it = rightDynode.begin(); it != rightDynode.end(); it++){
        plot(DD_DYNODE_QDC, (*it)->GetTrace().GetQdc(), 1);
    }

    //set up position calculation for left and right  signals
    position_left.first = 0, position_left.second = 0;
    position_right.first = 0, position_right.second = 0;
    double energy = 0;
    double xa_l = 0, ya_l = 0, xb_l = 0, yb_l = 0;
    double xa_r = 0, ya_r = 0, xb_r = 0, yb_r = 0;

    for(vector<ChanEvent *>::const_iterator it = leftAnode.begin(); it != leftAnode.end(); it++){
        //check signals energy vs threshold
        energy = (*it)->GetCalibratedEnergy();
	cout<<"PspmtProcessor::PreProcess()->energy: "<<energy<<endl;
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

    for(vector<ChanEvent *>::const_iterator it = rightAnode.begin();
        it != rightAnode.end(); it++){
        //check signals energy vs threshold
        energy = (*it)->GetCalibratedEnergy();
        if (energy < threshold_ || energy > 63000)
            continue;
        //parcel out position signals by tag
        if ((*it)->GetChanID().HasTag("xa") && xa_r == 0)
	    xa_r = energy;
        if ((*it)->GetChanID().HasTag("xb") && xb_r == 0)
            xb_r = energy;
        if ((*it)->GetChanID().HasTag("ya") && ya_r == 0)
            ya_r = energy;
        if ((*it)->GetChanID().HasTag("yb") && yb_r == 0)
            yb_r = energy;
    }

    if (xa_l > 0 && xb_l > 0 && ya_l > 0 && yb_l > 0){
        position_left.first = CalculatePosition(xa_l, xb_l, ya_l, yb_l, vdtype_).first;
        position_left.second  = CalculatePosition(xa_l, xb_l, ya_l, yb_l, vdtype_).second;
        plot(DD_POS_LEFT, position_left.first * positionScale_ + positionOffset_,
             position_left.second * positionScale_ + positionOffset_);
    }

    if (xa_r > 0 && xb_r > 0 && ya_r > 0 && yb_r > 0){
        position_right.first = CalculatePosition(xa_r, xb_r, ya_r, yb_r, vdtype_).first;
        position_right.second = CalculatePosition(xa_r, xb_r, ya_r, yb_r, vdtype_).second;
        plot(DD_POS_RIGHT, position_right.first * positionScale_ + positionOffset_,
             position_right.second * positionScale_ + positionOffset_);
    }

    /*if (DetectorDriver::get()->GetSysRootOutput()) {

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
        PSstruct.anodeLmulti = lowAnode.size();
        PSstruct.anodeHmulti = hiAnode.size();
        PSstruct.dyLmulti = lowDynode.size();
        PSstruct.dyHmulti = hiDynode.size();
        PSstruct.xposL = position_low.first;
        PSstruct.yposL = position_low.second;
        PSstruct.xposH = position_high.first;
        PSstruct.yposH = position_high.second;

        PSvec.emplace_back(PSstruct);
        PSstruct=DefaultStruc;
	}*/
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
