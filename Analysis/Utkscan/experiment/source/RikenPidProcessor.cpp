///@file RikenPidProcessor.cpp
///@A dedicated processor for constructing PID information
///@author A. Keeler, R. Yokoyama
///@date July 29, 2019
// Updated for E19044 experiment.

#include "RikenPidProcessor.hpp"

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

      const int DD_PIN2_3 = 71;     //! Pin2 vs Pin3 dE
      const int DD_PIN0_2 = 72;     //! Pin0 vs Pin2 dE
      const int DD_PIN1_3 = 73;     //! Pin1 vs Pin3 dE
      const int DD_TOF4_PIN2 = 74;            //! ToF4 vs Pin2 dE
      const int DD_TOF5_PIN2 = 75;            //! ToF5 vs Pin2 dE
      const int DD_TOF4_PIN3 = 76;            //! ToF4 vs Pin3 dE
      const int DD_TOF5_PIN3 = 77;            //! ToF5 vs Pin3 dE

   }  // namespace pid
}  // namespace dammIds

void RikenPidProcessor::DeclarePlots(void) {
   /* DeclareHistogram1D(D_IMAGEL_MULT, S5, "Multiplicity of image L"); */
   /* DeclareHistogram1D(D_DISPL_MULT, S5, "Multiplicity of dispersive PPAC L"); */
   /* DeclareHistogram2D(DD_TACS_MULT, S2, S5, "TAC multiplicity "); */
   /* DeclareHistogram2D(DD_TACS, S2, SD, "TAC energy "); */
   /* DeclareHistogram2D(DD_PINS_DE, S2, SD, "Pin dE"); */
   /* DeclareHistogram2D(DD_PINS_MULT, S2, S5, "Pins Multiplicity"); */
   /* DeclareHistogram2D(DD_PIN0_1, SC, SC, "Pin0 vs Pin1 dE "); */
   /* DeclareHistogram1D(D_DISPLR, SD, "Dispersive L - R"); */
   /* DeclareHistogram1D(D_DISPUD, SD, "Dispersive U - D"); */
   /* DeclareHistogram2D(DD_DISP_PLANE, SB, SB, "Disp U_D vs Disp L_R"); */

   /* DeclareHistogram2D(DD_TAC0_PIN0, SD, SD, "TAC0 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TAC0_PIN0_GATED_FIT, SD, SD, "FIT: TAC0 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TAC0_PIN0_GATED_YSO, SD, SD, "YSO: TAC0 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TAC0_PIN0_GATED_RIT, SD, SD, "RIT: TAC0 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF2_PIN1, SB, SD, "TOF2 vs Pin1 dE "); */
   /* DeclareHistogram2D(DD_TOF2_PIN1_GATED_FIT, SB, SD, "FIT: TOF2 vs Pin1 dE "); */
   /* DeclareHistogram2D(DD_TOF2_PIN1_GATED_YSO, SB, SD, "YSO: TOF2 vs Pin1 dE "); */
   /* DeclareHistogram2D(DD_TOF2_PIN1_GATED_RIT, SB, SD, "RIT: TOF2 vs Pin1 dE "); */

   /* DeclareHistogram2D(DD_TAC1_PIN0, SD, SD, "TAC1 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TAC1_PIN0_GATED_FIT, SD, SD, "FIT: TAC1 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TAC1_PIN0_GATED_YSO, SD, SD, "YSO: TAC1 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TAC1_PIN0_GATED_RIT, SD, SD, "RIT: TAC1 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF3_PIN1, SB, SD, "TOF3 vs Pin1 dE "); */
   /* DeclareHistogram2D(DD_TOF3_PIN1_GATED_FIT, SB, SD, "FIT: TOF3 vs Pin1 dE "); */
   /* DeclareHistogram2D(DD_TOF3_PIN1_GATED_YSO, SB, SD, "YSO: TOF3 vs Pin1 dE "); */
   /* DeclareHistogram2D(DD_TOF3_PIN1_GATED_RIT, SB, SD, "RIT: TOF3 vs Pin1 dE "); */

   /* DeclareHistogram2D(DD_TOF0_PIN0, SB, SD, "Tof0 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF0_PIN0_GATED_FIT, SB, SD, "FIT: Tof0 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF0_PIN0_GATED_YSO, SB, SD, "YSO: Tof0 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF0_PIN0_GATED_RIT, SB, SD, "RIT: Tof0 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF0_PIN0_FLIP, SB, SD, "Flipped Tof0 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF0_PIN0_GATED_FIT_FLIP, SB, SD, "YSO: Flipped Tof0 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF0_PIN0_GATED_YSO_FLIP, SB, SD, "YSO: Flipped Tof0 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF0_PIN0_GATED_RIT_FLIP, SB, SD, "YSO: Flipped Tof0 vs Pin0 dE "); */

   /* DeclareHistogram2D(DD_TOF1_PIN0, SB, SD, "Tof1 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF1_PIN0_GATED_FIT, SB, SD, "FIT: Tof1 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF1_PIN0_GATED_YSO, SB, SD, "YSO: Tof1 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF1_PIN0_GATED_RIT, SB, SD, "RIT: Tof1 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF1_PIN0_FLIP, SB, SD, "Flipped Tof1 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF1_PIN0_GATED_FIT_FLIP, SB, SD, "FIT: Flipped Tof1 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF1_PIN0_GATED_YSO_FLIP, SB, SD, "YSO: Flipped Tof1 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF1_PIN0_GATED_RIT_FLIP, SB, SD, "RIT: Flipped Tof1 vs Pin0 dE "); */

   /* DeclareHistogram2D(DD_TOF2_PIN0, SB, SD, "Tof2 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF2_PIN0_GATED_FIT, SB, SD, "FIT: Tof2 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF2_PIN0_GATED_YSO, SB, SD, "YSO: Tof2 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF2_PIN0_GATED_RIT, SB, SD, "RIT: Tof2 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF2_PIN0_FLIP, SB, SD, "Flipped Tof2 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF2_PIN0_GATED_FIT_FLIP, SB, SD, "FIT: Flipped Tof2 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF2_PIN0_GATED_YSO_FLIP, SB, SD, "YSO: Flipped Tof2 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF2_PIN0_GATED_RIT_FLIP, SB, SD, "RIT: Flipped Tof2 vs Pin0 dE "); */

   /* DeclareHistogram2D(DD_TOF3_PIN0, SB, SD, "Tof3 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF3_PIN0_GATED_FIT, SB, SD, "FIT: Tof3 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF3_PIN0_GATED_YSO, SB, SD, "YSO: Tof3 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF3_PIN0_GATED_RIT, SB, SD, "RIT: Tof3 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF3_PIN0_FLIP, SB, SD, "Flipped Tof3 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF3_PIN0_GATED_FIT_FLIP, SB, SD, "FIT: Flipped Tof3 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF3_PIN0_GATED_YSO_FLIP, SB, SD, "YSO: Flipped Tof3 vs Pin0 dE "); */
   /* DeclareHistogram2D(DD_TOF3_PIN0_GATED_RIT_FLIP, SB, SD, "RIT: Flipped Tof3 vs Pin0 dE "); */

   /* DeclareHistogram1D(D_RANGE, S3, "Range distribution with PID gate"); */

   /* DeclareHistogram2D(DD_PIN2_3, SC, SC, "Pin2 vs Pin3 dE "); */
   /* DeclareHistogram2D(DD_PIN0_2, SC, SC, "Pin0 vs Pin2 dE "); */
   /* DeclareHistogram2D(DD_PIN1_3, SC, SC, "Pin1 vs Pin3 dE "); */
   /* DeclareHistogram2D(DD_TOF4_PIN2, SB, SD, "Tof4 vs Pin2 dE "); */
   /* DeclareHistogram2D(DD_TOF5_PIN2, SB, SD, "Tof5 vs Pin2 dE "); */
   /* DeclareHistogram2D(DD_TOF4_PIN3, SB, SD, "Tof4 vs Pin3 dE "); */
   /* DeclareHistogram2D(DD_TOF5_PIN3, SB, SD, "Tof5 vs Pin3 dE "); */
}  // Declare plots

