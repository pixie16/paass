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
        const int D_TACS = 3;

        // Multiplicities of the TACs
        const int D_TACS_MULT = 4;

        // Energy deposits in the PINs
        const int DD_PINS_DE = 5;

        // Multiplicities of the Pins
        const int DD_PINS_MULT = 6;

        // Pin0 vs Pin1 dE
        const int DD_PIN0_1 = 7;

        // Tdiff vs Pin0 dE
        const int DD_TOF_PIN0 = 8;

        // Tdiff vs Pin1 dE
        const int DD_TOF_PIN1 = 9;

        // TAC0 vs Pin0 dE
        const int DD_TAC_PIN0 = 10;

        // TAC0 vs Pin1 dE
        const int DD_TAC_PIN1 = 11;
    }  // namespace pid
}  // namespace dammIds

void PidProcessor::DeclarePlots(void) {
    DeclareHistogram1D(D_TOF, SB, "Tdiff histograms");
    DeclareHistogram1D(D_RFQ_MULT, S5, "Multiplicity of RFQ");
    DeclareHistogram1D(D_PLASTIC_MULT, S5, "Multiplicity of beamline plastic");
    DeclareHistogram1D(D_TACS, SB, "TAC histogram");
    DeclareHistogram1D(D_TACS_MULT, S5, "TAC multiplicity histogram");
    DeclareHistogram2D(DD_PINS_DE, S2, SB, "dE histogram of Pins");
    DeclareHistogram2D(DD_PINS_MULT, S2, S5, "Multiplicity histogram of Pins");
    DeclareHistogram2D(DD_PIN0_1, SB, SB, "Pin0 vs Pin1 dE histogram");
    DeclareHistogram2D(DD_TOF_PIN0, SB, SB, "Tdiff vs Pin0 dE histogram");
    DeclareHistogram2D(DD_TOF_PIN1, SB, SB, "Tdiff vs Pin1 dE histogram");
    DeclareHistogram2D(DD_TAC_PIN0, SB, SB, "TAC vs Pin0 dE histogram");
    DeclareHistogram2D(DD_TAC_PIN1, SB, SB, "TAC vs Pin1 dE histogram");

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

    static const vector<ChanEvent *> &pin_vec = event.GetSummary("pid:pin",true)->GetList();
    static const vector<ChanEvent *> &tac_vec = event.GetSummary("pid:tac",true)->GetList();
    static const vector<ChanEvent *> &rfq_vec = event.GetSummary("pid:rfq",true)->GetList();
    static const vector<ChanEvent *> &plastic_vec = event.GetSummary("pid:plastic",true)->GetList();

    // A set of locations for Pins
    const auto pin_locations = DetectorLibrary::get()->GetLocations("pid", "pin");

    // Function that compares energies in two ChanEvent objects
    auto compare_energy = [](ChanEvent* x1, ChanEvent* x2) {return x1->GetEnergy() < x2->GetEnergy(); };
    
    // Function that returns time in ns
    auto get_time_in_ns = [](ChanEvent* x) {
        return x->GetTimeSansCfd() * Globals::get()->GetClockInSeconds(x->GetChanID().GetModFreq()) * 1e9;
    };

    //* Tof between rfq and beamline plastic */
    if (!rfq_vec.empty() && !plastic_vec.empty()) {
        // Get the first element in this event for RFQ
        auto rfq = rfq_vec.at(0);
        // Get elements with the largest energy in this event for plastic
        auto plastic = std::max_element(plastic_vec.begin(), plastic_vec.end(), compare_energy);
        // Check for nullptr
        if (!*plastic) {
            // Calculate tof
            auto tof = get_time_in_ns(rfq) - get_time_in_ns(*plastic);
            plot(D_TOF, tof);
            // ROOT outputs
            if (root_output) {
                pid_struct.rfq_time = get_time_in_ns(rfq);
                pid_struct.plastic_time = get_time_in_ns(*plastic);
                pid_struct.tof = tof;
            }
        }
    }

    //** TACs */
    if (!tac_vec.empty()) {
            auto tac = std::max_element(tac_vec.begin(), tac_vec.end(), compare_energy);
            if (!*tac) {
                plot(D_TACS, (*tac)->GetCalibratedEnergy());
                if (root_output)
                    pid_struct.tac = (*tac)->GetCalibratedEnergy();
            }
    }

    //** Pins */
    if (!pin_vec.empty()) {
        for (const auto& location : pin_locations) {
            std::vector<ChanEvent*> pin_i_vec;
            std::copy_if(pin_vec.begin(), pin_vec.end(), std::back_inserter(pin_i_vec),
                [location](ChanEvent* x) {return x->GetChanID().GetLocation() == (unsigned int)location; });
            auto pin = std::max_element(pin_i_vec.begin(), pin_i_vec.end(), compare_energy);
            if (!*pin) {
                if (root_output) {
                    pid_struct.pin_0_time = get_time_in_ns(*pin);
                    pid_struct.pin_0_energy = (*pin)->GetCalibratedEnergy();
                }
                plot(DD_PINS_DE, location, (*pin)->GetCalibratedEnergy());
                plot(DD_PINS_MULT, location, pin_i_vec.size());
            }
        }
    }

    // Fill the multiplicity plots
    plot(D_RFQ_MULT, rfq_vec.size());
    plot(D_PLASTIC_MULT, plastic_vec.size());
    plot(D_TACS_MULT, 0, tac_vec.size());

    // Plot 2d histograms
    plot(DD_PIN0_1, pid_struct.pin_0_energy, pid_struct.pin_1_energy);
    plot(DD_TOF_PIN0, pid_struct.tof, pid_struct.pin_0_energy);
    plot(DD_TOF_PIN1, pid_struct.tof, pid_struct.pin_1_energy);
    plot(DD_TAC_PIN0, pid_struct.tac, pid_struct.pin_0_energy);
    plot(DD_TAC_PIN1, pid_struct.tac, pid_struct.pin_1_energy);

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

