///@file PidProcessor.hpp
///@brief A class to handle VANDLE bars 
///Processes PID information and constructs PID plots for fragmentation-
///type experiments
///
///@author A. Keeler, R. Yokoyama
///@date 29 July 2019
///@details Updated for E19044 experiment

#ifndef __PIDPROCESSOR_HPP_
#define __PIDPROCESSOR_HPP_

#include <set>

#include "DammPlotIds.hpp"

#include "EventProcessor.hpp"
#include "RawEvent.hpp"
#include "PaassRootStruct.hpp"

/// Class to process Particle ID information for implant events
class PidProcessor : public EventProcessor {
public:
	/**
	 * @brief Constructor
	 * @param pin0 : channel location id of pin0
	 * @param pin1 : channel location id of pin1
	*/
	PidProcessor(const double &YSO_Implant_thresh,const double &FIT_thresh, const double &RIT_thresh);

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
	virtual bool Process(RawEvent &event);

private:

	///Functions for converting raw measurements into PID info
	virtual double CorrectTofByPosition(double &tof, double &position, double &slope, double &intercept);

	virtual double ConvertTofToAQ(double &tof);

	virtual double ConvertPinToZ(double &pin);

	double yso_threshold_; //threshold for yso implant gating
	double fit_threshold_; //threshold for fit implant gating
	double rit_threshold_; //threshold for rit implant gating

    //some tests
    double yso_energy_prev;
    double fit_energy_prev;
    double rit_energy_prev;
	processor_struct::PID pid_struct;


};

#endif  //__PID_PROCESSOR_HPP_
