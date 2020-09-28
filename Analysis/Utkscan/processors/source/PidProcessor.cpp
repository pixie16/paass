///@file PidProcessor.cpp
///@A dedicated processor for constructing PID information
///@author A. Keeler
///@date July 29, 2019

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
    const int D_TOF_SIZE = 0;
    const int D_PIN_SIZE = 1;

    const int D_PIN = 2;
    const int DD_PIN1_2 = 3;
    const int DD_PIN1_2_CUT = 11;

    const int D_TOF_I2N = 4;

    const int DD_PID = 5;

    const int D_POS_I2NS = 6;
    const int D_POS_I2S_N = 7;

    const int DD_POS_TOF_I2N = 8;

    const int DD_POS_CORRECTED_TOF = 9;

    const int DD_CORRECTED_PID = 10;


  }  // namespace pid
}  // namespace dammIds

void PidProcessor::DeclarePlots(void) {
  DeclareHistogram1D(D_TOF_SIZE, S4, "separator ToF multiplicities: 0 - i2n, 5 - i2s, 10 - i2ns");
  DeclareHistogram1D(D_PIN_SIZE, S2, "pin multiplicity");
  DeclareHistogram1D(D_PIN, SA, "pin energy");
  DeclareHistogram2D(DD_PIN1_2, SA, SA, "pin energies");
  DeclareHistogram2D(DD_PIN1_2_CUT, SA, SA, "pin energies - cut");
  DeclareHistogram1D(D_TOF_I2N, SB, "separator ToF");
  DeclareHistogram2D(DD_PID, SB, SA, "uncorrected PID plot");
  DeclareHistogram1D(D_POS_I2NS, SB,"position in I2 taking NS time difference");
  DeclareHistogram1D(D_POS_I2S_N, SB,"position in I2 taking NS tof difference to pin");
  DeclareHistogram2D(DD_POS_TOF_I2N, SB, SB, "Position in I2 vs I2N ToF");
  DeclareHistogram2D(DD_POS_CORRECTED_TOF, SB, SB, "Position in I2 vs corrected I2N ToF");
  DeclareHistogram2D(DD_CORRECTED_PID, SB, SA, "corrected PID plot");

}  // Declare plots

PidProcessor::PidProcessor(double slope, double intercept, double pin_threshold): EventProcessor(OFFSET, RANGE, "PidProcessor") {

  slope_ = slope;
  intercept_ = intercept;
  threshold_ = pin_threshold;
  associatedTypes.insert("pid");
  associatedTypes.insert("pin");
}

bool PidProcessor::PreProcess(RawEvent &event) {
  if (!EventProcessor::PreProcess(event))
    return false;

  if (DetectorDriver::get()->GetSysRootOutput()) {
    pid_struct = processor_struct::PID_DEFAULT_STRUCT;
  }

  // position corrections to the PID plot require either a position measurement in
  //the dispersive plane of the separator or two different ToF measurements to infer the position

  static const vector<ChanEvent *> &pin_1 = event.GetSummary("pid:pin1",true)->GetList();
  static const vector<ChanEvent *> &pin_2 = event.GetSummary("pid:pin2",true)->GetList();
  static const vector<ChanEvent *> &tof_1 = event.GetSummary("pid:pin1_i2n")->GetList();
  static const vector<ChanEvent *> &tof_2 = event.GetSummary("pid:pin1_i2s")->GetList();
  static const vector<ChanEvent *> &pos = event.GetSummary("pid:i2n_i2s")->GetList();


  //-----------Calculate A/Q from separator ToF

  double delta = -999;

  for (auto iterator = 0; iterator < pin_1.size(); iterator++){
    //  vector<ChanEvent *>::const_iterator iterator = pin.begin();
    //  while (iterator != pin.end() && pin_energy == 0){
    if (pin_1.at(iterator)->GetCalibratedEnergy() > threshold_)
      pin_energy = pin_1.at(iterator)->GetCalibratedEnergy();
  }
  if (pin_2.size() > 0)
    delta = 0.87558748 * (*pin_2.begin())->GetCalibratedEnergy() + 8.72618557 - pin_energy;

  if (pin_energy > 0 && delta < 40 && delta > -40 && tof_1.size() == 1 && tof_2.size() == 1) {
    
    tof = (*tof_1.begin())->GetCalibratedEnergy();
    
    position = ((*tof_2.begin())->GetCalibratedEnergy() - tof);
    plot(D_POS_I2S_N, position);
    
    corrected_tof = CorrectTofByPosition(tof, position, slope_, intercept_);  

  }

  //---------------plot Damm histograms

  plot(D_TOF_SIZE, tof_1.size());
  plot(D_TOF_SIZE, tof_2.size() + 5);
  plot(D_TOF_SIZE, pos.size() + 10);


  plot(D_PIN_SIZE, pin_1.size());
  plot(D_PIN, pin_energy);

  plot(D_TOF_I2N, tof);

  plot(DD_PID, tof, pin_energy);
  plot(DD_POS_TOF_I2N, tof, position);
  plot(DD_POS_CORRECTED_TOF, corrected_tof, position);
  plot(DD_CORRECTED_PID, corrected_tof, pin_energy);


  if(DetectorDriver::get()->GetSysRootOutput()){
    pid_struct.nuclear_z = ConvertPinToZ(pin_energy);
    pid_struct.mass_charge = ConvertTofToAQ(corrected_tof);

    pid_struct.uncorrected_tof = tof;
    pid_struct.position = position;

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

