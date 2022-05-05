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

      const int D_TOF0 = 0;        //! TDiff image plastic L and Pin0 (MSX40)
      //const int D_RFQ_MULT = 1;    //! Multiplicity of the RFQ
      const int D_IMAGEL_MULT = 1;    //! Multiplicity of the image plastic L
      //const int D_FP_MULT = 2;     //! Multiplicity of the plastic
      const int D_DISPL_MULT = 2;     //! Multiplicity of the dispersive PPAC L
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

      const int DD_TOF0_PIN0_GATED_RIT = 40;  //! GATED ToF vs Pin0 dE
      const int DD_TOF0_PIN1_GATED_RIT = 41;  //! GATED ToF vs Pin1 dE
      const int DD_TAC0_PIN0_GATED_RIT = 42;  //! GATED TAC0 vs Pin0 dE
      const int DD_TAC1_PIN0_GATED_RIT = 43;  //! GATED TAC1 vs Pin0 dE
      const int DD_TAC0_PIN1_GATED_RIT = 44;  //! GATED TAC0 vs Pin1 dE
      const int DD_TAC1_PIN1_GATED_RIT = 45;  //! GATED TAC1 vs Pin1 dE

      //const int D_TOF1 = 26;                  //! TDiff between image plastic L and cross plastic
      //const int DD_TOF1_PIN0 = 27;            //! ToF vs Pin0 dE
      //const int DD_TOF1_PIN0_GATED_YSO = 28;  //! GATED ToF vs Pin0 dE
      //const int DD_TOF1_PIN1 = 29;            //! ToF vs Pin1 dE
      //const int DD_TOF1_PIN1_GATED_YSO = 30;  //! GATED ToF vs Pin1 dE

      const int DD_TOF0_PIN0_FLIP = 31;            //! ToF vs Pin1 dE flipped
      const int DD_TOF0_PIN0_GATED_YSO_FLIP = 32;  //! GATED ToF vs Pin1 dE flipped

      const int D_DISPLR = 33;                //! Time difference between dispersive left and right

   }  // namespace pid
}  // namespace dammIds

