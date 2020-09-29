///@file PidProcessor.cpp
///@A dedicated processor for constructing PID information
///@author A. Keeler, R. Yokoyama
///@date July 29, 2019
// Updated for E19044 experiment.

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
#include "PidProcessor.hpp"

using namespace std;
using namespace dammIds::pid;

namespace dammIds {
    namespace pid {

        // TDiff between RFQ and beamline plastic
        const int D_TOF = 0;

        // Multiplicity of the RFQ
        const int D_RFQ_MULT = 1;

        // Multiplicity of the plastic
        const int D_PLASTIC_MULT = 2;

        // TAC energies
        const int DD_TACS = 3;

        // Multiplicities of the TACs
        const int DD_TACS_MULT = 4;

        // Energy deposits in the PINs
        const int DD_PINS_DE = 5;

        // Multiplicities of the Pins
        const int DD_PINS_MULT = 6;

        // Pin1 vs Pin2 dE
        const int DD_PIN0_1 = 7;

        // Tdiff vs Pin1 dE
        const int DD_TOF_PIN0 = 8;

        // Tdiff vs Pin2 dE
        const int DD_TOF_PIN1 = 9;
    }  // namespace pid
}  // namespace dammIds

void PidProcessor::DeclarePlots(void) {
    DeclareHistogram1D(D_TOF, SB, "Tdiff histograms");
    DeclareHistogram1D(D_RFQ_MULT, S5, "Multiplicity of RFQ");
    DeclareHistogram1D(D_PLASTIC_MULT, S5, "Multiplicity of beamline plastic");
    DeclareHistogram2D(DD_PINS_DE, S2, SB, "dE histogram of Pins");
    DeclareHistogram2D(DD_PINS_MULT, S2, S5, "Multiplicity histogram of Pins");
    DeclareHistogram2D(DD_PIN0_1, SB, SB, "Pin1 vs Pin2 dE histogram");
    DeclareHistogram2D(DD_TOF_PIN0, SB, SB, "Tdiff vs Pin1 dE histogram");
    DeclareHistogram2D(DD_TOF_PIN1, SB, SB, "Tdiff vs Pin2 dE histogram");

}  // Declare plots

PidProcessor::PidProcessor(): EventProcessor(OFFSET, RANGE, "PidProcessor") {

    associatedTypes.insert("pid");
    associatedTypes.insert("pin");
}

bool PidProcessor::PreProcess(RawEvent &event) {

    if (!EventProcessor::PreProcess(event))
    return false;

    // A flag for ROOT output
    const bool root_output = DetectorDriver::get()->GetSysRootOutput();
    
    if (root_output) {
        // Initialization of the pid_struct
        pid_struct = processor_struct::PID_DEFAULT_STRUCT;
    }

    static const vector<ChanEvent *> &pin_0_vec = event.GetSummary("pid:pin0",true)->GetList();
    static const vector<ChanEvent *> &pin_1_vec = event.GetSummary("pid:pin1",true)->GetList();
    static const vector<ChanEvent *> &tac_0_vec = event.GetSummary("pid:tac0",true)->GetList();
    static const vector<ChanEvent *> &tac_1_vec = event.GetSummary("pid:tac1",true)->GetList();
    static const vector<ChanEvent *> &rfq_vec = event.GetSummary("pid:rfq",true)->GetList();
    static const vector<ChanEvent *> &plastic_vec = event.GetSummary("pid:plastic",true)->GetList();

    // Function that compares energies in two ChanEvent objects
    auto compare_energy = [](ChanEvent* x1, ChanEvent* x2) {return x1->GetEnergy() < x2->GetEnergy(); };

    //* Tof between rfq and beamline plastic */
    if (!rfq_vec.empty() && !plastic_vec.empty()) {
        // Get the first element in this event for RFQ
        auto rfq = rfq_vec.at(0);
        // Get elements with the largest energy in this event for plastic
        auto plastic = std::max_element(plastic_vec.begin(), plastic_vec.end(), compare_energy);
        // Check for nullptr
        if (!*plastic) {
            // Calculate tof
            auto tof = rfq->GetHighResTimeInNs() - (*plastic)->GetHighResTimeInNs();
            plot(D_TOF, tof);
            // ROOT outputs
            if (root_output) {
                pid_struct.rfq_time = rfq->GetHighResTimeInNs();
                pid_struct.plastic_time = (*plastic)->GetHighResTimeInNs();
                pid_struct.tof = tof;
            }
        }
    }

    //** TACs */
    if (!tac_0_vec.empty()) {
        auto tac0 = std::max_element(tac_0_vec.begin(), tac_0_vec.end(), compare_energy);
        if (!*tac0) {
            plot(DD_TACS, 0, (*tac0)->GetCalibratedEnergy());
            if (root_output)
                pid_struct.tac_0 = (*tac0)->GetCalibratedEnergy();
        }
    }
    if (!tac_1_vec.empty()) {
        auto tac1 = std::max_element(tac_1_vec.begin(), tac_1_vec.end(), compare_energy);
        if (!*tac1) {
            plot(DD_TACS, 1, (*tac1)->GetCalibratedEnergy());
            if (root_output)
                pid_struct.tac_1 = (*tac1)->GetCalibratedEnergy();
        }
    }

    //** Pins */
    if (!pin_0_vec.empty()) {
        auto pin0 = std::max_element(pin_0_vec.begin(), pin_0_vec.end(), compare_energy);
        if (!*pin0) {
            plot(DD_PINS_DE, 0, (*pin0)->GetCalibratedEnergy());
            if (root_output)
                pid_struct.pin_0_time = (*pin0)->GetHighResTimeInNs();
                pid_struct.pin_0_energy = (*pin0)->GetCalibratedEnergy();
        }
    }
    if (!pin_1_vec.empty()) {
        auto pin1 = std::max_element(pin_1_vec.begin(), pin_1_vec.end(), compare_energy);
        if (!*pin1) {
            plot(DD_PINS_DE, 1, (*pin1)->GetCalibratedEnergy());
            if (root_output)
                pid_struct.pin_1_time = (*pin1)->GetHighResTimeInNs();
                pid_struct.pin_1_energy = (*pin1)->GetCalibratedEnergy();
        }
    }

    // Fill the multiplicity plots
    plot(D_RFQ_MULT, rfq_vec.size());
    plot(D_PLASTIC_MULT, plastic_vec.size());
    plot(DD_PINS_MULT, 0, pin_0_vec.size());
    plot(DD_PINS_MULT, 1, pin_1_vec.size());
    plot(DD_TACS_MULT, 0, tac_0_vec.size());
    plot(DD_TACS_MULT, 1, tac_1_vec.size());

    // Plot 2d histograms
    plot(DD_PIN0_1, pid_struct.pin_0_energy, pid_struct.pin_1_energy);
    plot(DD_TOF_PIN0, pid_struct.tof, pid_struct.pin_0_energy);
    plot(DD_TOF_PIN1, pid_struct.tof, pid_struct.pin_1_energy);

    if (root_output) {
        // Fill the event to the PixeTreeEvent object
        pixie_tree_event_->pid_vec_.emplace_back(pid_struct);
        pid_struct = processor_struct::PID_DEFAULT_STRUCT;
    }

    EndProcess();
    return true;
}

double PidProcessor::CorrectTofByPosition(double &tof, double &position, double &slope, double &intercept) {
    return tof - slope * position - intercept;
}

double PidProcessor::ConvertTofToAQ(double &tof){
    return tof;
}

double PidProcessor::ConvertPinToZ(double &pin){
    return pin;
}

