///@file RikenPidProcessor.hpp
///@brief A class to handle VANDLE bars 
///Processes PID information and constructs PID plots for fragmentation-
///type experiments
///
///@author I. Cox, T.T. King, A. Keeler, R. Yokoyama
///@date 29 July 2019
///@details Updated for RIBF168 experiment

#ifndef __RIKENPIDPROCESSOR_HPP_
#define __RIKENPIDPROCESSOR_HPP_

#include <set>

#include "DammPlotIds.hpp"

#include "EventProcessor.hpp"
#include "RawEvent.hpp"
#include "PaassRootStruct.hpp"

/// Class to process Particle ID information for implant events
class RikenPidProcessor : public EventProcessor {
public:
	/**
	 * @brief Constructor
	 * @param pin0 : channel location id of pin0
	 * @param pin1 : channel location id of pin1
	*/
	RikenPidProcessor(const double &YSO_Implant_thresh,const double &FIT_thresh, const double &RIT_thresh, const bool &tofflip);

	///Default Destructor
	~RikenPidProcessor() = default;

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

	double yso_threshold_; //threshold for yso implant gating
	double fit_threshold_; //threshold for fit implant gating
	double rit_threshold_; //threshold for rit implant gating
	bool tofflip_; //flip tof in the PID plots (DAMM)

	bool root_output;
	//some tests
	double yso_energy_prev;
	double fit_energy_prev;
	double rit_energy_prev;
	processor_struct::PID pid_struct;


};

#endif  //__PID_PROCESSOR_HPP_
