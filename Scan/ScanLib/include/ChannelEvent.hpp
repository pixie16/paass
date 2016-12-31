///@file ChannelEvent.hpp
///@author S. V. Paulauskas (from C. R. Thornsberry's work)
///@date December 2, 2016

#ifndef PIXIESUITE_CHANNELEVENT_HPP
#define PIXIESUITE_CHANNELEVENT_HPP

#include "XiaData.hpp"

class ChannelEvent{
public:
    bool valid_chan; /// True if the high resolution energy and time are valid.

    double hires_energy; /// High resolution energy from the integration of pulse fits.
    double hires_time; /// High resolution time taken from pulse fits (in ns).

    float *xvals; /// x values used for fitting.
    float *yvals; /// y values used for fitting (baseline corrected trace).
    float *cfdvals; /// y values for the cfd analyzed waveform.
    size_t size; /// Size of xvals and yvals arrays and of trace vector.

    float phase; /// Phase (leading edge) of trace (in ADC clock ticks (4E-9 Hz for 250 MHz digitizer)).
    float baseline; /// The baseline of the trace.
    float stddev; /// Standard deviation of the baseline.
    float maximum; /// The baseline corrected maximum value of the trace.
    float qdc; /// The calculated (baseline corrected) qdc.
    float cfdCrossing; /// The zero-crossing point of the cfd waveform.
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

    /// Perform CFD analysis on the waveform.
    float AnalyzeCFD(const float &F_=0.5, const size_t &D_=1, const size_t &L_=1);

    /// Clear all variables and clear the trace vector and arrays.
    void Clear();
};
#endif //PIXIESUITE_CHANNELEVENT_HPP
