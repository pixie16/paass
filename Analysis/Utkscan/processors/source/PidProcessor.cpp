///@file PidProcessor.cpp
///@A dedicated processor for constructing PID information
///@author A. Keeler, R. Yokoyama
///@date July 29, 2019
// Updated for E19044 experiment.

#include "PidProcessor.hpp"

#include <limits.h>
#include <signal.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "Globals.hpp"
#include "HelperFunctions.hpp"
#include "Messenger.hpp"

using namespace std;
using namespace dammIds::pid;

namespace dammIds {
namespace pid {

const int D_TOF = 0;         //! TDiff between RFQ and beamline plastic
const int D_RFQ_MULT = 1;    //! Multiplicity of the RFQ
const int D_FP_MULT = 2;     //! Multiplicity of the plastic
const int DD_TACS_MULT = 3;  //! Multiplicities of the TACs
const int DD_TACS = 4;       //! TAC energies
const int DD_PINS_DE = 5;    //! Energy deposits in the PINs
const int DD_PINS_MULT = 6;  //! Multiplicities of the Pins
const int DD_PIN0_1 = 7;     //! Pin0 vs Pin1 dE

const int DD_TOF_PIN0 = 8;         //! ToF vs Pin0 dE
const int DD_TOF_PIN0_GATED = 9;   //! GATED ToF vs Pin0 dE
const int DD_TOF_PIN1 = 10;        //! ToF vs Pin1 dE
const int DD_TOF_PIN1_GATED = 11;  //! GATED ToF vs Pin1 dE

const int DD_TAC0_PIN0 = 12;        //! TAC0 vs Pin0 dE
const int DD_TAC0_PIN0_GATED = 13;  //! GATED TAC0 vs Pin0 dE
const int DD_TAC1_PIN0 = 14;        //! TAC0 vs Pin0 dE
const int DD_TAC1_PIN0_GATED = 15;  //! YSO gated TAC0 vs Pin0 dE

const int DD_TAC0_PIN1 = 16;        //! TAC0 vs Pin1 dE
const int DD_TAC0_PIN1_GATED = 17;  //! GATED TAC0 vs Pin1 dE
const int DD_TAC1_PIN1 = 18;        //! TAC1 vs Pin1 dE
const int DD_TAC1_PIN1_GATED = 19;  //! GATED TAC1 vs Pin1 dE

}  // namespace pid
}  // namespace dammIds

void PidProcessor::DeclarePlots(void) {
    DeclareHistogram1D(D_TOF, SB, "TOF RFQ:FP (internal)");
    DeclareHistogram1D(D_RFQ_MULT, S5, "Multiplicity of RFQ");
    DeclareHistogram1D(D_FP_MULT, S5, "Multiplicity of beamline plastic");
    DeclareHistogram2D(DD_TACS_MULT,S2, S5, "TAC multiplicity ");

    DeclareHistogram2D(DD_TACS, S2, SB, "TAC energy ");
    DeclareHistogram2D(DD_PINS_DE, S2, SB, "Pin dE");
    DeclareHistogram2D(DD_PINS_MULT, S2, S5, "Pins Multiplicity");
    DeclareHistogram2D(DD_PIN0_1, SB, SB, "Pin0 vs Pin1 dE ");

    DeclareHistogram2D(DD_TOF_PIN0, SB, SB, "Tof vs Pin0 dE ");
    DeclareHistogram2D(DD_TOF_PIN0_GATED, SB, SB, "YSO: Tof vs Pin0 dE ");
    DeclareHistogram2D(DD_TOF_PIN1, SB, SB, "Tof vs Pin1 dE ");
    DeclareHistogram2D(DD_TOF_PIN1_GATED, SB, SB, "YSO:Tof vs Pin1 dE ");

    DeclareHistogram2D(DD_TAC0_PIN0, SB, SB, "TAC0 vs Pin0 dE ");
    DeclareHistogram2D(DD_TAC0_PIN0_GATED, SB, SB, "YSO: TAC0 vs Pin0 dE ");
    DeclareHistogram2D(DD_TAC1_PIN0, SB, SB, "TAC1 vs Pin0 dE ");
    DeclareHistogram2D(DD_TAC1_PIN0_GATED, SB, SB, "YSO: TAC1 vs Pin0 dE ");

    DeclareHistogram2D(DD_TAC0_PIN1, SB, SB, "TAC0 vs Pin1 dE ");
    DeclareHistogram2D(DD_TAC0_PIN1_GATED, SB, SB, "YSO: TAC0 vs Pin1 dE ");
    DeclareHistogram2D(DD_TAC1_PIN1, SB, SB, "TAC1 vs Pin1 dE ");
    DeclareHistogram2D(DD_TAC1_PIN1_GATED, SB, SB, "YSO: TAC1 vs Pin1 dE ");

}  // Declare plots

