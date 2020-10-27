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

const int D_TOF0 = 0;        //! TDiff between RFQ and beamline plastic
const int D_RFQ_MULT = 1;    //! Multiplicity of the RFQ
const int D_FP_MULT = 2;     //! Multiplicity of the plastic
const int DD_TACS_MULT = 3;  //! Multiplicities of the TACs
const int DD_TACS = 4;       //! TAC energies
const int DD_PINS_DE = 5;    //! Energy deposits in the PINs
const int DD_PINS_MULT = 6;  //! Multiplicities of the Pins
const int DD_PIN0_1 = 7;     //! Pin0 vs Pin1 dE

const int DD_TOF0_PIN0 = 8;             //! ToF vs Pin0 dE
const int DD_TOF0_PIN0_GATED_YSO = 9;   //! GATED ToF vs Pin0 dE
const int DD_TOF0_PIN1 = 10;            //! ToF vs Pin1 dE
const int DD_TOF0_PIN1_GATED_YSO = 11;  //! GATED ToF vs Pin1 dE

const int DD_TAC0_PIN0 = 12;            //! TAC0 vs Pin0 dE
const int DD_TAC0_PIN0_GATED_YSO = 13;  //! GATED TAC0 vs Pin0 dE
const int DD_TAC1_PIN0 = 14;            //! TAC0 vs Pin0 dE
const int DD_TAC1_PIN0_GATED_YSO = 15;  //! YSO gated TAC0 vs Pin0 dE

const int DD_TAC0_PIN1 = 16;            //! TAC0 vs Pin1 dE
const int DD_TAC0_PIN1_GATED_YSO = 17;  //! GATED TAC0 vs Pin1 dE
const int DD_TAC1_PIN1 = 18;            //! TAC1 vs Pin1 dE
const int DD_TAC1_PIN1_GATED_YSO = 19;  //! GATED TAC1 vs Pin1 dE

const int DD_TOF0_PIN0_GATED_FIT = 20;  //! GATED ToF vs Pin0 dE
const int DD_TOF0_PIN1_GATED_FIT = 21;  //! GATED ToF vs Pin1 dE
const int DD_TAC0_PIN0_GATED_FIT = 22;  //! GATED TAC0 vs Pin0 dE
const int DD_TAC1_PIN0_GATED_FIT = 23;  //! GATED TAC1 vs Pin0 dE
const int DD_TAC0_PIN1_GATED_FIT = 24;  //! GATED TAC0 vs Pin1 dE
const int DD_TAC1_PIN1_GATED_FIT = 25;  //! GATED TAC1 vs Pin1 dE

const int D_TOF1 = 26;                  //! TDiff between RFQ and beamline plastic
const int DD_TOF1_PIN0 = 27;            //! ToF vs Pin0 dE
const int DD_TOF1_PIN0_GATED_YSO = 28;  //! GATED ToF vs Pin0 dE
const int DD_TOF1_PIN1 = 29;            //! ToF vs Pin1 dE
const int DD_TOF1_PIN1_GATED_YSO = 30;  //! GATED ToF vs Pin1 dE

const int DD_TOF1_PIN0_FLIP = 31;            //! ToF vs Pin1 dE flipped
const int DD_TOF1_PIN0_GATED_YSO_FLIP = 32;  //! GATED ToF vs Pin1 dE flipped

}  // namespace pid
}  // namespace dammIds

