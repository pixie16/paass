//
// Created by darren on 2/21/19.
//


#include <algorithm>
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <csignal> //<signal.h>
#include <climits> //<limits.h>
#include <cmath>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "MtasPspmtProcessor.hpp"
#include "Globals.hpp"
#include "Messenger.hpp"

using namespace std;
using namespace dammIds::mtaspspmt;

namespace dammIds {
    namespace mtaspspmt {
        const int DD_MULTI = 4;
        const int DD_POS_DIAG = 1;
        const int DD_POS_IMPL = 2;

    }
}

void MtasPspmtProcessor::DeclarePlots() {  //(void) {
    DeclareHistogram2D(DD_POS_DIAG, SB, SB, "Diagnostic Detector Positions");
    DeclareHistogram2D(DD_POS_IMPL, SB, SB, "Implant Detector Positions");
    DeclareHistogram2D(DD_MULTI,S3,S3, "Multiplicity");
}

MtasPspmtProcessor::MtasPspmtProcessor(const std::string &dt, const double &scale,
                                       const unsigned int &offset, const double &threshold)
                    :EventProcessor(OFFSET, RANGE, "MtasPspmtProcessor"){
    if(dt == "implant")
        dttype_ = implant;
    else if(dt == "diagnostic")
        dttype_ = diagnostic;
    else
        dttype_ = UNKNOWN;

    DTtypeStr = dt;
    positionScale_ = scale;
    positionOffset_ = offset;
    threshold_ = threshold;
    ThreshStr = threshold;
    associatedTypes.insert("mtaspspmt");

}

bool MtasPspmtProcessor::PreProcess(RawEvent &event) {

    if (!EventProcessor::PreProcess(event)){
        return false;
    }

    // read in signals
    static const vector<ChanEvent *> &impl = event.GetSummary("mtaspspmt:implant")->GetList();
    static const vector<ChanEvent *> &diag = event.GetSummary("mtaspspmt:diagnostic")->GetList();



    // set up position calculation for signals
    position_mtas.first = 0, position_mtas.second = 0;
    //initialize
    double energy = 0.;
    double xa = 0, xb = 0, ya = 0, yb = 0;

    if (dttype_ == implant){
        plot(DD_MULTI, impl.size(),3);
    } else if (dttype_ == diagnostic){
        plot(DD_MULTI, diag.size(),3);
    }

    // Calculate info if IMPLANT DETECTOR
    double sumImpl = 0;
    for (auto it = impl.begin(); it !=impl.end(); it++) {
        energy = (*it)->GetCalibratedEnergy();
        if (energy < threshold_){
            continue;
        }
        std::string group = (*it)->GetChanID().GetGroup();
        if (group == "xa" && xa == 0){
            xa = energy;
            sumImpl += energy;
        }
        if (group == "xb" && xb == 0){
            xb = energy;
            sumImpl += energy;
        }

        //compute position if both signals are present
        if (xa > 0 && xb > 0){
            position_mtas.first = CalculatePosition(xa, xb, ya, yb, dttype_).first;
            position_mtas.second = CalculatePosition(xa, xb, ya, yb, dttype_).second;
            plot(DD_POS_IMPL, position_mtas.first * positionScale_ + positionOffset_,
                 position_mtas.second * positionScale_ + positionOffset_);
        }
    }

    // Calculate info if DIAGNOSTIC DETECTOR
    double sumDiag = 0;
    for (auto it = diag.begin(); it !=diag.end(); it++) {
        energy = (*it)->GetCalibratedEnergy();
        if (energy < threshold_){
            continue;
        }
        std::string group = (*it)->GetChanID().GetGroup();
        if (group == "xa" && xa == 0){
            xa = energy;
            sumDiag += energy;
        }
        if (group == "xb" && xb == 0){
            xb = energy;
            sumDiag += energy;
        }
        if (group == "ya" && ya == 0){
            ya = energy;
            sumDiag += energy;
        }
        if (group == "yb" && yb == 0){
            yb = energy;
            sumDiag += energy;
        }

        //compute position if all signals are present
        if (xa > 0 && xb > 0 && ya > 0 && yb > 0){
            position_mtas.first = CalculatePosition(xa, xb, ya, yb, dttype_).first;
            position_mtas.second = CalculatePosition(xa, xb, ya, yb, dttype_).second;
            plot(DD_POS_DIAG, position_mtas.first * positionScale_ + positionOffset_,
                 position_mtas.second * positionScale_ + positionOffset_);
        }
    }


    EndProcess();
    return (true);
}

pair<double, double> MtasPspmtProcessor::CalculatePosition(
        double &xa, double &xb, double &ya, double &yb,
        const MtasPspmtProcessor::DTTYPES &dttype) {
    double x = 0, y = 0;
    switch(dttype){
        case implant:
            x = (xa - xb ) / (xa+xb);
            y = 0.5;
            break;
        case diagnostic:
            x = ((ya + yb)-(xa + xb))/(xa+xb+ya+yb);
            y = ((xa + ya)-(xb + yb))/(xa+xb+ya+yb);
            break;
        case UNKNOWN:
        default:
            cerr<< "We received a Detector Type we didn't recognize: " << dttype << endl;
    }
    return make_pair(x, y);
}