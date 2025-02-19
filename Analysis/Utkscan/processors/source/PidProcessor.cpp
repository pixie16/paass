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

      const int D_IMAGEL_MULT = 0;    //! Multiplicity of the image plastic L

      const int DD_PINS_DE = 4;    //! Energy deposits in the PINs
      const int DD_PINS_MULT = 5;  //! Multiplicities of the Pins
      const int DD_PIN0_1 = 6;     //! Pin0 vs Pin1 dE

      const int DD_TOF2_PIN1 = 14;            //! TOF2 vs Pin1 dE
      const int DD_TOF2_PIN1_GATED_FIT = 15;  //! GATED TOF2 vs Pin1 dE
      const int DD_TOF2_PIN1_GATED_YSO = 16;  //! GATED TOF2 vs Pin1 dE
      const int DD_TOF2_PIN1_GATED_RIT = 17;  //! GATED TOF2 vs Pin1 dE

      const int DD_TOF3_PIN1 = 24;            //! TOF3 vs Pin1 dE
      const int DD_TOF3_PIN1_GATED_FIT = 25;  //! GATED TOF3 vs Pin1 dE
      const int DD_TOF3_PIN1_GATED_YSO = 26;  //! GATED TOF3 vs Pin1 dE
      const int DD_TOF3_PIN1_GATED_RIT = 27;  //! GATED TOF3 vs Pin1 dE

      const int DD_TOF0_PIN0 = 30;             //! ToF vs Pin0 dE
      const int DD_TOF0_PIN0_GATED_FIT = 31;  //! GATED ToF vs Pin0 dE
      const int DD_TOF0_PIN0_GATED_YSO = 32;   //! GATED ToF vs Pin0 dE
      const int DD_TOF0_PIN0_GATED_RIT = 33;  //! GATED ToF vs Pin0 dE

      const int DD_TOF1_PIN0 = 40;            //! ToF vs Pin0 dE
      const int DD_TOF1_PIN0_GATED_FIT = 41;  //! GATED ToF vs Pin0 dE
      const int DD_TOF1_PIN0_GATED_YSO = 42;  //! GATED ToF vs Pin0 dE
      const int DD_TOF1_PIN0_GATED_RIT = 43;  //! GATED ToF vs Pin0 dE

      const int DD_TOF2_PIN0 = 50;            //! ToF vs Pin0 dE
      const int DD_TOF2_PIN0_GATED_FIT = 51;  //! GATED ToF vs Pin0 dE
      const int DD_TOF2_PIN0_GATED_YSO = 52;  //! GATED ToF vs Pin0 dE
      const int DD_TOF2_PIN0_GATED_RIT = 53;  //! GATED ToF vs Pin0 dE

      const int DD_TOF3_PIN0 = 60;            //! ToF vs Pin0 dE
      const int DD_TOF3_PIN0_GATED_FIT = 61;  //! GATED ToF vs Pin0 dE
      const int DD_TOF3_PIN0_GATED_YSO = 62;  //! GATED ToF vs Pin0 dE
      const int DD_TOF3_PIN0_GATED_RIT = 63;  //! GATED ToF vs Pin0 dE

      const int D_RANGE = 70; //implantation range gated by PID (right now hard coded)

      const int DD_PIN2_3 = 71;     //! Pin2 vs Pin3 dE
      const int DD_PIN0_2 = 72;     //! Pin0 vs Pin2 dE
      const int DD_PIN1_3 = 73;     //! Pin1 vs Pin3 dE
      const int DD_TOF4_PIN2 = 74;            //! ToF4 vs Pin2 dE
      const int DD_TOF5_PIN2 = 75;            //! ToF5 vs Pin2 dE
      const int DD_TOF4_PIN3 = 76;            //! ToF4 vs Pin3 dE
      const int DD_TOF5_PIN3 = 77;            //! ToF5 vs Pin3 dE

      const int DD_CROSS_SCINT_POS = 80;      //! position information from scint in the 2nd cross (From Ben Kreider).

   }  // namespace pid
}  // namespace dammIds

