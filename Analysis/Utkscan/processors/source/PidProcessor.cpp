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
      const int D_DISPL_MULT = 1;     //! Multiplicity of the dispersive PPAC L
      const int DD_TACS_MULT = 2;  //! Multiplicities of the TACs
      const int DD_TACS = 3;       //! TAC energies
      const int DD_PINS_DE = 4;    //! Energy deposits in the PINs
      const int DD_PINS_MULT = 5;  //! Multiplicities of the Pins
      const int DD_PIN0_1 = 6;     //! Pin0 vs Pin1 dE
      const int D_DISPLR = 7;                //! Time difference between dispersive left and right
      const int D_DISPUD = 8;                //! Time difference between dispersive up and down
      const int DD_DISP_PLANE = 9;                //! 2-D image of dipersive up-down vs left-right

      const int DD_TAC0_PIN0 = 10;            //! TAC0 vs Pin0 dE
      const int DD_TAC0_PIN0_GATED_FIT = 11;  //! GATED TAC0 vs Pin0 dE
      const int DD_TAC0_PIN0_GATED_YSO = 12;  //! GATED TAC0 vs Pin0 dE
      const int DD_TAC0_PIN0_GATED_RIT = 13;  //! GATED TAC0 vs Pin0 dE
      const int DD_TOF2_PIN1 = 14;            //! TAC0 vs Pin1 dE
      const int DD_TOF2_PIN1_GATED_FIT = 15;  //! GATED TAC0 vs Pin1 dE
      const int DD_TOF2_PIN1_GATED_YSO = 16;  //! GATED TAC0 vs Pin1 dE
      const int DD_TOF2_PIN1_GATED_RIT = 17;  //! GATED TAC0 vs Pin1 dE

      const int DD_TAC1_PIN0 = 20;            //! TAC0 vs Pin0 dE
      const int DD_TAC1_PIN0_GATED_FIT = 21;  //! GATED TAC1 vs Pin0 dE
      const int DD_TAC1_PIN0_GATED_YSO = 22;  //! YSO gated TAC0 vs Pin0 dE
      const int DD_TAC1_PIN0_GATED_RIT = 23;  //! GATED TAC1 vs Pin0 dE
      const int DD_TOF3_PIN1 = 24;            //! TAC1 vs Pin1 dE
      const int DD_TOF3_PIN1_GATED_FIT = 25;  //! GATED TAC1 vs Pin1 dE
      const int DD_TOF3_PIN1_GATED_YSO = 26;  //! GATED TAC1 vs Pin1 dE
      const int DD_TOF3_PIN1_GATED_RIT = 27;  //! GATED TAC1 vs Pin1 dE

      const int DD_TOF0_PIN0 = 30;             //! ToF vs Pin0 dE
      const int DD_TOF0_PIN0_GATED_FIT = 31;  //! GATED ToF vs Pin0 dE
      const int DD_TOF0_PIN0_GATED_YSO = 32;   //! GATED ToF vs Pin0 dE
      const int DD_TOF0_PIN0_GATED_RIT = 33;  //! GATED ToF vs Pin0 dE
      const int DD_TOF0_PIN0_FLIP = 34;            //! ToF vs Pin1 dE flipped
      const int DD_TOF0_PIN0_GATED_FIT_FLIP = 35;  //! GATED ToF vs Pin1 dE flipped
      const int DD_TOF0_PIN0_GATED_YSO_FLIP = 36;  //! GATED ToF vs Pin1 dE flipped
      const int DD_TOF0_PIN0_GATED_RIT_FLIP = 37;  //! GATED ToF vs Pin1 dE flipped

      const int DD_TOF1_PIN0 = 40;            //! ToF vs Pin0 dE
      const int DD_TOF1_PIN0_GATED_FIT = 41;  //! GATED ToF vs Pin0 dE
      const int DD_TOF1_PIN0_GATED_YSO = 42;  //! GATED ToF vs Pin0 dE
      const int DD_TOF1_PIN0_GATED_RIT = 43;  //! GATED ToF vs Pin0 dE
      const int DD_TOF1_PIN0_FLIP = 44;            //! ToF vs Pin1 dE flipped
      const int DD_TOF1_PIN0_GATED_FIT_FLIP = 45;  //! GATED ToF vs Pin1 dE flipped
      const int DD_TOF1_PIN0_GATED_YSO_FLIP = 46;  //! GATED ToF vs Pin1 dE flipped
      const int DD_TOF1_PIN0_GATED_RIT_FLIP = 47;  //! GATED ToF vs Pin1 dE flipped

      const int DD_TOF2_PIN0 = 50;            //! ToF vs Pin0 dE
      const int DD_TOF2_PIN0_GATED_FIT = 51;  //! GATED ToF vs Pin0 dE
      const int DD_TOF2_PIN0_GATED_YSO = 52;  //! GATED ToF vs Pin0 dE
      const int DD_TOF2_PIN0_GATED_RIT = 53;  //! GATED ToF vs Pin0 dE
      const int DD_TOF2_PIN0_FLIP = 54;            //! ToF vs Pin1 dE flipped
      const int DD_TOF2_PIN0_GATED_FIT_FLIP = 55;  //! GATED ToF vs Pin1 dE flipped
      const int DD_TOF2_PIN0_GATED_YSO_FLIP = 56;  //! GATED ToF vs Pin1 dE flipped
      const int DD_TOF2_PIN0_GATED_RIT_FLIP = 57;  //! GATED ToF vs Pin1 dE flipped

      const int DD_TOF3_PIN0 = 60;            //! ToF vs Pin0 dE
      const int DD_TOF3_PIN0_GATED_FIT = 61;  //! GATED ToF vs Pin0 dE
      const int DD_TOF3_PIN0_GATED_YSO = 62;  //! GATED ToF vs Pin0 dE
      const int DD_TOF3_PIN0_GATED_RIT = 63;  //! GATED ToF vs Pin0 dE
      const int DD_TOF3_PIN0_FLIP = 64;            //! ToF vs Pin1 dE flipped
      const int DD_TOF3_PIN0_GATED_FIT_FLIP = 65;  //! GATED ToF vs Pin1 dE flipped
      const int DD_TOF3_PIN0_GATED_YSO_FLIP = 66;  //! GATED ToF vs Pin1 dE flipped
      const int DD_TOF3_PIN0_GATED_RIT_FLIP = 67;  //! GATED ToF vs Pin1 dE flipped

      const int D_RANGE = 70; //implantation range gated by PID (right now hard coded)


   }  // namespace pid
}  // namespace dammIds

