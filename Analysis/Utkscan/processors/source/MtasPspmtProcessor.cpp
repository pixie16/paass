///@file MtasPspmtProcessor.cpp
///@brief Processes information from a series of SiPMs to determine position. Based on PspmtProcessor.cpp
///@author D.McKinnon, A. Keeler, S. Go, S. V. Paulauskas
///@date February 21, 2019

#include <algorithm>
#include <climits>
#include <cmath>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "Globals.hpp"
#include "Messenger.hpp"
#include "MtasPspmtProcessor.hpp"
#include "HelperFunctions.hpp"

using namespace std;
using namespace dammIds::mtaspspmt;

namespace dammIds {
namespace mtaspspmt {
const int DD_POS_DIAG = 1;
const int DD_POS_IMPL = 2;
// added by Cooper
const int D_RATE_IMPL = 3;
const int D_MULTIPL = 4;

}  // namespace mtaspspmt
}  // namespace dammIds

void MtasPspmtProcessor::DeclarePlots() {
    DeclareHistogram2D(DD_POS_DIAG, SB, SB, "Diagnostic Detector Positions");
    DeclareHistogram2D(DD_POS_IMPL, SB, SB, "Implant Detector Positions");
    // Added by Cooper
    DeclareHistogram1D(D_RATE_IMPL, SE, "Implant Rate vs Current Time check");
    DeclareHistogram1D(D_MULTIPL, S4, "Left, Right, Total Hits");
}

MtasPspmtProcessor::MtasPspmtProcessor(const double &impl_scale, const unsigned int &impl_offset,
                                       const double &impl_threshold, const double &diag_scale,
                                       const unsigned int &diag_offset, const double &diag_threshold)
    : EventProcessor(OFFSET, RANGE, "MtasPspmtProcessor") {
    implPosScale_ = impl_scale;
    implPosOffset_ = impl_offset;
    implThreshold_ = 50;
    // implThreshold_ = impl_threshold;
    diagPosScale_ = diag_scale;
    diagPosOffset_ = diag_offset;
    diagThreshold_ = diag_threshold;
    associatedTypes.insert("mtaspspmt");
}

bool MtasPspmtProcessor::PreProcess(RawEvent &event) {
    // cout << "1";
    if (!EventProcessor::PreProcess(event)) {
        return false;
    }
    if (DetectorDriver::get()->GetSysRootOutput()) {
        MTASPSstruct = processor_struct::MTASPSPMT_DEFAULT_STRUCT;
    }

    // cout << "2";
    // added by Cooper
    if (evtNum_ == 0) {
        first_time = DetectorDriver::get()->GetFirstEventTimeinNs();
        cout << "haha" << first_time << endl;
    }
    double time1 = 0;
    // cout << "3";
    double bunch_time = 0.0;
    double counter_true = 0.0;
    evtNum_ = DetectorDriver::get()->GetEventNumber();
    // cout << "4";
    // read in signals, getting summary for "type:subtype"
    static const vector<ChanEvent *> &impl = event.GetSummary("mtaspspmt:implant")->GetList();
    // static const vector<ChanEvent *> &diag = event.GetSummary("mtaspspmt:diagnostic")->GetList();
    // cout << "5";
    // added by Cooper
    // double currentTime = impl.front()->GetTimeSansCfd() * Globals::get()->GetClockInSeconds(impl.front()->GetChanID().GetModFreq()) * 1.e9;
    double currentTime = impl.front()->GetTimeSansCfd() * 10; // removed 1.e9
    cout << "haha2" << currentTime << endl;
    double delta_T = currentTime - first_time;
    cout << "haha3" << delta_T << endl;
    double counter_bunch = 0.0;
    bunch_time = bunch_time + delta_T;
    // cout << delta_T << " \n";
    // cout << "6";
    if (delta_T > pow(10, 9)) {
        //counter_bunch = counter_true;
        bin_num += 1;
        cout << "bin number is" << bin_num << endl;
        first_time = currentTime;
        //double rate_1 = (counter_true / bunch_time) * pow(10, 9);
        // plot(DD_RATE_IMPL, rate_1, time1 * pow(10, 9));
        // bunch_time = 0.0;
        // counter_true = 0.0;
    }
    // cout << "7";
    // above added by Cooper

    //initialize
    double energy = 0;

    double xa = 0, xb = 0, ya = 0, yb = 0;
    bool counter_xa = false, counter_xb = false;
    // cout << "8";
    // IMPLANT DETECTOR Calculations
    double sumImpl = 0;
    int flag1 = 0;
    cout <<"mo"<< impl.size() << endl;
    for (auto it = impl.begin(); it != impl.end(); it++) {
        energy = (*it)->GetCalibratedEnergy();
        cout << "check here" << endl;
        cout << energy << endl;
        if (energy < implThreshold_) {
            cout << "passed" << endl;
            continue;
        }
        std::string group = (*it)->GetChanID().GetGroup();
        cout << "The group is " << group << endl;
        if (group == "xa" && xa == 0) {
            xa = energy;
            sumImpl += energy;
            counter_xa = true;
            cout << "counter_xa" << "is" << counter_xa << endl;
            plot(D_MULTIPL,1);
        }
        if (group == "xb" && xb == 0) {
            xb = energy;
            sumImpl += energy;
            counter_xb = true;
            cout << "counter_xb" << "is" << counter_xb << endl;
            plot(D_MULTIPL,2);
        }

        if (xa > 0 && xb > 0) {
            double x = 0., y = 0.;
            x = (xa - xb) / (xa + xb);
            y = 0.5;  // shift it up off the axis
            plot(DD_POS_IMPL, x * implPosScale_ + implPosOffset_, y * implPosScale_ + implPosOffset_);
            //if (flag1 < 1){
            plot(D_MULTIPL,3);//}
            // counter_true += 1;
            plot(D_RATE_IMPL, bin_num);
            //flag1 = 10;
        }
        // Below is added by Michael Cooper
    }
    // cout << "8";
    if (counter_xa && counter_xb) {
        cout << "hit3 \n";

    }
    // cout << "9";
    // time1 = time1 + delta_T;
    // added by Cooper
    /*
    // DIAGNOSTIC DETECTOR Calculations
    double sumDiag = 0;
    for (auto it = diag.begin(); it != diag.end(); it++) {
        energy = (*it)->GetCalibratedEnergy();
        if (energy < diagThreshold_) {
            continue;
        }
        std::string group = (*it)->GetChanID().GetGroup();
        if (group == "xa" && xa == 0) {
            xa = energy;
            sumDiag += energy;
        }
        if (group == "xb" && xb == 0) {
            xb = energy;
            sumDiag += energy;
        }
        if (group == "ya" && ya == 0) {
            ya = energy;
            sumDiag += energy;
        }
        if (group == "yb" && yb == 0) {
            yb = energy;
            sumDiag += energy;
        }

        if (xa > 0. && xb > 0. && ya > 0. && yb > 0.) {
            double x = 0., y = 0.;
            x = ((xa + ya) - (xb + yb)) / (xa + xb + ya + yb);
            y = ((xa + xb) - (ya + yb)) / (xa + xb + ya + yb);
            plot(DD_POS_DIAG, x * diagPosScale_ + diagPosOffset_, y * diagPosScale_ + diagPosOffset_);
        }
    }
    */
    // cout << "10";
    EndProcess();
    return (true);
}