void PidProcessor::DeclarePlots(void) {
    // DeclareHistogram1D(D_TOF0, SD, "TOF RFQ:FP (internal)");
    // DeclareHistogram1D(D_TOF1, SD, "TOF RFQ:Pin (internal)");
    DeclareHistogram1D(D_RFQ_MULT, S5, "Multiplicity of RFQ");
    DeclareHistogram1D(D_FP_MULT, S5, "Multiplicity of beamline plastic");
    DeclareHistogram2D(DD_TACS_MULT, S2, S5, "TAC multiplicity ");

    DeclareHistogram2D(DD_TACS, S2, SD, "TAC energy ");
    DeclareHistogram2D(DD_PINS_DE, S2, SD, "Pin dE");
    DeclareHistogram2D(DD_PINS_MULT, S2, S5, "Pins Multiplicity");
    DeclareHistogram2D(DD_PIN0_1, SC, SC, "Pin0 vs Pin1 dE ");

    DeclareHistogram2D(DD_TOF0_PIN0, SB, SD, "Tof0 vs Pin0 dE ");
    DeclareHistogram2D(DD_TOF0_PIN0_GATED_YSO, SB, SD, "YSO: Tof0 vs Pin0 dE ");
    DeclareHistogram2D(DD_TOF0_PIN0_GATED_FIT, SB, SD, "FIT: Tof0 vs Pin0 dE ");
    DeclareHistogram2D(DD_TOF0_PIN1, SB, SD, "Tof0 vs Pin1 dE ");
    DeclareHistogram2D(DD_TOF0_PIN1_GATED_YSO, SB, SD, "YSO:Tof0 vs Pin1 dE ");
    DeclareHistogram2D(DD_TOF0_PIN1_GATED_FIT, SB, SD, "FIT:Tof0 vs Pin1 dE ");

    DeclareHistogram2D(DD_TOF1_PIN0, SB, SD, "Tof1 vs Pin0 dE ");
    DeclareHistogram2D(DD_TOF1_PIN0_GATED_YSO, SB, SD, "YSO: Tof1 vs Pin0 dE ");
    // DeclareHistogram2D(DD_TOF1_PIN0_GATED_FIT, SB, SD, "FIT: Tof1 vs Pin0 dE ");
    DeclareHistogram2D(DD_TOF1_PIN1, SB, SD, "Tof1 vs Pin1 dE ");
    // DeclareHistogram2D(DD_TOF1_PIN1_GATED_YSO, SB, SD, "YSO:Tof1 vs Pin1 dE ");
    // DeclareHistogram2D(DD_TOF1_PIN1_GATED_FIT, SB, SD, "FIT:Tof1 vs Pin1 dE ");

    DeclareHistogram2D(DD_TAC0_PIN0, SD, SD, "TAC0 vs Pin0 dE ");
    DeclareHistogram2D(DD_TAC0_PIN0_GATED_YSO, SD, SD, "YSO: TAC0 vs Pin0 dE ");
    DeclareHistogram2D(DD_TAC0_PIN0_GATED_FIT, SD, SD, "FIT: TAC0 vs Pin0 dE ");
    DeclareHistogram2D(DD_TAC1_PIN0, SB, SD, "TAC1 vs Pin0 dE ");
    // DeclareHistogram2D(DD_TAC1_PIN0_GATED_YSO, SB, SD, "YSO: TAC1 vs Pin0 dE ");
    // DeclareHistogram2D(DD_TAC1_PIN0_GATED_FIT, SB, SD, "FIT: TAC1 vs Pin0 dE ");

    DeclareHistogram2D(DD_TAC0_PIN1, SB, SD, "TAC0 vs Pin1 dE ");
    DeclareHistogram2D(DD_TAC0_PIN1_GATED_YSO, SB, SD, "YSO: TAC0 vs Pin1 dE ");
    DeclareHistogram2D(DD_TAC0_PIN1_GATED_FIT, SB, SD, "FIT: TAC0 vs Pin1 dE ");
    // DeclareHistogram2D(DD_TAC1_PIN1, SB, SD, "TAC1 vs Pin1 dE ");
    // DeclareHistogram2D(DD_TAC1_PIN1_GATED_YSO, SB, SD, "YSO: TAC1 vs Pin1 dE ");
    // DeclareHistogram2D(DD_TAC1_PIN1_GATED_FIT, SB, SD, "FIT: TAC1 vs Pin1 dE ");

    DeclareHistogram2D(DD_TOF1_PIN0_FLIP, SA, SD, "Flipped Tof1 vs Pin0 dE ");
    DeclareHistogram2D(DD_TOF1_PIN0_GATED_YSO_FLIP, SA, SD, "YSO: Flipped Tof1 vs Pin0 dE ");

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
    bool YSO_Implant = false, FIT_Implant = false;
    if (TreeCorrelator::get()->checkPlace("pspmt_dynode_low_0")) {
        if (TreeCorrelator::get()->place("pspmt_dynode_low_0")->last().energy > yso_threshold_) {
            // cout<<"YSO_Implant energy = " << TreeCorrelator::get()->place("pspmt_dynode_low_0")->last().energy<<endl;
            YSO_Implant = true;
        };
    };
    if (TreeCorrelator::get()->checkPlace("pspmt_FIT_0")) {
        if (TreeCorrelator::get()->place("pspmt_FIT_0")->last().energy > fit_threshold_) {
            FIT_Implant = true;
            // cout<<"FIT_Implant"<<endl;
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
    static const vector<ChanEvent *> &pinCfd_vec = event.GetSummary("pid:pin_cfd", true)->GetList();

    // Function that compares energies in two ChanEvent objects
    auto compare_energy = [](ChanEvent *x1, ChanEvent *x2) { return x1->GetCalibratedEnergy() < x2->GetCalibratedEnergy(); };

    // Function that returns time in ns
    auto get_time_in_ns = [](ChanEvent *x) {
        return x->GetTimeSansCfd() * Globals::get()->GetClockInSeconds(x->GetChanID().GetModFreq()) * 1e9;
    };

    //* Tof between rfq and beamline FocalPlane */

    double tof0 = 0, tof1 = 0, pin0_energy = 0, pin1_energy = 0, tac0_energy = 0, tac1_energy = 0;

    double tof1_flip = 0;

//! TOF 0 group (RFQ->FP)
    if (!rfq_vec.empty() && !fp_vec.empty()) {
        // Get the first element in this event for RFQ
        auto rfq = rfq_vec.at(0);
        double internalTAC_Convert_Tick_adc = Globals::get()->GetAdcClockInSeconds(rfq->GetChanID().GetModFreq()) * 1e9;

        // Get elements with the largest energy in this event for fp
        auto fp = std::max_element(fp_vec.begin(), fp_vec.end(), compare_energy);
        // Check for nullptr
        if ((*fp)) {
            // Calculate tof
            tof0 = (rfq->GetTime() * internalTAC_Convert_Tick_adc) - ((*fp)->GetTime() * internalTAC_Convert_Tick_adc);
            // plot(D_TOF0, tof0);

            // ROOT outputs
            if (root_output) {
                pid_struct.rfq_time = (rfq)->GetTime() * internalTAC_Convert_Tick_adc;
                pid_struct.fp_time = (*fp)->GetTime() * internalTAC_Convert_Tick_adc;
                pid_struct.tof0 = tof0;
            }
        }
    }
    
    //! TOF 1 group (FP->Pin0)
    if (!fp_vec.empty() && !pinCfd_vec.empty()) {
        // Get elements with the largest energy in this event for fp
        auto fp = std::max_element(fp_vec.begin(), fp_vec.end(), compare_energy);
        // Check for nullptr
        double internalTAC_Convert_Tick_adc = Globals::get()->GetAdcClockInSeconds((*fp)->GetChanID().GetModFreq()) * 1e9;

        // Get elements with the largest energy in this event for pinCfd
        auto pinCfd = std::max_element(pinCfd_vec.begin(), pinCfd_vec.end(), compare_energy);
        // Check for nullptr
        if ((*pinCfd) && (*fp)) {
            // Calculate tof
            tof1 = ((*fp)->GetTime() * internalTAC_Convert_Tick_adc) - ((*pinCfd)->GetTime() * internalTAC_Convert_Tick_adc);
            tof1_flip =(((*pinCfd)->GetTime() * internalTAC_Convert_Tick_adc) - ((*fp)->GetTime() * internalTAC_Convert_Tick_adc)) + 350;
            // plot(D_TOF1, tof1);

            // ROOT outputs
            if (root_output) {
                pid_struct.fp_time = (*fp)->GetTime() * internalTAC_Convert_Tick_adc;
                pid_struct.pinCfd_time = (*pinCfd)->GetTime() * internalTAC_Convert_Tick_adc;
                pid_struct.tof1 = tof1;
            }
        }
    }

    //** TACs */
    if (!tac0_vec.empty()) {
        tac0_energy = 0;
        if (tac0_vec.size() >= 2) {
            auto tac_max = max_element(tac0_vec.begin(), tac0_vec.end(), compare_energy);
            tac0_energy = (*tac_max)->GetCalibratedEnergy();
        } else {
            tac0_energy = tac0_vec.at(0)->GetCalibratedEnergy();
        }
        if (root_output) {  // only record to root the largest one
            pid_struct.tac_0 = tac0_energy;
        }
        for (auto tac = tac0_vec.begin(); tac != tac0_vec.end(); ++tac) {
            plot(DD_TACS, 0, (*tac)->GetCalibratedEnergy());
        }
    }
    if (!tac1_vec.empty()) {
        tac1_energy = 0;
        if (tac1_vec.size() >= 2) {
            auto tac_max = max_element(tac1_vec.begin(), tac1_vec.end(), compare_energy);
            tac1_energy = (*tac_max)->GetCalibratedEnergy();
        } else {
            tac1_energy = tac1_vec.at(0)->GetCalibratedEnergy();
        }
        if (root_output) {
            pid_struct.tac_1 = tac1_energy;
        }
        for (auto tac = tac1_vec.begin(); tac != tac1_vec.end(); ++tac) {
            plot(DD_TACS, 1, (*tac)->GetCalibratedEnergy());
        }
    }

    //** Pins */

    if (!pin0_vec.empty()) {
        auto pin = max_element(pin0_vec.begin(), pin0_vec.end(), compare_energy);
        pin0_energy = (*pin)->GetCalibratedEnergy();
        plot(DD_PINS_DE, 0, pin0_energy);
        if (root_output) {
            pid_struct.pin_0_energy = pin0_energy;
            pid_struct.pin_0_time = get_time_in_ns(*pin);
        }
    }
    if (!pin1_vec.empty()) {
        auto pin = max_element(pin1_vec.begin(), pin1_vec.end(), compare_energy);
        pin1_energy = (*pin)->GetCalibratedEnergy();
        plot(DD_PINS_DE, 1, pin1_energy);
        if (root_output) {
            pid_struct.pin_1_energy = pin1_energy;
            pid_struct.pin_0_time = get_time_in_ns(*pin);
        }
    }

    // Fill the multiplicity plots
    plot(D_RFQ_MULT, rfq_vec.size());
    plot(D_FP_MULT, fp_vec.size());
    plot(DD_TACS_MULT, 0, tac0_vec.size());
    plot(DD_TACS_MULT, 1, tac1_vec.size());
    plot(DD_PINS_MULT, 0, pin0_vec.size());
    plot(DD_PINS_MULT, 1, pin1_vec.size());

    // Plot 2d histograms
    plot(DD_PIN0_1, pin0_energy, pin1_energy);

    plot(DD_TOF0_PIN0, tof0, pin0_energy);
    plot(DD_TOF0_PIN1, tof0, pin1_energy);

    plot(DD_TOF1_PIN0, tof1, pin0_energy);
    plot(DD_TOF1_PIN1, tof1, pin1_energy);

    plot(DD_TOF1_PIN0_FLIP, tof1_flip, pin0_energy);

    plot(DD_TAC0_PIN0, tac0_energy, pin0_energy);
    plot(DD_TAC0_PIN1, tac0_energy, pin1_energy);

    // plot(DD_TAC1_PIN0, tac1_energy, pin0_energy);
    // plot(DD_TAC1_PIN1, tac1_energy, pin1_energy);

    if (YSO_Implant) {
        // cout << "In YSO_Implant" << endl;
        plot(DD_TOF0_PIN0_GATED_YSO, tof0, pin0_energy);
        plot(DD_TOF0_PIN1_GATED_YSO, tof0, pin1_energy);
        plot(DD_TAC0_PIN0_GATED_YSO, tac0_energy, pin0_energy);
        plot(DD_TAC0_PIN1_GATED_YSO, tac0_energy, pin1_energy);
        // plot(DD_TAC1_PIN0_GATED_YSO, tac1_energy, pin0_energy);
        // plot(DD_TAC1_PIN1_GATED_YSO, tac1_energy, pin1_energy);
        plot(DD_TOF1_PIN0_GATED_YSO, tof1, pin0_energy);
        plot(DD_TOF1_PIN0_GATED_YSO_FLIP, tof1_flip, pin0_energy);
    }
    if (FIT_Implant) {
        plot(DD_TOF0_PIN0_GATED_FIT, tof0, pin0_energy);
        plot(DD_TOF0_PIN1_GATED_FIT, tof0, pin1_energy);
        plot(DD_TAC0_PIN0_GATED_FIT, tac0_energy, pin0_energy);
        plot(DD_TAC0_PIN1_GATED_FIT, tac0_energy, pin1_energy);
        // plot(DD_TAC1_PIN0_GATED_FIT, tac1_energy, pin0_energy);
        // plot(DD_TAC1_PIN1_GATED_FIT, tac1_energy, pin1_energy);
    }

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