PidProcessor::PidProcessor(const double &YSO_Implant_thresh, const double &FIT_thresh) : EventProcessor(OFFSET, RANGE, "PidProcessor") {
    associatedTypes.insert("pid");
    associatedTypes.insert("pin");

    yso_threshold_ = YSO_Implant_thresh;
    fit_threshold_ = FIT_thresh;
}

bool PidProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    // A flag for ROOT output
    const bool root_output = DetectorDriver::get()->GetSysRootOutput();

    //! Generate YSO implant Bool with standard tree correlator places
    bool YSO_Implant = false;
    if (TreeCorrelator::get()->checkPlace("pspmt_dynode_low_0")) {
        if (TreeCorrelator::get()->place("pspmt_dynode_low_0")->last().energy > yso_threshold_) {
            YSO_Implant = true;
        };
    };

    if (root_output) {
        // Initialization of the pid_struct
        pid_struct = processor_struct::PID_DEFAULT_STRUCT;
    }

    static const vector<ChanEvent *> &pin0_vec = event.GetSummary("pid:pin0", true)->GetList();
    static const vector<ChanEvent *> &pin1_vec = event.GetSummary("pid:pin1", true)->GetList();
    static const vector<ChanEvent *> &tac0_vec = event.GetSummary("pid:tac0", true)->GetList();
    static const vector<ChanEvent *> &tac1_vec = event.GetSummary("pid:tac1", true)->GetList();
    static const vector<ChanEvent *> &rfq_vec = event.GetSummary("pid:rfq", true)->GetList();
    static const vector<ChanEvent *> &fp_vec = event.GetSummary("pid:fp", true)->GetList();

    // Function that compares energies in two ChanEvent objects
    auto compare_energy = [](ChanEvent *x1, ChanEvent *x2) { return x1->GetEnergy() < x2->GetEnergy(); };

    // Function that returns time in ns
    auto get_time_in_ns = [](ChanEvent *x) {
        return x->GetTimeSansCfd() * Globals::get()->GetClockInSeconds(x->GetChanID().GetModFreq()) * 1e9;
    };

    //* Tof between rfq and beamline FocalPlane */
    // cout<<"tofCompute::  rfq_vec.size()="<<rfq_vec.size()<< "   fp.size()"<<fp_vec.size()<<endl;
    if (!rfq_vec.empty() && !fp_vec.empty()) {
        // Get the first element in this event for RFQ
        auto rfq = rfq_vec.at(0);
        const double internalTAC_Convert_Tick_adc = Globals::get()->GetAdcClockInSeconds(rfq->GetChanID().GetModFreq()) * 1e9;
        const double internalTAC_Convert_Tick_dsp = Globals::get()->GetClockInSeconds(rfq->GetChanID().GetModFreq()) * 1e9;
        // Get elements with the largest energy in this event for fp
        auto fp = std::max_element(fp_vec.begin(), fp_vec.end(), compare_energy);
        // Check for nullptr
        if ((*fp)) {
            // Calculate tof
            double tof = (rfq->GetTime() * internalTAC_Convert_Tick_adc) - ((*fp)->GetTime() * internalTAC_Convert_Tick_adc);
            // cout<<std::setprecision(14)<<"ToF= "<< tof<<"rfq time = "<< rfq->GetTime() << "   fp time= " << (*fp)->GetTime()<<endl;
            plot(D_TOF, tof);
            // ROOT outputs
            if (root_output) {
                pid_struct.rfq_time = get_time_in_ns(rfq);
                pid_struct.fp_time = get_time_in_ns(*fp);
                pid_struct.tof = tof;
            }
        }
    }

    //** TACs */
    if (!tac0_vec.empty()) {
        for (auto tac = tac0_vec.begin(); tac != tac0_vec.end(); ++tac) {
            plot(DD_TACS, 0, (*tac)->GetCalibratedEnergy());
            if (root_output) {
                pid_struct.tac_0 = (*tac)->GetCalibratedEnergy();
            }
        }
    }
    if (!tac1_vec.empty()) {
        for (auto tac = tac1_vec.begin(); tac != tac1_vec.end(); ++tac) {
            plot(DD_TACS, 1, (*tac)->GetCalibratedEnergy());
            if (root_output) {
                pid_struct.tac_1 = (*tac)->GetCalibratedEnergy();
            }
        }
    }

    //** Pins */

    if (!pin0_vec.empty()) {
        plot(DD_PINS_MULT, 0, pin0_vec.size());
        auto pin = max_element(pin0_vec.begin(),pin0_vec.end(),compare_energy);
            plot(DD_PINS_DE, 0, (*pin)->GetCalibratedEnergy());
            if (root_output) {
                pid_struct.pin_0_energy = (*pin)->GetCalibratedEnergy();
                pid_struct.pin_0_time = get_time_in_ns(*pin);
            }
    }
    if (!pin1_vec.empty()) {
        plot(DD_PINS_MULT, 1, pin1_vec.size());
        auto pin = max_element(pin1_vec.begin(),pin1_vec.end(),compare_energy);
            plot(DD_PINS_DE, 1, (*pin)->GetCalibratedEnergy());
            if (root_output) {
                pid_struct.pin_1_energy = (*pin)->GetCalibratedEnergy();
                pid_struct.pin_0_time = get_time_in_ns(*pin);
            }
    }

    // Fill the multiplicity plots
    plot(D_RFQ_MULT, rfq_vec.size());
    plot(D_FP_MULT, fp_vec.size());
    //plot(D_TACS_MULT, 0, tac_vec.size());

    // Plot 2d histograms
    plot(DD_PIN0_1, pid_struct.pin_0_energy, pid_struct.pin_1_energy);

    plot(DD_TOF_PIN0, pid_struct.tof, pid_struct.pin_0_energy);
    plot(DD_TOF_PIN1, pid_struct.tof, pid_struct.pin_1_energy);

    plot(DD_TAC0_PIN0, pid_struct.tac_0, pid_struct.pin_0_energy);
    plot(DD_TAC0_PIN1, pid_struct.tac_0, pid_struct.pin_1_energy);

    plot(DD_TAC1_PIN0, pid_struct.tac_1, pid_struct.pin_0_energy);
    plot(DD_TAC1_PIN1, pid_struct.tac_1, pid_struct.pin_1_energy);

    // if( YSO_Implant){
    //     plot()
    // }

    if (root_output) {
        // Fill the event to the PixeTreeEvent object
        pixie_tree_event_->pid_vec_.emplace_back(pid_struct);
        pid_struct = processor_struct::PID_DEFAULT_STRUCT;
    }

    EndProcess();
    return true;
}

bool PidProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    EndProcess();
    return true;
}

double PidProcessor::CorrectTofByPosition(double &tof, double &position, double &slope, double &intercept) {
    return tof - slope * position - intercept;
}

double PidProcessor::ConvertTofToAQ(double &tof) {
    return tof;
}

double PidProcessor::ConvertPinToZ(double &pin) {
    return pin;
}