void PidProcessor::DeclarePlots(void) {
   // DeclareHistogram1D(D_TOF0, SD, "TOF RFQ:FP (internal)");
   // DeclareHistogram1D(D_TOF1, SD, "TOF RFQ:Pin (internal)");
   //DeclareHistogram1D(D_RFQ_MULT, S5, "Multiplicity of RFQ");
   DeclareHistogram1D(D_IMAGEL_MULT, S5, "Multiplicity of image L");
   //DeclareHistogram1D(D_FP_MULT, S5, "Multiplicity of beamline plastic");
   DeclareHistogram1D(D_DISPL_MULT, S5, "Multiplicity of dispersive PPAC L");
   DeclareHistogram2D(DD_TACS_MULT, S2, S5, "TAC multiplicity ");
   DeclareHistogram1D(D_DISPLR, SD, "Dispersive L - R");

   DeclareHistogram2D(DD_TACS, S2, SD, "TAC energy ");
   DeclareHistogram2D(DD_PINS_DE, S2, SD, "Pin dE");
   DeclareHistogram2D(DD_PINS_MULT, S2, S5, "Pins Multiplicity");
   DeclareHistogram2D(DD_PIN0_1, SC, SC, "Pin0 vs Pin1 dE ");

   DeclareHistogram2D(DD_TOF0_PIN0, SB, SD, "Tof0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF0_PIN0_GATED_YSO, SB, SD, "YSO: Tof0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF0_PIN0_GATED_FIT, SB, SD, "FIT: Tof0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF0_PIN0_GATED_RIT, SB, SD, "RIT: Tof0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF0_PIN1, SB, SD, "Tof0 vs Pin1 dE ");
   DeclareHistogram2D(DD_TOF0_PIN1_GATED_YSO, SB, SD, "YSO:Tof0 vs Pin1 dE ");
   DeclareHistogram2D(DD_TOF0_PIN1_GATED_FIT, SB, SD, "FIT:Tof0 vs Pin1 dE ");
   DeclareHistogram2D(DD_TOF0_PIN1_GATED_RIT, SB, SD, "RIT:Tof0 vs Pin1 dE ");

   //DeclareHistogram2D(DD_TOF1_PIN0, SB, SD, "Tof1 vs Pin0 dE ");
   //DeclareHistogram2D(DD_TOF1_PIN0_GATED_YSO, SB, SD, "YSO: Tof1 vs Pin0 dE ");
   // DeclareHistogram2D(DD_TOF1_PIN0_GATED_FIT, SB, SD, "FIT: Tof1 vs Pin0 dE ");
   // DeclareHistogram2D(DD_TOF1_PIN0_GATED_RIT, SB, SD, "RIT: Tof1 vs Pin0 dE ");
   //DeclareHistogram2D(DD_TOF1_PIN1, SB, SD, "Tof1 vs Pin1 dE ");
   // DeclareHistogram2D(DD_TOF1_PIN1_GATED_YSO, SB, SD, "YSO:Tof1 vs Pin1 dE ");
   // DeclareHistogram2D(DD_TOF1_PIN1_GATED_FIT, SB, SD, "FIT:Tof1 vs Pin1 dE ");
   // DeclareHistogram2D(DD_TOF1_PIN1_GATED_RIT, SB, SD, "RIT:Tof1 vs Pin1 dE ");

   DeclareHistogram2D(DD_TAC0_PIN0, SD, SD, "TAC0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TAC0_PIN0_GATED_YSO, SD, SD, "YSO: TAC0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TAC0_PIN0_GATED_FIT, SD, SD, "FIT: TAC0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TAC0_PIN0_GATED_RIT, SD, SD, "RIT: TAC0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TAC1_PIN0, SB, SD, "TAC1 vs Pin0 dE ");
   // DeclareHistogram2D(DD_TAC1_PIN0_GATED_YSO, SB, SD, "YSO: TAC1 vs Pin0 dE ");
   // DeclareHistogram2D(DD_TAC1_PIN0_GATED_FIT, SB, SD, "FIT: TAC1 vs Pin0 dE ");
   // DeclareHistogram2D(DD_TAC1_PIN0_GATED_RIT, SB, SD, "RIT: TAC1 vs Pin0 dE ");

   DeclareHistogram2D(DD_TAC0_PIN1, SB, SD, "TAC0 vs Pin1 dE ");
   DeclareHistogram2D(DD_TAC0_PIN1_GATED_YSO, SB, SD, "YSO: TAC0 vs Pin1 dE ");
   DeclareHistogram2D(DD_TAC0_PIN1_GATED_FIT, SB, SD, "FIT: TAC0 vs Pin1 dE ");
   DeclareHistogram2D(DD_TAC0_PIN1_GATED_RIT, SB, SD, "RIT: TAC0 vs Pin1 dE ");
   // DeclareHistogram2D(DD_TAC1_PIN1, SB, SD, "TAC1 vs Pin1 dE ");
   // DeclareHistogram2D(DD_TAC1_PIN1_GATED_YSO, SB, SD, "YSO: TAC1 vs Pin1 dE ");
   // DeclareHistogram2D(DD_TAC1_PIN1_GATED_FIT, SB, SD, "FIT: TAC1 vs Pin1 dE ");
   // DeclareHistogram2D(DD_TAC1_PIN1_GATED_RIT, SB, SD, "RIT: TAC1 vs Pin1 dE ");

   DeclareHistogram2D(DD_TOF0_PIN0_FLIP, SA, SD, "Flipped Tof0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF0_PIN0_GATED_YSO_FLIP, SA, SD, "YSO: Flipped Tof0 vs Pin0 dE ");

}  // Declare plots

PidProcessor::PidProcessor(const double &YSO_Implant_thresh, const double &FIT_thresh, const double &RIT_thresh) : EventProcessor(OFFSET, RANGE, "PidProcessor") {
   associatedTypes.insert("pid");
   associatedTypes.insert("pin");

   yso_threshold_ = YSO_Implant_thresh;
   fit_threshold_ = FIT_thresh;
   rit_threshold_ = RIT_thresh;
}

bool PidProcessor::PreProcess(RawEvent &event) {
   if (!EventProcessor::PreProcess(event))
      return false;

   // A flag for ROOT output
   const bool root_output = DetectorDriver::get()->GetSysRootOutput();

   //! Generate YSO implant Bool with standard tree correlator places
   bool YSO_Implant = false, FIT_Implant = false, RIT_Implant = false;
   if (TreeCorrelator::get()->checkPlace("pspmt_FIT_0")) {
      if (TreeCorrelator::get()->place("pspmt_FIT_0")->last().energy > fit_threshold_) {
         FIT_Implant = true;
         // cout<<"FIT_Implant"<<endl;
      };
   };
   if (TreeCorrelator::get()->checkPlace("pspmt_dynode_low_0")) {
      //std::cout<<"pspmt_dynoe_low_0 = "<<TreeCorrelator::get()->place("pspmt_dynode_low_0")->last().energy<<std::endl;
      if (TreeCorrelator::get()->place("pspmt_dynode_low_0")->last().energy > yso_threshold_) {
         // cout<<"YSO_Implant energy = " << TreeCorrelator::get()->place("pspmt_dynode_low_0")->last().energy<<endl;
         YSO_Implant = true;
         FIT_Implant = false;
      };
   };
   if (TreeCorrelator::get()->checkPlace("pspmt_RIT_0")) {
      if (TreeCorrelator::get()->place("pspmt_RIT_0")->last().energy > rit_threshold_) {
         RIT_Implant = true;
         YSO_Implant = false;
         FIT_Implant = false;
         // cout<<"FIT_Implant"<<endl;
      };
   };

   if (root_output) {
      // Initialization of the pid_struct
      pid_struct = processor_struct::PID_DEFAULT_STRUCT;
   }

   //static const vector<ChanEvent *> &pin0_vec = event.GetSummary("pid:pin0", true)->GetList();
   //static const vector<ChanEvent *> &pin1_vec = event.GetSummary("pid:pin1", true)->GetList();
   //static const vector<ChanEvent *> &tac0_vec = event.GetSummary("pid:tac0", true)->GetList();
   //static const vector<ChanEvent *> &tac1_vec = event.GetSummary("pid:tac1", true)->GetList();
   //static const vector<ChanEvent *> &rfq_vec = event.GetSummary("pid:rfq", true)->GetList();
   //static const vector<ChanEvent *> &fp_vec = event.GetSummary("pid:fp", true)->GetList();
   //static const vector<ChanEvent *> &pinCfd_vec = event.GetSummary("pid:pin_cfd", true)->GetList();
   static const vector<ChanEvent *> &cross_scint_b1_vec = event.GetSummary("pid:cross_scint_b1", true)->GetList();
   static const vector<ChanEvent *> &cross_scint_t1_vec = event.GetSummary("pid:cross_scint_t1", true)->GetList();
   static const vector<ChanEvent *> &cross_pin0_vec = event.GetSummary("pid:cross_pin0", true)->GetList();
   static const vector<ChanEvent *> &cross_pin1_vec = event.GetSummary("pid:cross_pin1", true)->GetList();
   static const vector<ChanEvent *> &tac0_vec = event.GetSummary("pid:tac0", true)->GetList();
   static const vector<ChanEvent *> &tac1_vec = event.GetSummary("pid:tac1", true)->GetList();
   static const vector<ChanEvent *> &tac2_vec = event.GetSummary("pid:tac2", true)->GetList();
   static const vector<ChanEvent *> &dispL_vec = event.GetSummary("pid:disp_L_logic", true)->GetList();
   static const vector<ChanEvent *> &dispR_vec = event.GetSummary("pid:disp_R_logic", true)->GetList();
   static const vector<ChanEvent *> &imageL_vec = event.GetSummary("pid:image_L_logic", true)->GetList();
   static const vector<ChanEvent *> &pinLogic_vec = event.GetSummary("pid:cross_pin0_logic", true)->GetList();

   // Function that compares energies in two ChanEvent objects
   auto compare_energy = [](ChanEvent *x1, ChanEvent *x2) { return x1->GetCalibratedEnergy() < x2->GetCalibratedEnergy(); };

   // Function that returns time in ns
   auto get_time_in_ns = [](ChanEvent *x) {
      return x->GetTimeSansCfd() * Globals::get()->GetClockInSeconds(x->GetChanID().GetModFreq()) * 1e9;
   };

   //* Tof between rfq and beamline FocalPlane */

   double tof0 = 0, pin0_energy = 0, pin1_energy = 0, tac0_energy = 0, tac1_energy = 0, tac2_energy;
   double cross_scint_b1_energy = 0, cross_scint_t1_energy = 0;
   double tof0_flip = 0;
   double disp_LR = 0;
   //std::cout<<"come here #195"<<std::endl;

   //! TOF 0 group (image L to cross pin0)
   if (!imageL_vec.empty() && !pinLogic_vec.empty()) {
      // Get elements with the largest energy in this event for image L
      auto imageL = std::max_element(imageL_vec.begin(), imageL_vec.end(), compare_energy);
      double internalTAC_Convert_Tick_adc = Globals::get()->GetAdcClockInSeconds((*imageL)->GetChanID().GetModFreq()) * 1e9;

      // Get elements with the largest energy in this event for pinLogic
      auto pinLogic = std::max_element(pinLogic_vec.begin(), pinLogic_vec.end(), compare_energy);
      // Check for nullptr
      if ((*imageL) && (*pinLogic)) {
         // Calculate tof
         tof0 = ((*imageL)->GetTime() * internalTAC_Convert_Tick_adc) - ((*pinLogic)->GetTime() * internalTAC_Convert_Tick_adc);
         tof0_flip = (((*pinLogic)->GetTime() * internalTAC_Convert_Tick_adc) - ((*imageL)->GetTime() * internalTAC_Convert_Tick_adc)) + 350;
         // plot(D_TOF0, tof0);

         // ROOT outputs
         if (root_output) {
            pid_struct.image_scint_L_logic_time = (*imageL)->GetTime() * internalTAC_Convert_Tick_adc;
            pid_struct.cross_pin_0_logic_time = (*pinLogic)->GetTime() * internalTAC_Convert_Tick_adc;
            pid_struct.tof0 = tof0;
         }
      }
   }
   //std::cout<<"come here #220"<<std::endl;

   //! dispersive L-R group (diespersive L -> R)
   if (!dispL_vec.empty() && !dispR_vec.empty()) {
      // Get elements with the largest energy in this event for dispL
      auto dispL = std::max_element(dispL_vec.begin(), dispL_vec.end(), compare_energy);
      double internalTAC_Convert_Tick_adc = Globals::get()->GetAdcClockInSeconds((*dispL)->GetChanID().GetModFreq()) * 1e9;

      // Get elements with the largest energy in this event for dispR
      auto dispR = std::max_element(dispR_vec.begin(), dispR_vec.end(), compare_energy);
      // Check for nullptr
      if ((*dispL) && (*dispR)) {
         // Calculate LR
         disp_LR = ((*dispL)->GetTime() * internalTAC_Convert_Tick_adc) - ((*dispR)->GetTime() * internalTAC_Convert_Tick_adc);

         // ROOT outputs
         if (root_output) {
            pid_struct.disp_L_logic_time = (*dispL)->GetTime() * internalTAC_Convert_Tick_adc;
            pid_struct.disp_R_logic_time = (*dispR)->GetTime() * internalTAC_Convert_Tick_adc;
            pid_struct.disp_LR = disp_LR;
         }
      }
   }
   //std::cout<<"come here #243"<<std::endl;

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
   if (!tac2_vec.empty()) {
      tac2_energy = 0;
      if (tac2_vec.size() >= 2) {
         auto tac_max = max_element(tac2_vec.begin(), tac2_vec.end(), compare_energy);
         tac2_energy = (*tac_max)->GetCalibratedEnergy();
      } else {
         tac2_energy = tac2_vec.at(0)->GetCalibratedEnergy();
      }
      if (root_output) {
         pid_struct.tac_2 = tac2_energy;
      }
      for (auto tac = tac2_vec.begin(); tac != tac2_vec.end(); ++tac) {
         plot(DD_TACS, 2, (*tac)->GetCalibratedEnergy());
      }
   }
   //std::cout<<"come here #291"<<std::endl;

   //** Pins */

   if (!cross_pin0_vec.empty()) {
      auto pin = max_element(cross_pin0_vec.begin(), cross_pin0_vec.end(), compare_energy);
      pin0_energy = (*pin)->GetCalibratedEnergy();
      plot(DD_PINS_DE, 0, pin0_energy);
      if (root_output) {
         pid_struct.cross_pin_0_energy = pin0_energy;
         pid_struct.cross_pin_0_time = get_time_in_ns(*pin);
      }
   }
   if (!cross_pin1_vec.empty()) {
      auto pin = max_element(cross_pin1_vec.begin(), cross_pin1_vec.end(), compare_energy);
      pin1_energy = (*pin)->GetCalibratedEnergy();
      plot(DD_PINS_DE, 1, pin1_energy);
      if (root_output) {
         pid_struct.cross_pin_1_energy = pin1_energy;
         pid_struct.cross_pin_1_time = get_time_in_ns(*pin);
      }
   }
   //std::cout<<"come here #313"<<std::endl;

   //** Cross plastic *//
   if (!cross_scint_b1_vec.empty()) {
      auto cross_scint = max_element(cross_scint_b1_vec.begin(), cross_scint_b1_vec.end(), compare_energy);
      cross_scint_b1_energy = (*cross_scint)->GetCalibratedEnergy();
      if (root_output) {
         pid_struct.cross_scint_b1_energy = cross_scint_b1_energy;
         pid_struct.cross_scint_t1_time = get_time_in_ns(*cross_scint);
      }
   }
   if (!cross_scint_t1_vec.empty()) {
      auto cross_scint = max_element(cross_scint_t1_vec.begin(), cross_scint_t1_vec.end(), compare_energy);
      cross_scint_t1_energy = (*cross_scint)->GetCalibratedEnergy();
      if (root_output) {
         pid_struct.cross_scint_t1_energy = cross_scint_t1_energy;
         pid_struct.cross_scint_t1_time = get_time_in_ns(*cross_scint);
      }
   }
   //std::cout<<"come here #332"<<std::endl;

   // Fill the multiplicity plots
   plot(D_IMAGEL_MULT, imageL_vec.size());
   plot(D_DISPL_MULT, dispL_vec.size());
   plot(DD_TACS_MULT, 0, tac0_vec.size());
   plot(DD_TACS_MULT, 1, tac1_vec.size());
   plot(DD_TACS_MULT, 2, tac2_vec.size());
   plot(DD_PINS_MULT, 0, cross_pin0_vec.size());
   plot(DD_PINS_MULT, 1, cross_pin1_vec.size());

   // Plot 2d histograms
   plot(DD_PIN0_1, pin0_energy, pin1_energy);

   plot(DD_TOF0_PIN0, tof0, pin0_energy);
   plot(DD_TOF0_PIN1, tof0, pin1_energy);

   //plot(DD_TOF1_PIN0, tof1, pin0_energy);
   //plot(DD_TOF1_PIN1, tof1, pin1_energy);

   plot(DD_TOF0_PIN0_FLIP, tof0_flip, pin0_energy);

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
      //plot(DD_TOF1_PIN0_GATED_YSO, tof1, pin0_energy);
      //plot(DD_TOF1_PIN0_GATED_YSO_FLIP, tof1_flip, pin0_energy);
   }
   if (FIT_Implant) {
      plot(DD_TOF0_PIN0_GATED_FIT, tof0, pin0_energy);
      plot(DD_TOF0_PIN1_GATED_FIT, tof0, pin1_energy);
      plot(DD_TAC0_PIN0_GATED_FIT, tac0_energy, pin0_energy);
      plot(DD_TAC0_PIN1_GATED_FIT, tac0_energy, pin1_energy);
      // plot(DD_TAC1_PIN0_GATED_FIT, tac1_energy, pin0_energy);
      // plot(DD_TAC1_PIN1_GATED_FIT, tac1_energy, pin1_energy);
   }
   if (RIT_Implant) {
      plot(DD_TOF0_PIN0_GATED_RIT, tof0, pin0_energy);
      plot(DD_TOF0_PIN1_GATED_RIT, tof0, pin1_energy);
      plot(DD_TAC0_PIN0_GATED_RIT, tac0_energy, pin0_energy);
      plot(DD_TAC0_PIN1_GATED_RIT, tac0_energy, pin1_energy);
      // plot(DD_TAC1_PIN0_GATED_RIT, tac1_energy, pin0_energy);
      // plot(DD_TAC1_PIN1_GATED_RIT, tac1_energy, pin1_energy);
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
