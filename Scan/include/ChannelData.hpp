#ifndef __CHANNELDATA_HPP__
#define __CHANNELDATA_HPP__

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <cstdlib>

#include <XiaData.hpp>

class ChannelData{
public:    
    /// Default constructor.
    ChannelData();
    
    /// Constructor from a XiaData. ChannelData will take ownership of the XiaData.
    ChannelData(XiaData *event_);
    
    /// Destructor.
    ~ChannelData();
    
    /// Find the leading edge of the pulse at a given percentage of pulse maximum.
    float FindLeadingEdge(const float &thresh_=0.05);
    
    /// Clear all variables and clear the trace vector and arrays.
    void Clear();
private:
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
};

#endif