void PidProcessor::DeclarePlots(void) {
   DeclareHistogram1D(D_IMAGEL_MULT, S5, "Multiplicity of image L");
   DeclareHistogram1D(D_DISPL_MULT, S5, "Multiplicity of dispersive PPAC L");
   DeclareHistogram2D(DD_TACS_MULT, S2, S5, "TAC multiplicity ");
   DeclareHistogram2D(DD_TACS, S2, SD, "TAC energy ");
   DeclareHistogram2D(DD_PINS_DE, S2, SD, "Pin dE");
   DeclareHistogram2D(DD_PINS_MULT, S2, S5, "Pins Multiplicity");
   DeclareHistogram2D(DD_PIN0_1, SC, SC, "Pin0 vs Pin1 dE ");
   DeclareHistogram1D(D_DISPLR, SD, "Dispersive L - R");
   DeclareHistogram1D(D_DISPUD, SD, "Dispersive U - D");
   DeclareHistogram2D(DD_DISP_PLANE, SB, SB, "Disp U_D vs Disp L_R");

   DeclareHistogram2D(DD_TAC0_PIN0, SD, SD, "TAC0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TAC0_PIN0_GATED_FIT, SD, SD, "FIT: TAC0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TAC0_PIN0_GATED_YSO, SD, SD, "YSO: TAC0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TAC0_PIN0_GATED_RIT, SD, SD, "RIT: TAC0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF2_PIN1, SB, SD, "TOF2 vs Pin1 dE ");
   DeclareHistogram2D(DD_TOF2_PIN1_GATED_FIT, SB, SD, "FIT: TOF2 vs Pin1 dE ");
   DeclareHistogram2D(DD_TOF2_PIN1_GATED_YSO, SB, SD, "YSO: TOF2 vs Pin1 dE ");
   DeclareHistogram2D(DD_TOF2_PIN1_GATED_RIT, SB, SD, "RIT: TOF2 vs Pin1 dE ");

   DeclareHistogram2D(DD_TAC1_PIN0, SD, SD, "TAC1 vs Pin0 dE ");
   DeclareHistogram2D(DD_TAC1_PIN0_GATED_FIT, SD, SD, "FIT: TAC1 vs Pin0 dE ");
   DeclareHistogram2D(DD_TAC1_PIN0_GATED_YSO, SD, SD, "YSO: TAC1 vs Pin0 dE ");
   DeclareHistogram2D(DD_TAC1_PIN0_GATED_RIT, SD, SD, "RIT: TAC1 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF3_PIN1, SB, SD, "TOF3 vs Pin1 dE ");
   DeclareHistogram2D(DD_TOF3_PIN1_GATED_FIT, SB, SD, "FIT: TOF3 vs Pin1 dE ");
   DeclareHistogram2D(DD_TOF3_PIN1_GATED_YSO, SB, SD, "YSO: TOF3 vs Pin1 dE ");
   DeclareHistogram2D(DD_TOF3_PIN1_GATED_RIT, SB, SD, "RIT: TOF3 vs Pin1 dE ");

   DeclareHistogram2D(DD_TOF0_PIN0, SB, SD, "Tof0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF0_PIN0_GATED_FIT, SB, SD, "FIT: Tof0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF0_PIN0_GATED_YSO, SB, SD, "YSO: Tof0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF0_PIN0_GATED_RIT, SB, SD, "RIT: Tof0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF0_PIN0_FLIP, SB, SD, "Flipped Tof0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF0_PIN0_GATED_FIT_FLIP, SB, SD, "YSO: Flipped Tof0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF0_PIN0_GATED_YSO_FLIP, SB, SD, "YSO: Flipped Tof0 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF0_PIN0_GATED_RIT_FLIP, SB, SD, "YSO: Flipped Tof0 vs Pin0 dE ");

   DeclareHistogram2D(DD_TOF1_PIN0, SB, SD, "Tof1 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF1_PIN0_GATED_FIT, SB, SD, "FIT: Tof1 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF1_PIN0_GATED_YSO, SB, SD, "YSO: Tof1 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF1_PIN0_GATED_RIT, SB, SD, "RIT: Tof1 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF1_PIN0_FLIP, SB, SD, "Flipped Tof1 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF1_PIN0_GATED_FIT_FLIP, SB, SD, "FIT: Flipped Tof1 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF1_PIN0_GATED_YSO_FLIP, SB, SD, "YSO: Flipped Tof1 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF1_PIN0_GATED_RIT_FLIP, SB, SD, "RIT: Flipped Tof1 vs Pin0 dE ");

   DeclareHistogram2D(DD_TOF2_PIN0, SB, SD, "Tof2 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF2_PIN0_GATED_FIT, SB, SD, "FIT: Tof2 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF2_PIN0_GATED_YSO, SB, SD, "YSO: Tof2 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF2_PIN0_GATED_RIT, SB, SD, "RIT: Tof2 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF2_PIN0_FLIP, SB, SD, "Flipped Tof2 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF2_PIN0_GATED_FIT_FLIP, SB, SD, "FIT: Flipped Tof2 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF2_PIN0_GATED_YSO_FLIP, SB, SD, "YSO: Flipped Tof2 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF2_PIN0_GATED_RIT_FLIP, SB, SD, "RIT: Flipped Tof2 vs Pin0 dE ");

   DeclareHistogram2D(DD_TOF3_PIN0, SB, SD, "Tof3 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF3_PIN0_GATED_FIT, SB, SD, "FIT: Tof3 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF3_PIN0_GATED_YSO, SB, SD, "YSO: Tof3 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF3_PIN0_GATED_RIT, SB, SD, "RIT: Tof3 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF3_PIN0_FLIP, SB, SD, "Flipped Tof3 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF3_PIN0_GATED_FIT_FLIP, SB, SD, "FIT: Flipped Tof3 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF3_PIN0_GATED_YSO_FLIP, SB, SD, "YSO: Flipped Tof3 vs Pin0 dE ");
   DeclareHistogram2D(DD_TOF3_PIN0_GATED_RIT_FLIP, SB, SD, "RIT: Flipped Tof3 vs Pin0 dE ");

   DeclareHistogram1D(D_RANGE, S3, "Range distribution with PID gate");

}  // Declare plots

PidProcessor::PidProcessor(const double &YSO_Implant_thresh, const double &FIT_thresh, const double &RIT_thresh) : EventProcessor(OFFSET, RANGE, "PidProcessor") {
   associatedTypes.insert("pid");
   associatedTypes.insert("pin");

   yso_threshold_ = YSO_Implant_thresh;
   fit_threshold_ = FIT_thresh;
   rit_threshold_ = RIT_thresh;

   yso_energy_prev = 0;
   fit_energy_prev = 0;
   fit_energy_prev = 0;
}

bool PidProcessor::PreProcess(RawEvent &event) {
   if (!EventProcessor::PreProcess(event))
      return false;

   // A flag for ROOT output
   const bool root_output = DetectorDriver::get()->GetSysRootOutput();

   //! Generate YSO implant Bool with standard tree correlator places
   //std::cout<<"yso_thresh = "<<yso_threshold_<<", fit_thresh = "<<fit_threshold_<<", rit_thresh = "<<rit_threshold_<<std::endl;
   if (root_output) {
      // Initialization of the pid_struct
      pid_struct = processor_struct::PID_DEFAULT_STRUCT;
   }

   static const vector<ChanEvent *> &cross_scint_b1_vec = event.GetSummary("pid:cross_scint_b1", true)->GetList();
   static const vector<ChanEvent *> &cross_scint_t1_vec = event.GetSummary("pid:cross_scint_t1", true)->GetList();
   static const vector<ChanEvent *> &cross_pin0_vec = event.GetSummary("pid:cross_pin0", true)->GetList();
   static const vector<ChanEvent *> &cross_pin1_vec = event.GetSummary("pid:cross_pin1", true)->GetList();
   static const vector<ChanEvent *> &tac0_vec = event.GetSummary("pid:tac0", true)->GetList();
   static const vector<ChanEvent *> &tac1_vec = event.GetSummary("pid:tac1", true)->GetList();
   static const vector<ChanEvent *> &tac2_vec = event.GetSummary("pid:tac2", true)->GetList();
   static const vector<ChanEvent *> &tac3_vec = event.GetSummary("pid:tac3", true)->GetList();
   static const vector<ChanEvent *> &dispL_vec = event.GetSummary("pid:disp_L_logic", true)->GetList();
   static const vector<ChanEvent *> &dispR_vec = event.GetSummary("pid:disp_R_logic", true)->GetList();
   static const vector<ChanEvent *> &dispU_vec = event.GetSummary("pid:disp_U_logic", true)->GetList();
   static const vector<ChanEvent *> &dispD_vec = event.GetSummary("pid:disp_D_logic", true)->GetList();
   static const vector<ChanEvent *> &imageL_vec = event.GetSummary("pid:image_L_logic", true)->GetList();
   static const vector<ChanEvent *> &pinLogic_vec = event.GetSummary("pid:cross_pin0_logic", true)->GetList();
   static const vector<ChanEvent *> &b2Logic_vec = event.GetSummary("pid:cross_scint_b2_logic", true)->GetList();

   // Function that compares energies in two ChanEvent objects
   auto compare_energy = [](ChanEvent *x1, ChanEvent *x2) { return x1->GetCalibratedEnergy() < x2->GetCalibratedEnergy(); };

   // Function that returns time in ns
   // Note: this function returns time WITHOUT Pixie onboard CFD
   auto get_time_in_ns = [](ChanEvent *x) {
      return x->GetTimeSansCfd() * Globals::get()->GetClockInSeconds(x->GetChanID().GetModFreq()) * 1e9;
   };

   //* Tof between rfq and beamline FocalPlane */

   double tof0 = 0, tof1 = 0, tof2 = 0, tof3 = 0, pin0_energy = 0, pin1_energy = 0, tac0_energy = 0, tac1_energy = 0, tac2_energy=0, tac3_energy=0;
   double cross_scint_b1_energy = 0, cross_scint_t1_energy = 0;
   double tof0_flip = 0, tof1_flip = 0, tof2_flip = 0, tof3_flip = 0;
   double disp_LR = 0;
   double disp_UD = 0;
   
   if(!imageL_vec.empty() && root_output){
      auto imageL = std::max_element(imageL_vec.begin(), imageL_vec.end(), compare_energy);
      double internalTAC_Convert_Tick_adc = Globals::get()->GetAdcClockInSeconds((*imageL)->GetChanID().GetModFreq()) * 1e9;
      if((*imageL))
         pid_struct.image_scint_L_logic_time = (*imageL)->GetTime() * internalTAC_Convert_Tick_adc;
   }
   if(!pinLogic_vec.empty() && root_output){
      auto pinLogic = std::max_element(pinLogic_vec.begin(), pinLogic_vec.end(), compare_energy);
      double internalTAC_Convert_Tick_adc = Globals::get()->GetAdcClockInSeconds((*pinLogic)->GetChanID().GetModFreq()) * 1e9;
      if((*pinLogic))
        pid_struct.cross_pin_0_logic_time = (*pinLogic)->GetTime() * internalTAC_Convert_Tick_adc;
   }
   if(!b2Logic_vec.empty() && root_output){
      auto b2Logic = std::max_element(b2Logic_vec.begin(), b2Logic_vec.end(), compare_energy);
      double internalTAC_Convert_Tick_adc = Globals::get()->GetAdcClockInSeconds((*b2Logic)->GetChanID().GetModFreq()) * 1e9;
      if((*b2Logic))
        pid_struct.cross_scint_b2_logic_time = (*b2Logic)->GetTime() * internalTAC_Convert_Tick_adc;
   }
   //std::cout<<"come to line 251"<<std::endl;
   if (!cross_pin0_vec.empty() && root_output) {
      // Get elements with the largest energy in this event 
      auto pin = std::max_element(cross_pin0_vec.begin(), cross_pin0_vec.end(), compare_energy);
      // Check for nullptr
      if (!(*pin)->GetTrace().empty()) {
         pid_struct.cross_pin_0_tracemax = (*pin)->GetTrace().GetMaxInfo().second;
         pid_struct.cross_pin_0_traceqdc = (*pin)->GetTrace().GetQdc();
      }
   }
   //std::cout<<"come to line 260"<<std::endl;
   if (!cross_pin1_vec.empty() && root_output) {
      // Get elements with the largest energy in this event 
      auto pin = std::max_element(cross_pin1_vec.begin(), cross_pin1_vec.end(), compare_energy);
      // Check for nullptr
      if (!(*pin)->GetTrace().empty()) {
         pid_struct.cross_pin_1_tracemax = (*pin)->GetTrace().GetMaxInfo().second;
         pid_struct.cross_pin_1_traceqdc = (*pin)->GetTrace().GetQdc();
      }
   }
   //std::cout<<"come to line 269"<<std::endl;


   //! TOF 0 group (image L to cross pin0 onboard cfd)
   if (!imageL_vec.empty() && !pinLogic_vec.empty()) {
      // Get elements with the largest energy in this event for image L
      auto imageL = std::max_element(imageL_vec.begin(), imageL_vec.end(), compare_energy);
      double internalTAC_Convert_Tick_adc_imageL = Globals::get()->GetAdcClockInSeconds((*imageL)->GetChanID().GetModFreq()) * 1e9;

      // Get elements with the largest energy in this event for pinLogic
      auto pinLogic = std::max_element(pinLogic_vec.begin(), pinLogic_vec.end(), compare_energy);
      double internalTAC_Convert_Tick_adc_pinLogic = Globals::get()->GetAdcClockInSeconds((*pinLogic)->GetChanID().GetModFreq()) * 1e9;
      // Check for nullptr
      if ((*imageL) && (*pinLogic)) {
         // Calculate tof
         tof0 = ((*pinLogic)->GetTime() * internalTAC_Convert_Tick_adc_pinLogic) - ((*imageL)->GetTime() * internalTAC_Convert_Tick_adc_imageL);
         tof0_flip = ((*imageL)->GetTime() * internalTAC_Convert_Tick_adc_imageL) - ((*pinLogic)->GetTime() * internalTAC_Convert_Tick_adc_pinLogic);

         // ROOT outputs
         if (root_output) {
            pid_struct.tof0 = tof0;
         }
      }
   }
   //std::cout<<"come to line 322"<<std::endl;
   //
   //! TOF 1 group (image L to cross pin0 highrestime)
   if (!imageL_vec.empty() && !cross_pin0_vec.empty()) {
      // Get elements with the largest energy in this event for image L
      auto imageL = std::max_element(imageL_vec.begin(), imageL_vec.end(), compare_energy);
      double internalTAC_Convert_Tick_adc_imageL = Globals::get()->GetAdcClockInSeconds((*imageL)->GetChanID().GetModFreq()) * 1e9;

      // Get elements with the largest energy in this event for pinLogic
      auto pin = std::max_element(cross_pin0_vec.begin(), cross_pin0_vec.end(), compare_energy);
      // Check for nullptr
      if ((*imageL) && (*pin)) {
         // Calculate tof
         tof1 = (*pin)->GetHighResTimeInNs() - ((*imageL)->GetTime() * internalTAC_Convert_Tick_adc_imageL);
         tof1_flip = ((*imageL)->GetTime() * internalTAC_Convert_Tick_adc_imageL) - (*pin)->GetHighResTimeInNs();

         // ROOT outputs
         if (root_output) {
            pid_struct.tof1 = tof1;
         }
      }
   }
   //std::cout<<"come to line 344"<<std::endl;
   //
   //! TOF 2 group (image L to cross scint onboard cfd)
   if (!imageL_vec.empty() && !b2Logic_vec.empty()) {
      // Get elements with the largest energy in this event for image L
      auto imageL = std::max_element(imageL_vec.begin(), imageL_vec.end(), compare_energy);
      double internalTAC_Convert_Tick_adc_imageL = Globals::get()->GetAdcClockInSeconds((*imageL)->GetChanID().GetModFreq()) * 1e9;

      // Get elements with the largest energy in this event for pinLogic
      auto b2Logic = std::max_element(b2Logic_vec.begin(), b2Logic_vec.end(), compare_energy);
      double internalTAC_Convert_Tick_adc_b2Logic = Globals::get()->GetAdcClockInSeconds((*b2Logic)->GetChanID().GetModFreq()) * 1e9;
      // Check for nullptr
      if ((*imageL) && (*b2Logic)) {
         // Calculate tof
         tof2 = ((*b2Logic)->GetTime() * internalTAC_Convert_Tick_adc_b2Logic) - ((*imageL)->GetTime() * internalTAC_Convert_Tick_adc_imageL);
         tof2_flip = ((*imageL)->GetTime() * internalTAC_Convert_Tick_adc_imageL) - ((*b2Logic)->GetTime() * internalTAC_Convert_Tick_adc_b2Logic);

         // ROOT outputs
         if (root_output) {
            pid_struct.tof2 = tof2;
         }
      }
   }
   //std::cout<<"come to line 367"<<std::endl;
   //
   //! TOF 3 group (image L to cross scint highrestime)
   if (!imageL_vec.empty() && !cross_scint_b1_vec.empty()) {
      // Get elements with the largest energy in this event for image L
      auto imageL = std::max_element(imageL_vec.begin(), imageL_vec.end(), compare_energy);
      double internalTAC_Convert_Tick_adc_imageL = Globals::get()->GetAdcClockInSeconds((*imageL)->GetChanID().GetModFreq()) * 1e9;

      // Get elements with the largest energy in this event for pinLogic
      auto cross_scint_b1 = std::max_element(cross_scint_b1_vec.begin(), cross_scint_b1_vec.end(), compare_energy);
      // Check for nullptr
      if ((*imageL) && (*cross_scint_b1)) {
         // Calculate tof
         tof3 = (*cross_scint_b1)->GetHighResTimeInNs() - ((*imageL)->GetTime() * internalTAC_Convert_Tick_adc_imageL);
         tof3_flip = ((*imageL)->GetTime() * internalTAC_Convert_Tick_adc_imageL) - (*cross_scint_b1)->GetHighResTimeInNs();

         // ROOT outputs
         if (root_output) {
            pid_struct.tof3 = tof3;
         }
      }
   }
   //std::cout<<"come to line 389"<<std::endl;
   //
   //
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

   //! dispersive U-D group (diespersive U -> D)
   if (!dispU_vec.empty() && !dispD_vec.empty()) {
      // Get elements with the largest energy in this event for dispU
      auto dispU = std::max_element(dispU_vec.begin(), dispU_vec.end(), compare_energy);
      double internalTAC_Convert_Tick_adc = Globals::get()->GetAdcClockInSeconds((*dispU)->GetChanID().GetModFreq()) * 1e9;

      // Get elements with the largest energy in this event for dispD
      auto dispD = std::max_element(dispD_vec.begin(), dispD_vec.end(), compare_energy);
      // Check for nullptr
      if ((*dispU) && (*dispD)) {
         // Calculate UD
         disp_UD = ((*dispU)->GetTime() * internalTAC_Convert_Tick_adc) - ((*dispD)->GetTime() * internalTAC_Convert_Tick_adc);

         // ROOT outputs
         if (root_output) {
            pid_struct.disp_U_logic_time = (*dispU)->GetTime() * internalTAC_Convert_Tick_adc;
            pid_struct.disp_D_logic_time = (*dispD)->GetTime() * internalTAC_Convert_Tick_adc;
            pid_struct.disp_UD = disp_UD;
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
   
   if (!tac3_vec.empty()) {
      tac3_energy = 0;
      if (tac3_vec.size() >= 2) {
         auto tac_max = max_element(tac3_vec.begin(), tac3_vec.end(), compare_energy);
         tac3_energy = (*tac_max)->GetCalibratedEnergy();
      } else {
         tac3_energy = tac3_vec.at(0)->GetCalibratedEnergy();
      }
      if (root_output) {
         pid_struct.tac_3 = tac3_energy;
      }
      for (auto tac = tac3_vec.begin(); tac != tac3_vec.end(); ++tac) {
         plot(DD_TACS, 3, (*tac)->GetCalibratedEnergy());
      }
   }

   //** Pins */

   if (!cross_pin0_vec.empty()) {
      auto pin = max_element(cross_pin0_vec.begin(), cross_pin0_vec.end(), compare_energy);
      pin0_energy = (*pin)->GetCalibratedEnergy();
      plot(DD_PINS_DE, 0, pin0_energy);
      if (root_output) {
         pid_struct.cross_pin_0_energy = pin0_energy;
         pid_struct.cross_pin_0_time = (*pin)->GetHighResTimeInNs();
      }
   }
   if (!cross_pin1_vec.empty()) {
      auto pin = max_element(cross_pin1_vec.begin(), cross_pin1_vec.end(), compare_energy);
      pin1_energy = (*pin)->GetCalibratedEnergy();
      plot(DD_PINS_DE, 1, pin1_energy);
      if (root_output) {
         pid_struct.cross_pin_1_energy = pin1_energy;
         pid_struct.cross_pin_1_time = (*pin)->GetHighResTimeInNs();
      }
   }

   //** Cross plastic *//
   if (!cross_scint_b1_vec.empty()) {
      auto cross_scint = max_element(cross_scint_b1_vec.begin(), cross_scint_b1_vec.end(), compare_energy);
      cross_scint_b1_energy = (*cross_scint)->GetCalibratedEnergy();
      if (root_output) {
         pid_struct.cross_scint_b1_energy = cross_scint_b1_energy;
         pid_struct.cross_scint_b1_time = (*cross_scint)->GetHighResTimeInNs();
      }
   }
   if (!cross_scint_t1_vec.empty()) {
      auto cross_scint = max_element(cross_scint_t1_vec.begin(), cross_scint_t1_vec.end(), compare_energy);
      cross_scint_t1_energy = (*cross_scint)->GetCalibratedEnergy();
      if (root_output) {
         pid_struct.cross_scint_t1_energy = cross_scint_t1_energy;
         pid_struct.cross_scint_t1_time = (*cross_scint)->GetHighResTimeInNs();
      }
   }
   //
   //make the variables proper for DAMM plot (not ROOT file)
   if(tof0!=0){
      tof0 = tof0*10+500;
      tof0_flip = tof0_flip*10+1000;
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
      tof3 = tof3*50+49500;
      tof3_flip = tof3_flip*50-47000;
   }else{
      tof3 = -999;
      tof3_flip = -999;
   }
   if(disp_LR!=0){
      disp_LR = disp_LR*50+5000;
   }else{
      disp_LR = -999;
   }
   if(disp_UD!=0){
      disp_UD = disp_UD*100+500;
   }else{
      disp_UD = -999;
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
            // cout<<"FIT_Implant"<<endl;
         }
      }
   }

   pid_struct.stop_in = stop_in;
   pid_struct.fit_energy = fit_energy;
   pid_struct.yso_energy = yso_energy;
   pid_struct.rit_energy = rit_energy;
   //std::cout<<"stop in: "<<pid_struct.stop_in<<std::endl;

   // Fill the plots
   plot(D_IMAGEL_MULT, imageL_vec.size());
   plot(D_DISPL_MULT, dispL_vec.size());
   plot(DD_TACS_MULT, 0, tac0_vec.size());
   plot(DD_TACS_MULT, 1, tac1_vec.size());
   plot(DD_TACS_MULT, 2, tac2_vec.size());
   plot(DD_TACS_MULT, 3, tac3_vec.size());
   plot(DD_PINS_MULT, 0, cross_pin0_vec.size());
   plot(DD_PINS_MULT, 1, cross_pin1_vec.size());
   plot(DD_PIN0_1, pin0_energy, pin1_energy);
   plot(D_DISPLR, disp_LR);
   plot(D_DISPUD, disp_UD);
   plot(DD_DISP_PLANE,disp_UD/8.,disp_LR/8.);

   plot(DD_TAC0_PIN0, tac0_energy, pin0_energy);
   plot(DD_TOF2_PIN1, tof2, pin1_energy);
   plot(DD_TAC1_PIN0, tac1_energy, pin0_energy);
   plot(DD_TOF3_PIN1, tof3, pin1_energy);
   plot(DD_TOF0_PIN0, tof0, pin0_energy);
   plot(DD_TOF0_PIN0_FLIP, tof0_flip, pin0_energy);
   plot(DD_TOF1_PIN0, tof1, pin0_energy);
   plot(DD_TOF1_PIN0_FLIP, tof1_flip, pin0_energy);
   plot(DD_TOF2_PIN0, tof2, pin0_energy);
   plot(DD_TOF2_PIN0_FLIP, tof2_flip, pin0_energy);
   plot(DD_TOF3_PIN0, tof3, pin0_energy);
   plot(DD_TOF3_PIN0_FLIP, tof3_flip, pin0_energy);

   if (FIT_Implant) {
      plot(DD_TAC0_PIN0_GATED_FIT, tac0_energy, pin0_energy);
      plot(DD_TOF2_PIN1_GATED_FIT, tof2, pin1_energy);
      plot(DD_TAC1_PIN0_GATED_FIT, tac1_energy, pin0_energy);
      plot(DD_TOF3_PIN1_GATED_FIT, tof3, pin1_energy);
      plot(DD_TOF0_PIN0_GATED_FIT, tof0, pin0_energy);
      plot(DD_TOF0_PIN0_GATED_FIT_FLIP, tof0_flip, pin0_energy);
      plot(DD_TOF1_PIN0_GATED_FIT, tof1, pin0_energy);
      plot(DD_TOF1_PIN0_GATED_FIT_FLIP, tof1_flip, pin0_energy);
      plot(DD_TOF2_PIN0_GATED_FIT, tof2, pin0_energy);
      plot(DD_TOF2_PIN0_GATED_FIT_FLIP, tof2_flip, pin0_energy);
      plot(DD_TOF3_PIN0_GATED_FIT, tof3, pin0_energy);
      plot(DD_TOF3_PIN0_GATED_FIT_FLIP, tof3_flip, pin0_energy);
   }
   if (YSO_Implant) {
      // cout << "In YSO_Implant" << endl;
      plot(DD_TAC0_PIN0_GATED_YSO, tac0_energy, pin0_energy);
      plot(DD_TOF2_PIN1_GATED_YSO, tof2, pin1_energy);
      plot(DD_TAC1_PIN0_GATED_YSO, tac1_energy, pin0_energy);
      plot(DD_TOF3_PIN1_GATED_YSO, tof3, pin1_energy);
      plot(DD_TOF0_PIN0_GATED_YSO, tof0, pin0_energy);
      plot(DD_TOF0_PIN0_GATED_YSO_FLIP, tof0_flip, pin0_energy);
      plot(DD_TOF1_PIN0_GATED_YSO, tof1, pin0_energy);
      plot(DD_TOF1_PIN0_GATED_YSO_FLIP, tof1_flip, pin0_energy);
      plot(DD_TOF2_PIN0_GATED_YSO, tof2, pin0_energy);
      plot(DD_TOF2_PIN0_GATED_YSO_FLIP, tof2_flip, pin0_energy);
      plot(DD_TOF3_PIN0_GATED_YSO, tof3, pin0_energy);
      plot(DD_TOF3_PIN0_GATED_YSO_FLIP, tof3_flip, pin0_energy);
   }
   if (RIT_Implant) {
      plot(DD_TAC0_PIN0_GATED_RIT, tac0_energy, pin0_energy);
      plot(DD_TOF2_PIN1_GATED_RIT, tof2, pin1_energy);
      plot(DD_TAC1_PIN0_GATED_RIT, tac1_energy, pin0_energy);
      plot(DD_TOF3_PIN1_GATED_RIT, tof3, pin1_energy);
      plot(DD_TOF0_PIN0_GATED_RIT, tof0, pin0_energy);
      plot(DD_TOF0_PIN0_GATED_RIT_FLIP, tof0_flip, pin0_energy);
      plot(DD_TOF1_PIN0_GATED_RIT, tof1, pin0_energy);
      plot(DD_TOF1_PIN0_GATED_RIT_FLIP, tof1_flip, pin0_energy);
      plot(DD_TOF2_PIN0_GATED_RIT, tof2, pin0_energy);
      plot(DD_TOF2_PIN0_GATED_RIT_FLIP, tof2_flip, pin0_energy);
      plot(DD_TOF3_PIN0_GATED_RIT, tof3, pin0_energy);
      plot(DD_TOF3_PIN0_GATED_RIT_FLIP, tof3_flip, pin0_energy);
   }

   if(tof3>1300 && tof3<1600 && pin0_energy>500 && pin0_energy<700){
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