RikenPidProcessor::RikenPidProcessor(const double &YSO_Implant_thresh, const double &FIT_thresh, const double &RIT_thresh, const bool &TOFflip) : EventProcessor(OFFSET, RANGE, "RikenPidProcessor") {
   associatedTypes.insert("pid");
   associatedTypes.insert("pin");

   yso_threshold_ = YSO_Implant_thresh;
   fit_threshold_ = FIT_thresh;
   rit_threshold_ = RIT_thresh;

   yso_energy_prev = 0;
   fit_energy_prev = 0;
   fit_energy_prev = 0;

   tofflip_ = TOFflip;

   root_output = DetectorDriver::get()->GetSysRootOutput();
}

bool RikenPidProcessor::PreProcess(RawEvent &event) {
   if (!EventProcessor::PreProcess(event))
      return false;

   // A flag for ROOT output
   if (root_output) {
      // Initialization of the pid_struct
      pid_struct = processor_struct::PID_DEFAULT_STRUCT;
   }

   //Following are added for the RIBF2024 PID
   //up and down stream plastic (L+R) for tof
   const vector<ChanEvent *> &up_scintL_vec = event.GetSummary("pid:up_scit_L_logic", true)->GetList();
   const vector<ChanEvent *> &up_scintR_vec = event.GetSummary("pid:up_scint_R_logic", true)->GetList();
   const vector<ChanEvent *> &down_scintL_vec = event.GetSummary("pid:down_scint_L_logic", true)->GetList();
   const vector<ChanEvent *> &down_scintR_vec = event.GetSummary("pid:down_scint_R_logic", true)->GetList();

   //ionization chamber for dE
   const vector<ChanEvent *> &ic0_vec = event.GetSummary("pid:ic0", true)->GetList();
   const vector<ChanEvent *> &ic1_vec = event.GetSummary("pid:ic1", true)->GetList();
   const vector<ChanEvent *> &ic2_vec = event.GetSummary("pid:ic2", true)->GetList();
   const vector<ChanEvent *> &ic3_vec = event.GetSummary("pid:ic3", true)->GetList();
   const vector<ChanEvent *> &ic4_vec = event.GetSummary("pid:ic4", true)->GetList();
   const vector<ChanEvent *> &ic5_vec = event.GetSummary("pid:ic5", true)->GetList();

   // Function that compares energies in two ChanEvent objects
   auto compare_energy = [](ChanEvent *x1, ChanEvent *x2) { return x1->GetCalibratedEnergy() < x2->GetCalibratedEnergy(); };

   //* Tof between rfq and beamline FocalPlane */

   double tof0 = 0, tof1 = 0, tof2 = 0, tof3 = 0;

   //For RIBF PID
   double up_scintL_time = 0, up_scintR_time = 0, down_scintL_time = 0, down_scintR_time = 0;
   if(!up_scintL_vec.empty()){
      auto up_scintL = max_element(up_scintL_vec.begin(), up_scintL_vec.end(), compare_energy);
      up_scintL_time = (*up_scintL)->GetTimeInNs();
      if(root_output){
         pid_struct.up_scint_L_logic_time = up_scintL_time;
      }
   }
   if(!up_scintR_vec.empty()){
      auto up_scintR = max_element(up_scintR_vec.begin(), up_scintR_vec.end(), compare_energy);
      up_scintR_time = (*up_scintR)->GetTimeInNs();
      if(root_output){
         pid_struct.up_scint_R_logic_time = up_scintR_time;
      }
   }
   if(!down_scintL_vec.empty()){
      auto down_scintL = max_element(down_scintL_vec.begin(), down_scintL_vec.end(), compare_energy);
      down_scintL_time = (*down_scintL)->GetTimeInNs();
      if(root_output){
         pid_struct.down_scint_L_logic_time = down_scintL_time;
      }
   }
   if(!down_scintR_vec.empty()){
      auto down_scintR = max_element(down_scintR_vec.begin(), down_scintR_vec.end(), compare_energy);
      down_scintR_time = (*down_scintR)->GetTimeInNs();
      if(root_output){
         pid_struct.down_scint_R_logic_time = down_scintR_time;
      }
   }
   double ic0_energy = 0, ic1_energy = 0, ic2_energy = 0, ic3_energy = 0, ic4_energy = 0, ic5_energy = 0;
   double ic_avg=0;
   double ic0_time = 0, ic1_time = 0, ic2_time = 0, ic3_time = 0, ic4_time = 0, ic5_time = 0;
   int nonZeroIC = 0;
   if(!ic0_vec.empty()){
      auto ic0 = max_element(ic0_vec.begin(), ic0_vec.end(), compare_energy);
      ic0_energy = (*ic0)->GetCalibratedEnergy();
      ic0_time = (*ic0)->GetTimeSansCfdInNs();
      if(ic0_energy>0){
         ic_avg = ic_avg + log10(ic0_energy);
         nonZeroIC++;
      }
      if(root_output){
         pid_struct.ic0_energy = ic0_energy;
         pid_struct.ic0_time = ic0_time;
      }
   }
   if(!ic1_vec.empty()){
      auto ic1 = max_element(ic1_vec.begin(), ic1_vec.end(), compare_energy);
      ic1_energy = (*ic1)->GetCalibratedEnergy();
      ic1_time = (*ic1)->GetTimeSansCfdInNs();
      if(ic1_energy>0){
         ic_avg = ic_avg + log10(ic1_energy);
         nonZeroIC++;
      }
      if(root_output){
         pid_struct.ic1_energy = ic1_energy;
         pid_struct.ic1_time = ic1_time;
      }
   }
   if(!ic2_vec.empty()){
      auto ic2 = max_element(ic2_vec.begin(), ic2_vec.end(), compare_energy);
      ic2_energy = (*ic2)->GetCalibratedEnergy();
      ic2_time = (*ic2)->GetTimeSansCfdInNs();
      if(ic2_energy>0){
         ic_avg = ic_avg + log10(ic2_energy);
         nonZeroIC++;
      }
      if(root_output){
         pid_struct.ic2_energy = ic2_energy;
         pid_struct.ic2_time = ic2_time;
      }
   }
   if(!ic3_vec.empty()){
      auto ic3 = max_element(ic3_vec.begin(), ic3_vec.end(), compare_energy);
      ic3_energy = (*ic3)->GetCalibratedEnergy();
      ic3_time = (*ic3)->GetTimeSansCfdInNs();
      if(ic3_energy>0){
         ic_avg = ic_avg + log10(ic3_energy);
         nonZeroIC++;
      }
      if(root_output){
         pid_struct.ic3_energy = ic3_energy;
         pid_struct.ic3_time = ic3_time;
      }
   }
   if(!ic4_vec.empty()){
      auto ic4 = max_element(ic4_vec.begin(), ic4_vec.end(), compare_energy);
      ic4_energy = (*ic4)->GetCalibratedEnergy();
      ic4_time = (*ic4)->GetTimeSansCfdInNs();
      if(ic4_energy>0){
         ic_avg = ic_avg + log10(ic4_energy);
         nonZeroIC++;
      }
      if(root_output){
         pid_struct.ic4_energy = ic4_energy;
         pid_struct.ic4_time = ic4_time;
      }
   }
   if(!ic5_vec.empty()){
      auto ic5 = max_element(ic5_vec.begin(), ic5_vec.end(), compare_energy);
      ic5_energy = (*ic5)->GetCalibratedEnergy();
      ic5_time = (*ic5)->GetTimeSansCfdInNs();
      if(ic5_energy>0){
         ic_avg = ic_avg + log10(ic5_energy);
         nonZeroIC++;
      }
      if(root_output){
         pid_struct.ic5_energy = ic5_energy;
         pid_struct.ic5_time = ic5_time;
      }
   }
   if(nonZeroIC>0){
      ic_avg = ic_avg/nonZeroIC;
      ic_avg = pow(10, ic_avg);
   }
   if(root_output){
      pid_struct.ic_avg_energy = ic_avg;
   }
   if(up_scintL_time>0 && up_scintR_time>0 && down_scintL_time>0 && down_scintR_time>0){
      tof0 = 0.5*(down_scintL_time+down_scintR_time)-0.5*(up_scintL_time+up_scintR_time);
      if(root_output){
         pid_struct.tof0 = tof0;
      }
   }
   //
   //make the variables proper for DAMM plot (not ROOT file)
   if(tof0!=0){
      tof0 = tof0*10+500;
   }else{
      tof0 = -999;
   }
   if(tof1!=0){
      tof1 = tof1*10+10500;
   }else{
      tof1 = -999;
   }
   if(tof2!=0){
      tof2 = tof2*50+50000;
   }else{
      tof2 = -999;
   }
   if(tof3!=0){
      tof3 = tof3*50+49500;
   }else{
      tof3 = -999;
   }

   //will add more here
   //////////////////////////////////////

   TString stop_in = "";
   if(ic_avg>0){
      stop_in = "ic";
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
         }
      }
   }

   pid_struct.stop_in = stop_in;
   pid_struct.fit_energy = fit_energy;
   pid_struct.yso_energy = yso_energy;
   pid_struct.rit_energy = rit_energy;

   // Fill the plots
   if (tofflip_){
      plot(DD_TOF0_PIN0, tof0, ic_avg);
      if (FIT_Implant) {
         plot(DD_TOF0_PIN0_GATED_FIT, tof0, ic_avg);
      }
      if (YSO_Implant) {
         plot(DD_TOF0_PIN0_GATED_YSO, tof0, ic_avg);
      }
      if (RIT_Implant) {
         plot(DD_TOF0_PIN0_GATED_RIT, tof0, ic_avg);
      }
   } else {
      plot(DD_TOF0_PIN0, -1 * tof0, ic_avg);
      if (FIT_Implant) {
         plot(DD_TOF0_PIN0_GATED_FIT, -1 * tof0, ic_avg);
      }
      if (YSO_Implant) {
         plot(DD_TOF0_PIN0_GATED_YSO, -1 * tof0, ic_avg);
      }
      if (RIT_Implant) {
         plot(DD_TOF0_PIN0_GATED_RIT, -1 * tof0, ic_avg);
      }
   }

   if(tof3>1300 && tof3<1600 ){
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

bool RikenPidProcessor::Process(RawEvent &event) {
   if (!EventProcessor::Process(event))
      return false;

   EndProcess();
   return true;
}

