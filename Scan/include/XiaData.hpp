#ifndef XIADATA_HPP
#define XIADATA_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h>

/*! \brief A pixie16 channel event
 *
 * All data is grouped together into channels.  For each pixie16 channel that
 * fires the energy, time (both trigger time and event time), and trace (if
 * applicable) are obtained.  Additional information includes the channels
 * identifier, calibrated energies, trace analysis information.
 * Note that this currently stores raw values internally through pixie word types
 *   but returns data values through native C types. This is potentially non-portable.
 */
class XiaData{
  public:
	double energy; /// Raw pixie energy.
	double time; /// Raw pixie event time. Measured in filter clock ticks (8E-9 Hz for RevF).

	std::vector<int> adcTrace; /// ADC trace capture.

	static const int numQdcs = 8; /// Number of QDCs onboard.
	unsigned int qdcValue[numQdcs]; /// QDCs from onboard.

	unsigned int modNum; /// Module number.
	unsigned int chanNum; /// Channel number.
	unsigned int trigTime; /// The channel trigger time, trigger time and the lower 32 bits of the event time are not necessarily the same but could be separated by a constant value.
	unsigned int cfdTime; /// CFD trigger time in units of 1/256 pixie clock ticks.
	unsigned int eventTimeLo; /// Lower 32 bits of pixie16 event time.
	unsigned int eventTimeHi; /// Upper 32 bits of pixie16 event time.
	double eventTime; /// The event time recorded by Pixie.

	bool virtualChannel; /// Flagged if generated virtually in Pixie DSP.
	bool pileupBit; /// Pile-up flag from Pixie.
	bool saturatedBit; /// Saturation flag from Pixie.
	bool cfdForceTrig; /// CFD was forced to trigger.
	bool cfdTrigSource; /// The ADC that the CFD/FPGA synched with.

	/// Default constructor.
	XiaData();
	
	/// Constructor from a pointer to another XiaData.
	XiaData(XiaData *other_);
	
	/// Virtual destructor.
	virtual ~XiaData();

	/// Get the event ID number (mod * chan).
	int getID(){ return modNum*chanNum; }
	
	/// Reserve specified number of bins for the channel trace.
	void reserve(const size_t &size_);
	
	/// Fill the trace vector with a specified value.
	void assign(const size_t &size_, const int &input_);
	
	/// Push back the trace vector with a value.
	void push_back(const int &input_); 

	/// Return true if the time of arrival for rhs is later than that of lhs.
	static bool compareTime(XiaData *lhs, XiaData *rhs){ return (lhs->time < rhs->time); }
	
	/// Return true if lhs has a lower event id (mod * chan) than rhs.
	static bool compareChannel(XiaData *lhs, XiaData *rhs){ return ((lhs->modNum*lhs->chanNum) < (rhs->modNum*rhs->chanNum)); }
	
	/// Return one of the onboard qdc values.
	unsigned int getQdcValue(int id){ return (id < 0 || id >= numQdcs ? -1 : qdcValue[id]); }
	
	/// Clear all variables.
	void clear();
};

class ChannelEvent{
  public:
	bool valid_chan; /// True if the high resolution energy and time are valid.

	double hires_energy; /// High resolution energy from the integration of pulse fits.
	double hires_time; /// High resolution time taken from pulse fits (in ns).

	float *xvals; /// x values used for fitting.
	float *yvals; /// y values used for fitting (baseline corrected trace).
	size_t size; /// Size of xvals and yvals arrays and of trace vector.
	
	float phase; /// Phase (leading edge) of trace (in ADC clock ticks (4E-9 Hz for 250 MHz digitizer)).
	float baseline; /// The baseline of the trace.
	float stddev; /// Standard deviation of the baseline.
	float maximum; /// The baseline corrected maximum value of the trace.
	float qdc; /// The calculated (baseline corrected) qdc.
	size_t max_index; /// The index of the maximum trace bin (in ADC clock ticks).

	bool baseline_corrected; /// True if the trace has been baseline corrected.
	bool ignore; /// Ignore this event.
	
	XiaData *event; /// The low level pixie event.
	
	/// Default constructor.
	ChannelEvent();
	
	/// Constructor from a XiaData. ChannelEvent will take ownership of the XiaData.
	ChannelEvent(XiaData *event_);
	
	/// Destructor.
	~ChannelEvent();
	
	/// Correct the trace baseline, baseline standard deviation, and find the pulse maximum.
	float CorrectBaseline();
	
	/// Find the leading edge of the pulse at a given percentage of pulse maximum.
	float FindLeadingEdge(const float &thresh_=0.05);
	
	/// Integrate the baseline corrected trace in the range [start_, stop_] and return the result.
	float IntegratePulse(const size_t &start_=0, const size_t &stop_=0);
	
	/// Integrate the baseline corrected trace in the range [start_, stop_] and return the result.
	float FindQDC(const size_t &start_=0, const size_t &stop_=0);
	
	/// Clear all variables and clear the trace vector and arrays.
	void Clear();
};

#endif
