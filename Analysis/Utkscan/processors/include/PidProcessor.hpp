///@file PidProcessor.hpp
///@brief A class to handle VANDLE bars 
///Processes PID information and constructs PID plots for fragmentation-
///type experiments
///
///@author A. Keeler
///@date 29 July 2019

#ifndef __PIDPROCESSOR_HPP_
#define __PIDPROCESSOR_HPP_

#include <set>

#include "DammPlotIds.hpp"

#include "EventProcessor.hpp"
#include "RawEvent.hpp"
#include "ProcessorRootStruc.hpp"

/// Class to process Particle ID information for implant events
class PidProcessor : public EventProcessor {
public:
  ///Constructor
  ///param [in] slope : set slope for linear position correction of separator ToF
  ///param [in] intercept : set intercept for linear ToF correction
  ///param [in] threshold : set energy threshold for PIN to remove light ions
  PidProcessor(double slope, double intercept, double threshold);

  ///Default Destructor
  ~PidProcessor() = default;

  ///Declare Damm histograms used in the analysis
  virtual void DeclarePlots(void);

  ///Preprocess the PID data
  ///param [in] event : the event to preprocess
  ///return true if successful
  virtual bool PreProcess(RawEvent &event);

  ///Process the PID data
  ///param [in] event : the event to process
  ///return true if successful
  //virtual bool Process(RawEvent &event);

private:

  double slope_ = 0;
  double intercept_ = 0;
  double threshold_ = 0;

  double pin_energy = 0;

  double tof = 0;
  double position = 0;
  double corrected_tof = 0;


  ///Functions for converting raw measurements into PID info
  double CorrectTofByPosition(double &tof, double &position, double &slope, double &intercept);

  double ConvertTofToAQ(double &tof);

  double ConvertPinToZ(double &pin);

  processor_struct::PID pid_struct;


};

#endif  //__PID_PROCESSOR_HPP_
