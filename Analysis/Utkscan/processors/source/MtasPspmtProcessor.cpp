///@file MtasPspmtProcessor.cpp
///@brief Processes information from a series of SiPMs to determine position. Based on PspmtProcessor.cpp
///@author D.McKinnon, A. Keeler, S. Go, S. V. Paulauskas
///@date February 21, 2019


#include <algorithm>
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <csignal>
#include <climits>
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
        const int DD_POS_DIAG = 1;
        const int DD_POS_IMPL = 2;
    }
}

void MtasPspmtProcessor::DeclarePlots() {
    DeclareHistogram2D(DD_POS_DIAG, SB, SB, "Diagnostic Detector Positions");
    DeclareHistogram2D(DD_POS_IMPL, SB, SB, "Implant Detector Positions");
}

MtasPspmtProcessor::MtasPspmtProcessor(const double &impl_scale, const unsigned int &impl_offset,
                                       const double &impl_threshold, const double &diag_scale,
                                       const unsigned int &diag_offset, const double &diag_threshold)
                    :EventProcessor(OFFSET, RANGE, "MtasPspmtProcessor"){

    implPosScale_ = impl_scale;
    implPosOffset_ = impl_offset;
    implThreshold_ = impl_threshold;
    diagPosScale_ = diag_scale;
    diagPosOffset_ = diag_offset;
    diagThreshold_ = diag_threshold;
    associatedTypes.insert("mtaspspmt");
}

bool MtasPspmtProcessor::PreProcess(RawEvent &event) {

    if (!EventProcessor::PreProcess(event)){
        return false;
    }

    // read in signals, getting summary for "type:subtype"
    static const vector<ChanEvent *> &impl = event.GetSummary("mtaspspmt:implant")->GetList();
    static const vector<ChanEvent *> &diag = event.GetSummary("mtaspspmt:diagnostic")->GetList();

    //initialize
    double energy = 0;
    double xa = 0, xb = 0, ya = 0, yb = 0;

    // IMPLANT DETECTOR Calculations
    double sumImpl = 0;
    for (auto it = impl.begin(); it !=impl.end(); it++) {
        energy = (*it)->GetCalibratedEnergy();
        if (energy < implThreshold_){
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

        if (xa > 0 && xb > 0){
            double x = 0. ,y = 0.;
            x = (xa - xb ) / (xa+xb);
            y = 0.5; // shift it up off the axis
            plot(DD_POS_IMPL, x * implPosScale_ + implPosOffset_, y * implPosScale_ + implPosOffset_);
        }
    }

    // DIAGNOSTIC DETECTOR Calculations
    double sumDiag = 0;
    for (auto it = diag.begin(); it !=diag.end(); it++) {
        energy = (*it)->GetCalibratedEnergy();
        if (energy < diagThreshold_){
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

        if (xa > 0. && xb > 0. && ya > 0. && yb > 0.){
            double x = 0., y = 0.;
            x = ((xa + ya)-(xb + yb))/(xa+xb+ya+yb);
            y = ((xa + xb)-(ya + yb))/(xa+xb+ya+yb);
            plot(DD_POS_DIAG, x * diagPosScale_ + diagPosOffset_, y * diagPosScale_ + diagPosOffset_);
        }
    }

    EndProcess();
    return (true);
}