void PidProcessor::DeclarePlots(void) {
   DeclareHistogram1D(D_IMAGEL_MULT, S5, "Multiplicity of image L");
   DeclareHistogram2D(DD_PINS_DE, S2, SD, "Pin dE");
   DeclareHistogram2D(DD_PINS_MULT, S4, S3, "Pins Multiplicity");
   DeclareHistogram2D(DD_PIN0_1, SC, SC, "Pin0 vs Pin1 dE ");

   DeclareHistogram2D(DD_TOF2_PIN1, SB, SD, "TOF2 vs Pin1 dE ");
   DeclareHistogram2D(DD_TOF2_PIN1_GATED_FIT, SB, SD, "FIT: TOF2 vs Pin1 dE ");
   DeclareHistogram2D(DD_TOF2_PIN1_GATED_YSO, SB, SD, "YSO: TOF2 vs Pin1 dE ");
   DeclareHistogram2D(DD_TOF2_PIN1_GATED_RIT, SB, SD, "RIT: TOF2 vs Pin1 dE ");

   DeclareHistogram2D(DD_TOF3_PIN1, SB, SD, "TOF3 vs Pin1 dE ");
   DeclareHistogram2D(DD_TOF3_PIN1_GATED_FIT, SB, SD, "FIT: TOF3 vs Pin1 dE ");
   DeclareHistogram2D(DD_TOF3_PIN1_GATED_YSO, SB, SD, "YSO: TOF3 vs Pin1 dE ");
   DeclareHistogram2D(DD_TOF3_PIN1_GATED_RIT, SB, SD, "RIT: TOF3 vs Pin1 dE ");

   DeclareHistogram2D(DD_TOF0_PIN0, SB, SD, "Tof0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF0_PIN0_GATED_FIT, SB, SD, "FIT: Tof0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF0_PIN0_GATED_YSO, SB, SD, "YSO: Tof0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF0_PIN0_GATED_RIT, SB, SD, "RIT: Tof0 vs Pin0 dE ");

   DeclareHistogram2D(DD_TOF1_PIN0, SB, SD, "Tof1 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF1_PIN0_GATED_FIT, SB, SD, "FIT: Tof1 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF1_PIN0_GATED_YSO, SB, SD, "YSO: Tof1 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF1_PIN0_GATED_RIT, SB, SD, "RIT: Tof1 vs Pin0 dE ");

   DeclareHistogram2D(DD_TOF2_PIN0, SB, SD, "Tof2 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF2_PIN0_GATED_FIT, SB, SD, "FIT: Tof2 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF2_PIN0_GATED_YSO, SB, SD, "YSO: Tof2 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF2_PIN0_GATED_RIT, SB, SD, "RIT: Tof2 vs Pin0 dE ");

   DeclareHistogram2D(DD_TOF3_PIN0, SB, SD, "Tof3 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF3_PIN0_GATED_FIT, SB, SD, "FIT: Tof3 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF3_PIN0_GATED_YSO, SB, SD, "YSO: Tof3 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF3_PIN0_GATED_RIT, SB, SD, "RIT: Tof3 vs Pin0 dE ");

   DeclareHistogram1D(D_RANGE, S3, "Range distribution with PID gate");

   DeclareHistogram2D(DD_PIN2_3, SC, SC, "Pin2 vs Pin3 dE ");
   DeclareHistogram2D(DD_PIN0_2, SC, SC, "Pin0 vs Pin2 dE ");
   DeclareHistogram2D(DD_PIN1_3, SC, SC, "Pin1 vs Pin3 dE ");
   DeclareHistogram2D(DD_TOF4_PIN2, SB, SD, "Tof4 vs Pin2 dE ");
   DeclareHistogram2D(DD_TOF5_PIN2, SB, SD, "Tof5 vs Pin2 dE ");
   DeclareHistogram2D(DD_TOF4_PIN3, SB, SD, "Tof4 vs Pin3 dE ");
   DeclareHistogram2D(DD_TOF5_PIN3, SB, SD, "Tof5 vs Pin3 dE ");

   DeclareHistogram2D(DD_CROSS_SCINT_POS, SB, SB, "Pos. of 2nd Cross Scint ");
}  // Declare plots

PidProcessor::PidProcessor(const double &YSO_Implant_thresh, const double &FIT_thresh, const double &RIT_thresh, const bool &tofflip) : EventProcessor(OFFSET, RANGE, "PidProcessor") {
   associatedTypes.insert("pid");
   associatedTypes.insert("pin");

   yso_threshold_ = YSO_Implant_thresh;
   fit_threshold_ = FIT_thresh;
   rit_threshold_ = RIT_thresh;
   tofflip_ = tofflip;

   yso_energy_prev = 0;
   fit_energy_prev = 0;
   fit_energy_prev = 0;
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

   const ChanEvent* max_cross_scint_b1 = event.GetSummary("pid:cross_scint_b1", true)->GetMaxEvent();
   const ChanEvent* max_cross_scint_t1 = event.GetSummary("pid:cross_scint_t1", true)->GetMaxEvent();
   const ChanEvent* max_cross_scint_v1 = event.GetSummary("pid:cross_scint_v1", true)->GetMaxEvent(); // These correspond to the fp2 cross scint
   const ChanEvent* max_cross_scint_v2 = event.GetSummary("pid:cross_scint_v2", true)->GetMaxEvent();// These correspond to the fp2 cross scint
   const ChanEvent* max_cross_scint_v3 = event.GetSummary("pid:cross_scint_v3", true)->GetMaxEvent();// These correspond to the fp2 cross scint
   const ChanEvent* max_cross_scint_v4 = event.GetSummary("pid:cross_scint_v4", true)->GetMaxEvent();// These correspond to the fp2 cross scint
   const ChanEvent* max_cross_pin0 = event.GetSummary("pid:cross_pin0", true)->GetMaxEvent();
   const ChanEvent* max_cross_pin1 = event.GetSummary("pid:cross_pin1", true)->GetMaxEvent();
   const ChanEvent* max_cross_pin2 = event.GetSummary("pid:cross_pin2", true)->GetMaxEvent();
   const ChanEvent* max_cross_pin3 = event.GetSummary("pid:cross_pin3", true)->GetMaxEvent();

   const ChanEvent* max_db3_ppac_up_A = event.GetSummary("pid:db3_ppac_upstream_anode", true)->GetMaxEvent();
   const ChanEvent* max_db3_ppac_up_L = event.GetSummary("pid:db3_ppac_upstream_left", true)->GetMaxEvent();
   const ChanEvent* max_db3_ppac_up_R = event.GetSummary("pid:db3_ppac_upstream_right", true)->GetMaxEvent();
   const ChanEvent* max_db3_ppac_up_U = event.GetSummary("pid:db3_ppac_upstream_up", true)->GetMaxEvent();
   const ChanEvent* max_db3_ppac_up_D = event.GetSummary("pid:db3_ppac_upstream_down", true)->GetMaxEvent();
   const ChanEvent* max_db3_ppac_down_A = event.GetSummary("pid:db3_ppac_downstream_anode", true)->GetMaxEvent();
   const ChanEvent* max_db3_ppac_down_L = event.GetSummary("pid:db3_ppac_downstream_left", true)->GetMaxEvent();
   const ChanEvent* max_db3_ppac_down_R = event.GetSummary("pid:db3_ppac_downstream_right", true)->GetMaxEvent();
   const ChanEvent* max_db3_ppac_down_U = event.GetSummary("pid:db3_ppac_downstream_up", true)->GetMaxEvent();
   const ChanEvent* max_db3_ppac_down_D = event.GetSummary("pid:db3_ppac_downstream_down", true)->GetMaxEvent();
   const ChanEvent* max_db3_scint_L = event.GetSummary("pid:db3_scint_L", true)->GetMaxEvent();
   const ChanEvent* max_db3_scint_R = event.GetSummary("pid:db3_scint_R", true)->GetMaxEvent();

   const ChanEvent* max_db4_ppac_L = event.GetSummary("pid:db4_ppac_left", true)->GetMaxEvent();
   const ChanEvent* max_db4_ppac_R = event.GetSummary("pid:db4_ppac_right", true)->GetMaxEvent();
   const ChanEvent* max_db4_ppac_U = event.GetSummary("pid:db4_ppac_up", true)->GetMaxEvent();
   const ChanEvent* max_db4_ppac_D = event.GetSummary("pid:db4_ppac_down", true)->GetMaxEvent();

   const ChanEvent* max_db5_ppac_up_A = event.GetSummary("pid:db5_ppac_upstream_anode", true)->GetMaxEvent();
   const ChanEvent* max_db5_ppac_up_L = event.GetSummary("pid:db4_ppac_upstream_left", true)->GetMaxEvent();
   const ChanEvent* max_db5_ppac_up_R = event.GetSummary("pid:db5_ppac_upstream_right", true)->GetMaxEvent();
   const ChanEvent* max_db5_ppac_up_U = event.GetSummary("pid:db5_ppac_upstream_up", true)->GetMaxEvent();
   const ChanEvent* max_db5_ppac_up_D = event.GetSummary("pid:db5_ppac_upstream_down", true)->GetMaxEvent();
   const ChanEvent* max_db5_ppac_down_A = event.GetSummary("pid:db5_ppac_downstream_anode", true)->GetMaxEvent();
   const ChanEvent* max_db5_ppac_down_L = event.GetSummary("pid:db5_ppac_downstream_left", true)->GetMaxEvent();
   const ChanEvent* max_db5_ppac_down_R = event.GetSummary("pid:db5_ppac_downstream_right", true)->GetMaxEvent();
   const ChanEvent* max_db5_ppac_down_U = event.GetSummary("pid:db5_ppac_downstream_up", true)->GetMaxEvent();
   const ChanEvent* max_db5_ppac_down_D = event.GetSummary("pid:db5_ppac_downstream_down", true)->GetMaxEvent();

   // Function that compares energies in two ChanEvent objects
   auto compare_energy = [](ChanEvent *x1, ChanEvent *x2) { return x1->GetCalibratedEnergy() < x2->GetCalibratedEnergy(); };

   // Function that returns time in ns
   // Note: this function returns time WITHOUT Pixie onboard CFD

   //* Tof between rfq and beamline FocalPlane */

   double tof0 = 0, tof1 = 0, tof2 = 0, tof3 = 0, tof4 = 0, tof5 = 0;
   double pin0_energy = 0, pin1_energy = 0, pin2_energy = 0, pin3_energy = 0;
   double cross_scint_b1_energy = 0, cross_scint_t1_energy = 0;
   double cross_scint_v1_energy = 0, cross_scint_v2_energy = 0, cross_scint_v3_energy = 0, cross_scint_v4_energy = 0;
   double cross_scint_v1_qdc = 0, cross_scint_v2_qdc = 0, cross_scint_v3_qdc = 0, cross_scint_v4_qdc = 0;
   double tof0_flip = 0, tof1_flip = 0, tof2_flip = 0, tof3_flip = 0, tof4_flip = 0, tof5_flip = 0;
   double db3_ppac_up_LR = 0;
   double db3_ppac_up_UD = 0;
   double db3_ppac_down_LR = 0;
   double db3_ppac_down_UD = 0;
   double db4_ppac_LR = 0;
   double db4_ppac_UD = 0;
   double db5_ppac_up_LR = 0;
   double db5_ppac_up_UD = 0;
   double db5_ppac_down_LR = 0;
   double db5_ppac_down_UD = 0;
   //save the time from different detectors for TOF calculation
   double cross_scint_t1_time = -999, cross_scint_b1_time = -999;
   double cross_scint_v1_time = -999, cross_scint_v2_time = -999, cross_scint_v3_time = -999, cross_scint_v4_time = -999;
   double db3_scint_L = -999, db3_scint_R = -999;
   double db3_ppac_up = -999, db3_ppac_up_L = -999, db3_ppac_up_R = -999, db3_ppac_up_U = -999, db3_ppac_up_D = -999;
   double db3_ppac_down = -999, db3_ppac_down_L = -999, db3_ppac_down_R = -999, db3_ppac_down_U = -999, db3_ppac_down_D = -999;
   double db4_ppac_L = -999, db4_ppac_R = -999, db4_ppac_U = -999, db4_ppac_D = -999;
   double db5_ppac_up = -999, db5_ppac_up_L = -999, db5_ppac_up_R = -999, db5_ppac_up_U = -999, db5_ppac_up_D = -999;
   double db5_ppac_down = -999, db5_ppac_down_L = -999, db5_ppac_down_R = -999, db5_ppac_down_U = -999, db5_ppac_down_D = -999;
   double pin0_time = -999, pin1_time = -999, pin2_time = -999, pin3_time = -999;

   //** Cross plastic *//
   if (max_cross_scint_b1) {
      cross_scint_b1_energy = max_cross_scint_b1->GetCalibratedEnergy();
      cross_scint_b1_time =   max_cross_scint_b1->GetHighResTimeInNs();
      if (root_output) {
         pid_struct.cross_scint_b1_energy = cross_scint_b1_energy;
         pid_struct.cross_scint_b1_time = cross_scint_b1_time;
      }
   }
   if (max_cross_scint_t1) {
      cross_scint_t1_energy = max_cross_scint_t1->GetCalibratedEnergy();
      cross_scint_t1_time =   max_cross_scint_t1->GetHighResTimeInNs();
      if (root_output) {
         pid_struct.cross_scint_t1_energy = cross_scint_t1_energy;
         pid_struct.cross_scint_t1_time = cross_scint_t1_time;//in e21069B, trace is not taken for this channel/module in Gamma crate
      }
   }
   if (max_cross_scint_v1) {
      cross_scint_v1_energy = max_cross_scint_v1->GetCalibratedEnergy();
      cross_scint_v1_qdc =    max_cross_scint_v1->GetTrace().GetQdc();
      cross_scint_v1_time =   max_cross_scint_v1->GetHighResTimeInNs();
      if (root_output) {
         pid_struct.cross_scint_v1_energy = cross_scint_v1_energy;
         pid_struct.cross_scint_v1_qdc = cross_scint_v1_qdc;
         pid_struct.cross_scint_v1_time = cross_scint_v1_time;
      }
   }
   if (max_cross_scint_v2) {
      cross_scint_v2_energy = max_cross_scint_v2->GetCalibratedEnergy();
      cross_scint_v2_qdc =    max_cross_scint_v2->GetTrace().GetQdc();
      cross_scint_v2_time =   max_cross_scint_v2->GetHighResTimeInNs();
      if (root_output) {
         pid_struct.cross_scint_v2_energy = cross_scint_v2_energy;
         pid_struct.cross_scint_v2_qdc = cross_scint_v2_qdc;
         pid_struct.cross_scint_v2_time = cross_scint_v2_time;
      }
   }
   if (max_cross_scint_v3) {
      cross_scint_v3_energy = max_cross_scint_v3->GetCalibratedEnergy();
      cross_scint_v3_qdc =    max_cross_scint_v3->GetTrace().GetQdc();
      cross_scint_v3_time =   max_cross_scint_v3->GetHighResTimeInNs();
      if (root_output) {
         pid_struct.cross_scint_v3_energy = cross_scint_v3_energy;
         pid_struct.cross_scint_v3_qdc = cross_scint_v3_qdc;
         pid_struct.cross_scint_v3_time = cross_scint_v3_time;
      }
   }
   if (max_cross_scint_v4) {
      cross_scint_v4_energy = max_cross_scint_v4->GetCalibratedEnergy();
      cross_scint_v4_qdc =    max_cross_scint_v4->GetTrace().GetQdc();
      cross_scint_v4_time =   max_cross_scint_v4->GetHighResTimeInNs();
      if (root_output) {
         pid_struct.cross_scint_v4_energy = cross_scint_v4_energy;
         pid_struct.cross_scint_v4_qdc = cross_scint_v4_qdc;
         pid_struct.cross_scint_v4_time = cross_scint_v4_time;
      }
   }


   //------- PINS ---------------------------------------

   if (max_cross_pin0) {
      pin0_energy = max_cross_pin0->GetCalibratedEnergy();
      pin0_time =    max_cross_pin0->GetHighResTimeInNs();
      if(pin0_time==0){
         pin0_time = max_cross_pin0->GetTimeInNs();
      }
      plot(DD_PINS_DE, 0, pin0_energy);
      if (root_output) {
         pid_struct.cross_pin_0_energy = pin0_energy;
         pid_struct.cross_pin_0_time = pin0_time;
         if (!max_cross_pin0->GetTrace().empty()) {
            pid_struct.cross_pin_0_tracemax = max_cross_pin0->GetTrace().GetMaxInfo().second;
            pid_struct.cross_pin_0_traceqdc = max_cross_pin0->GetTrace().GetQdc();
         }
      }
   }
   if (max_cross_pin1) {
      pin1_energy = max_cross_pin1->GetCalibratedEnergy();
      pin1_time =    max_cross_pin1->GetHighResTimeInNs();
      if(pin1_time==1){
         pin1_time = max_cross_pin1->GetTimeInNs();
      }
      plot(DD_PINS_DE, 1, pin1_energy);
      if (root_output) {
         pid_struct.cross_pin_1_energy = pin1_energy;
         pid_struct.cross_pin_1_time = pin1_time;
         if (!max_cross_pin1->GetTrace().empty()) {
            pid_struct.cross_pin_1_tracemax = max_cross_pin1->GetTrace().GetMaxInfo().second;
            pid_struct.cross_pin_1_traceqdc = max_cross_pin1->GetTrace().GetQdc();
         }
      }
   }
   if (max_cross_pin2) {
      pin2_energy = max_cross_pin2->GetCalibratedEnergy();
      pin2_time =    max_cross_pin2->GetHighResTimeInNs();
      if(pin2_time==2){
         pin2_time = max_cross_pin2->GetTimeInNs();
      }
      plot(DD_PINS_DE, 2, pin2_energy);
      if (root_output) {
         pid_struct.cross_pin_2_energy = pin2_energy;
         pid_struct.cross_pin_2_time = pin2_time;
         if (!max_cross_pin2->GetTrace().empty()) {
            pid_struct.cross_pin_2_tracemax = max_cross_pin2->GetTrace().GetMaxInfo().second;
            pid_struct.cross_pin_2_traceqdc = max_cross_pin2->GetTrace().GetQdc();
         }
      }
   }
   if (max_cross_pin3) {
      pin3_energy = max_cross_pin3->GetCalibratedEnergy();
      pin3_time =    max_cross_pin3->GetHighResTimeInNs();
      if(pin3_time==3){
         pin3_time = max_cross_pin3->GetTimeInNs();
      }
      plot(DD_PINS_DE, 3, pin3_energy);
      if (root_output) {
         pid_struct.cross_pin_3_energy = pin3_energy;
         pid_struct.cross_pin_3_time = pin3_time;
         if (!max_cross_pin3->GetTrace().empty()) {
            pid_struct.cross_pin_3_tracemax = max_cross_pin3->GetTrace().GetMaxInfo().second;
            pid_struct.cross_pin_3_traceqdc = max_cross_pin3->GetTrace().GetQdc();
         }
      }
   }



   //------- DB3 Scintillator ------------------
   if (max_db3_scint_L) {
      // Get elements with the largest energy in this event for image L
      db3_scint_L = max_db3_scint_L->GetHighResTimeInNs();
   }
   if (max_db3_scint_R) {
      // Get elements with the largest energy in this event for image L
      db3_scint_R = max_db3_scint_R->GetHighResTimeInNs();
   }


   //------- DB3 Upstream PPAC ------------------
   if (max_db3_ppac_up_A) {
      db3_ppac_up = max_db3_ppac_up_A->GetHighResTimeInNs();
   }
   if (max_db3_ppac_up_L) {
      db3_ppac_up_L = max_db3_ppac_up_L->GetHighResTimeInNs();
   }
   if (max_db3_ppac_up_R) {
      db3_ppac_up_R = max_db3_ppac_up_R->GetHighResTimeInNs();
   }
   if(db3_ppac_up_L>0 && db3_ppac_up_R>0){
      db3_ppac_up_LR = db3_ppac_up_L - db3_ppac_up_R;
   }
   if (max_db3_ppac_up_U) {
      db3_ppac_up_U = max_db3_ppac_up_U->GetHighResTimeInNs();
   }
   if (max_db3_ppac_up_D) {
      db3_ppac_up_D = max_db3_ppac_up_D->GetHighResTimeInNs();
   }
   if(db3_ppac_up_U>0 && db3_ppac_up_D>0){
      db3_ppac_up_UD = db3_ppac_up_U - db3_ppac_up_D;
   }



   //------- DB3 Downstream PPAC ------------------
   if (max_db3_ppac_down_A) {
      db3_ppac_down = max_db3_ppac_down_A->GetHighResTimeInNs();
   }
   if (max_db3_ppac_down_L) {
      db3_ppac_down_L = max_db3_ppac_down_L->GetHighResTimeInNs();
   }
   if (max_db3_ppac_down_R) {
      db3_ppac_down_R = max_db3_ppac_down_R->GetHighResTimeInNs();
   }
   if(db3_ppac_down_L>0 && db3_ppac_down_R>0){
      db3_ppac_down_LR = db3_ppac_down_L - db3_ppac_down_R;
   }
   if (max_db3_ppac_down_U) {
      db3_ppac_down_U = max_db3_ppac_down_U->GetHighResTimeInNs();
   }
   if (max_db3_ppac_down_D) {
      db3_ppac_down_D = max_db3_ppac_down_D->GetHighResTimeInNs();
   }
   if(db3_ppac_down_U>0 && db3_ppac_down_D>0){
      db3_ppac_down_UD = db3_ppac_down_U - db3_ppac_down_D;
   }


   //------- DB4 Downstream PPAC ------------------
   if (max_db4_ppac_L) {
      db4_ppac_L = max_db4_ppac_L->GetHighResTimeInNs();
   }
   if (max_db4_ppac_R) {
      db4_ppac_R = max_db4_ppac_R->GetHighResTimeInNs();
   }
   if(db4_ppac_L>0 && db4_ppac_R>0){
      db4_ppac_LR = db4_ppac_L - db4_ppac_R;
   }
   if (max_db4_ppac_U) {
      db4_ppac_U = max_db4_ppac_U->GetHighResTimeInNs();
   }
   if (max_db4_ppac_D) {
      db4_ppac_D = max_db4_ppac_D->GetHighResTimeInNs();
   }
   if(db4_ppac_U>0 && db4_ppac_D>0){
      db4_ppac_UD = db4_ppac_U - db4_ppac_D;
   }


   //------- DB3 Upstream PPAC ------------------
   if (max_db5_ppac_up_A) {
      db5_ppac_up = max_db5_ppac_up_A->GetHighResTimeInNs();
   }
   if (max_db5_ppac_up_L) {
      db5_ppac_up_L = max_db5_ppac_up_L->GetHighResTimeInNs();
   }
   if (max_db5_ppac_up_R) {
      db5_ppac_up_R = max_db5_ppac_up_R->GetHighResTimeInNs();
   }
   if(db5_ppac_up_L>0 && db5_ppac_up_R>0){
      db5_ppac_up_LR = db5_ppac_up_L - db5_ppac_up_R;
   }
   if (max_db5_ppac_up_U) {
      db5_ppac_up_U = max_db5_ppac_up_U->GetHighResTimeInNs();
   }
   if (max_db5_ppac_up_D) {
      db5_ppac_up_D = max_db5_ppac_up_D->GetHighResTimeInNs();
   }
   if(db5_ppac_up_U>0 && db5_ppac_up_D>0){
      db5_ppac_up_UD = db5_ppac_up_U - db5_ppac_up_D;
   }



   //------- DB3 Downstream PPAC ------------------
   if (max_db5_ppac_down_A) {
      db5_ppac_down = max_db5_ppac_down_A->GetHighResTimeInNs();
   }
   if (max_db5_ppac_down_L) {
      db5_ppac_down_L = max_db5_ppac_down_L->GetHighResTimeInNs();
   }
   if (max_db5_ppac_down_R) {
      db5_ppac_down_R = max_db5_ppac_down_R->GetHighResTimeInNs();
   }
   if(db5_ppac_down_L>0 && db5_ppac_down_R>0){
      db5_ppac_down_LR = db5_ppac_down_L - db5_ppac_down_R;
   }
   if (max_db5_ppac_down_U) {
      db5_ppac_down_U = max_db5_ppac_down_U->GetHighResTimeInNs();
   }
   if (max_db5_ppac_down_D) {
      db5_ppac_down_D = max_db5_ppac_down_D->GetHighResTimeInNs();
   }
   if(db5_ppac_down_U>0 && db5_ppac_down_D>0){
      db5_ppac_down_UD = db5_ppac_down_U - db5_ppac_down_D;
   }



   //! TOF 0 group (db3 ppac downstream to cross plastic)
   if (db3_ppac_down>0 && cross_scint_t1_time>0) {
      tof0 = cross_scint_t1_time - db3_ppac_down;
      tof0_flip = db3_ppac_down - cross_scint_t1_time;
   }else if (db3_ppac_down>0 && cross_scint_b1_time>0) {
      tof0 = cross_scint_b1_time - db3_ppac_down;
      tof0_flip = db3_ppac_down - cross_scint_b1_time;
   }
   //! TOF 1 group (image L to cross pin0 highrestime)
   if (db3_scint_L>0 && pin0_time>0) {
      tof1 = pin0_time - db3_scint_L;
      tof1_flip = db3_scint_L - pin0_time;
   }
   //
   //! TOF 3 group (ppac db4 to cross scint highrestime)
   if (db3_scint_L>0 && cross_scint_b1_time>0) {
      tof3 = cross_scint_b1_time - db3_scint_L;
      tof3_flip = db3_scint_L - cross_scint_b1_time;
   }else if (db3_scint_L>0 && cross_scint_t1_time>0) {
      tof3 = cross_scint_t1_time - db3_scint_L;
      tof3_flip = db3_scint_L - cross_scint_t1_time;
   }
   //////////////// TOF 4,5 are for Cross 2
   //! TOF 4 group (image L to cross pin2 highrestime)
   if (db3_scint_L>0 && pin2_time>0) {
      // Get elements with the largest energy in this event for db3_scint_ L
      tof4 = pin2_time - db3_scint_L;
      tof4_flip = db3_scint_L - pin2_time;
   }
   //! TOF 5 group (db3 ppac downstream to cross plastic MTAS crate FP1 logic)
   if (db3_ppac_down>0 && cross_scint_v1_time>0 && cross_scint_v2_time>0 && cross_scint_v3_time>0 && cross_scint_v4_time>0) {
      tof5 = 0.25*(cross_scint_v1_time+cross_scint_v2_time+cross_scint_v3_time+cross_scint_v4_time) - db3_ppac_down;
      tof5_flip = db3_ppac_down - 0.25*(cross_scint_v1_time+cross_scint_v2_time+cross_scint_v3_time+cross_scint_v4_time);
   }
   if(root_output){
      pid_struct.db3_scint_L = db3_scint_L;
      pid_struct.db3_scint_R = db3_scint_R;
      pid_struct.db3_ppac_upstream = db3_ppac_up;
      pid_struct.db3_ppac_upstream_left = db3_ppac_up_L;
      pid_struct.db3_ppac_upstream_right = db3_ppac_up_R;
      pid_struct.db3_ppac_upstream_LR = db3_ppac_up_LR;
      pid_struct.db3_ppac_upstream_up = db3_ppac_up_U;
      pid_struct.db3_ppac_upstream_down = db3_ppac_up_D;
      pid_struct.db3_ppac_upstream_UD = db3_ppac_up_UD;
      pid_struct.db3_ppac_downstream = db3_ppac_down;
      pid_struct.db3_ppac_downstream_left = db3_ppac_down_L;
      pid_struct.db3_ppac_downstream_right = db3_ppac_down_R;
      pid_struct.db3_ppac_downstream_LR = db3_ppac_down_LR;
      pid_struct.db3_ppac_downstream_down = db3_ppac_down_U;
      pid_struct.db3_ppac_downstream_down = db3_ppac_down_D;
      pid_struct.db3_ppac_downstream_UD = db3_ppac_down_UD;
      pid_struct.db4_ppac_left = db4_ppac_L;
      pid_struct.db4_ppac_right = db4_ppac_R;
      pid_struct.db4_ppac_LR = db4_ppac_LR;
      pid_struct.db4_ppac_down = db4_ppac_U;
      pid_struct.db4_ppac_down = db4_ppac_D;
      pid_struct.db4_ppac_UD = db4_ppac_UD;
      pid_struct.db5_ppac_upstream = db5_ppac_up;
      pid_struct.db5_ppac_upstream_left = db5_ppac_up_L;
      pid_struct.db5_ppac_upstream_right = db5_ppac_up_R;
      pid_struct.db5_ppac_upstream_LR = db5_ppac_up_LR;
      pid_struct.db5_ppac_upstream_up = db5_ppac_up_U;
      pid_struct.db5_ppac_upstream_down = db5_ppac_up_D;
      pid_struct.db5_ppac_upstream_UD = db5_ppac_up_UD;
      pid_struct.db5_ppac_downstream = db5_ppac_down;
      pid_struct.db5_ppac_downstream_left = db5_ppac_down_L;
      pid_struct.db5_ppac_downstream_right = db5_ppac_down_R;
      pid_struct.db5_ppac_downstream_LR = db5_ppac_down_LR;
      pid_struct.db5_ppac_downstream_down = db5_ppac_down_U;
      pid_struct.db5_ppac_downstream_down = db5_ppac_down_D;
      pid_struct.db5_ppac_downstream_UD = db5_ppac_down_UD;

      pid_struct.tof0 = tof0;
      pid_struct.tof1 = tof1;
      pid_struct.tof3 = tof3;
      pid_struct.tof4 = tof4;
      pid_struct.tof5 = tof5;
   }

   //make the variables proper for DAMM plot (not ROOT file)
   if(tof0!=0){
      tof0 = tof0*10+11000;
      tof0_flip = tof0_flip*10-7100;
   }else{
      tof0 = -999;
      tof0_flip = -999;
   }
   if(tof1!=0){
      tof1 = tof1*10+10500;
      tof1_flip = tof1_flip*10-7500;
   }else{
      tof1 = -999;
      tof1_flip = -999;
   }
   if(tof2!=0){
      tof2 = tof2*50+50000;
      tof2_flip = tof2_flip*10-7500;
   }else{
      tof2 = -999;
      tof2_flip = -999;
   }
   if(tof3!=0){
      tof3 = tof3*50+44500;
      tof3_flip = tof3_flip*50-43500;
   }else{
      tof3 = -999;
      tof3_flip = -999;
   }
   if(tof4!=0){
      tof4 = tof4*50+49500; // I think these parameters might need to be changed
      tof4_flip = tof4_flip*50-47000;
   }else{
      tof4 = -999;
      tof4_flip = -999;
   }
   if(tof5!=0){
      tof5 = tof5*45+47500; // I think these parameters might need to be changed
      tof5_flip = tof5_flip*45-45.6e3;
   }else{
      tof5 = -999;
      tof5_flip = -999;
   }
   if(db4_ppac_LR!=0){
      db4_ppac_LR = db4_ppac_LR*50+5000;
   }else{
      db4_ppac_LR = -999;
   }
   if(db4_ppac_UD!=0){
      db4_ppac_UD = db4_ppac_UD*100+500;
   }else{
      db4_ppac_UD = -999;
   }

   //will add more here
   //////////////////////////////////////

   TString stop_in = "";
   if(cross_scint_t1_energy>0 || cross_scint_b1_energy>0){
      stop_in = "scint";
   }
   if(pin0_energy>0){
      stop_in = "pin0";
   }
   if(pin1_energy>0){
      stop_in = "pin1";
   }
   double fit_energy = 0, yso_energy = 0, rit_energy=0;
   bool YSO_Implant = false, FIT_Implant = false, RIT_Implant = false;
   if (TreeCorrelator::get()->checkPlace("pspmt_FIT_0")) {
      fit_energy = TreeCorrelator::get()->place("pspmt_FIT_0")->last().energy;
      if(abs(fit_energy-fit_energy_prev)<1e-3){
         fit_energy = 0;
      }else{
         fit_energy_prev = fit_energy;
         if (fit_energy > fit_threshold_) {
            FIT_Implant = true;
            // cout<<"FIT_Implant"<<endl;
            stop_in = "FIT";
         }
      }
   }
   if (TreeCorrelator::get()->checkPlace("pspmt_dynode_low_0")) {
      //std::cout<<"pspmt_dynoe_low_0 = "<<TreeCorrelator::get()->place("pspmt_dynode_low_0")->last().energy<<std::endl;
      yso_energy = TreeCorrelator::get()->place("pspmt_dynode_low_0")->last().energy;
      if(abs(yso_energy-yso_energy_prev)<1e-3){
         yso_energy = 0;
      }else{
         yso_energy_prev = yso_energy;
         if (yso_energy > yso_threshold_) {
            // cout<<"YSO_Implant energy = " << TreeCorrelator::get()->place("pspmt_dynode_low_0")->last().energy<<endl;
            YSO_Implant = true;
            FIT_Implant = false;
            stop_in = "YSO";
         }
      }
   }else if(TreeCorrelator::get()->checkPlace("mtasimplantsipm_dyn_l_0")) {
      //std::cout<<"pspmt_dynoe_low_0 = "<<TreeCorrelator::get()->place("pspmt_dynode_low_0")->last().energy<<std::endl;
      yso_energy = TreeCorrelator::get()->place("mtasimplantsipm_dyn_l_0")->last().energy;
      if(abs(yso_energy-yso_energy_prev)<1e-3){
         yso_energy = 0;
      }else{
         yso_energy_prev = yso_energy;
         if (yso_energy > yso_threshold_) {
            // cout<<"YSO_Implant energy = " << TreeCorrelator::get()->place("pspmt_dynode_low_0")->last().energy<<endl;
            YSO_Implant = true;
            FIT_Implant = false;
            stop_in = "YSO";
         }
      }
   }
   if (TreeCorrelator::get()->checkPlace("pspmt_RIT_0")) {
      rit_energy = TreeCorrelator::get()->place("pspmt_RIT_0")->last().energy;
      if(abs(rit_energy-rit_energy_prev)<1e-3){
         rit_energy = 0;
      }else{
         rit_energy_prev = rit_energy;
         if (rit_energy > rit_threshold_) {
            RIT_Implant = true;
            YSO_Implant = false;
            FIT_Implant = false;
            stop_in = "RIT";
         }
      }
   }

   pid_struct.stop_in = stop_in;
   pid_struct.fit_energy = fit_energy;
   pid_struct.yso_energy = yso_energy;
   pid_struct.rit_energy = rit_energy;

   // Fill the plots
   plot(DD_PINS_MULT, 0, event.GetSummary("pid:cross_pin0")->GetMult());
   plot(DD_PINS_MULT, 1, event.GetSummary("pid:cross_pin1")->GetMult());
   plot(DD_PINS_MULT, 2, event.GetSummary("pid:cross_pin2")->GetMult());
   plot(DD_PINS_MULT, 3, event.GetSummary("pid:cross_pin3")->GetMult());
   plot(DD_PIN0_1, pin0_energy, pin1_energy);
   plot(DD_PIN2_3, pin2_energy, pin3_energy);
   plot(DD_PIN0_2, pin0_energy, pin2_energy);
   plot(DD_PIN1_3, pin1_energy, pin3_energy);

   plot(DD_TOF2_PIN1, tof2, pin1_energy);
   plot(DD_TOF3_PIN1, tof3, pin1_energy);
   if(!tofflip_){
      plot(DD_TOF0_PIN0, tof0, pin0_energy);
      plot(DD_TOF1_PIN0, tof1, pin0_energy);
      plot(DD_TOF2_PIN0, tof2, pin0_energy);
      plot(DD_TOF3_PIN0, tof3, pin0_energy);
   }else{
      plot(DD_TOF0_PIN0, tof0_flip, pin0_energy);
      plot(DD_TOF1_PIN0, tof1_flip, pin0_energy);
      plot(DD_TOF2_PIN0, tof2_flip, pin0_energy);
      plot(DD_TOF3_PIN0, tof3_flip, pin0_energy);
   }

   if(!tofflip_){
      plot(DD_TOF4_PIN2, tof4, pin2_energy);
      plot(DD_TOF5_PIN2, tof5, pin2_energy);
      plot(DD_TOF4_PIN3, tof4, pin3_energy);
      plot(DD_TOF5_PIN3, tof5, pin3_energy);
   }else{
      plot(DD_TOF4_PIN2, tof4_flip, pin2_energy);
      plot(DD_TOF5_PIN2, tof5_flip, pin2_energy);
      plot(DD_TOF4_PIN3, tof4_flip, pin3_energy);
      plot(DD_TOF5_PIN3, tof5_flip, pin3_energy);
   }

   if (FIT_Implant) {
      plot(DD_TOF2_PIN1_GATED_FIT, tof2, pin1_energy);
      plot(DD_TOF3_PIN1_GATED_FIT, tof3, pin1_energy);
      if(!tofflip_){
         plot(DD_TOF0_PIN0_GATED_FIT, tof0, pin0_energy);
         plot(DD_TOF1_PIN0_GATED_FIT, tof1, pin0_energy);
         plot(DD_TOF2_PIN0_GATED_FIT, tof2, pin0_energy);
         plot(DD_TOF3_PIN0_GATED_FIT, tof3, pin0_energy);
      }else{
         plot(DD_TOF0_PIN0_GATED_FIT, tof0_flip, pin0_energy);
         plot(DD_TOF1_PIN0_GATED_FIT, tof1_flip, pin0_energy);
         plot(DD_TOF2_PIN0_GATED_FIT, tof2_flip, pin0_energy);
         plot(DD_TOF3_PIN0_GATED_FIT, tof3_flip, pin0_energy);
      }
   }
   if (YSO_Implant) {
      plot(DD_TOF2_PIN1_GATED_YSO, tof2, pin1_energy);
      plot(DD_TOF3_PIN1_GATED_YSO, tof3, pin1_energy);
      if(!tofflip_){
         plot(DD_TOF0_PIN0_GATED_YSO, tof0, pin0_energy);
         plot(DD_TOF1_PIN0_GATED_YSO, tof1, pin0_energy);
         plot(DD_TOF2_PIN0_GATED_YSO, tof2, pin0_energy);
         plot(DD_TOF3_PIN0_GATED_YSO, tof3, pin0_energy);
      }else{
         plot(DD_TOF0_PIN0_GATED_YSO, tof0_flip, pin0_energy);
         plot(DD_TOF1_PIN0_GATED_YSO, tof1_flip, pin0_energy);
         plot(DD_TOF2_PIN0_GATED_YSO, tof2_flip, pin0_energy);
         plot(DD_TOF3_PIN0_GATED_YSO, tof3_flip, pin0_energy);
      }
   }
   if (RIT_Implant) {
      plot(DD_TOF2_PIN1_GATED_RIT, tof2, pin1_energy);
      plot(DD_TOF3_PIN1_GATED_RIT, tof3, pin1_energy);
      if(!tofflip_){
         plot(DD_TOF0_PIN0_GATED_RIT, tof0, pin0_energy);
         plot(DD_TOF1_PIN0_GATED_RIT, tof1, pin0_energy);
         plot(DD_TOF2_PIN0_GATED_RIT, tof2, pin0_energy);
         plot(DD_TOF3_PIN0_GATED_RIT, tof3, pin0_energy);
      }else{
         plot(DD_TOF0_PIN0_GATED_RIT, tof0_flip, pin0_energy);
         plot(DD_TOF1_PIN0_GATED_RIT, tof1_flip, pin0_energy);
         plot(DD_TOF2_PIN0_GATED_RIT, tof2_flip, pin0_energy);
         plot(DD_TOF3_PIN0_GATED_RIT, tof3_flip, pin0_energy);
      }
   }

   if(tof3>0 && tof3<1600 && pin1_energy>500 && pin1_energy<5000){
      if(stop_in=="scint"){
         plot(D_RANGE, 1);
      }else if(stop_in=="pin0"){
         plot(D_RANGE, 2);
      }else if(stop_in=="pin1"){
         plot(D_RANGE, 3);
      }else if(stop_in=="FIT"){
         plot(D_RANGE, 4);
      }else if(stop_in=="YSO"){
         plot(D_RANGE, 5);
      }else if(stop_in=="RIT"){
         plot(D_RANGE, 6);
      }
   }

   pair<double, double> scint_pos = GetCrossScintPosition(cross_scint_v4_qdc,cross_scint_v1_qdc,cross_scint_v2_qdc,cross_scint_v3_qdc);
   double pos_scale = 512;
   double pos_offset = 1024;
   double scint_x = scint_pos.first;
   double scint_y = scint_pos.second;
   if(scint_x>-2 && scint_x<2 && scint_y>-2 && scint_y<2){
      plot(DD_CROSS_SCINT_POS, scint_x*pos_scale+pos_offset, scint_y*pos_scale+pos_offset);
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

pair<double, double> PidProcessor::GetCrossScintPosition(double qdc1, double qdc2, double qdc3, double qdc4){
   double xcorr=-999., ycorr=-999.;
   if(qdc1>0 && qdc2>0 && qdc3>0 && qdc4>0){
      double a = 0.85;
      double b = 2.38;
      double c = -2.15;
      double d = 0.53;
      double e = 0.16;
      double f = -1.21;
      double S = 6.2;
      double A1 = 4.00;
      double A2 = 4.00;
      double A3 = 4.00;
      double A4 = 4.00;
      double xcm = (-qdc1/A1+qdc2/A2-qdc3/A3+qdc4/A4)/(qdc1/A1+qdc2/A2+qdc3/A3+qdc4/A4);
      double ycm = (+qdc1/A1+qdc2/A2-qdc3/A3-qdc4/A4)/(qdc1/A1+qdc2/A2+qdc3/A3+qdc4/A4);
      double r = sqrt(pow(xcm,2)+pow(ycm,2));
      double r2 = pow(r,2);
      double r3 = r*r2;
      double r4 = pow(r2,2);
      double theta = atan(ycm/xcm) + (xcm<0? M_PI : 0);
      double cos1 = cos(theta);
      double cos3 = pow(cos1,3);
      double sin1 = sin(theta);
      double sin3 = pow(sin1,3);
      xcorr = S*xcm/(1+a*r+b*r2+c*r3+d*r4) + r*(e*cos1+f*cos3);
      ycorr = S*ycm/(1+a*r+b*r2+c*r3+d*r4) + r*(e*sin1+f*sin3);
   }
   return make_pair(xcorr, ycorr);
}
