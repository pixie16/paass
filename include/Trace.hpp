/** \file Trace.hpp
 * \brief A simple class to store the traces.
 *
 * A simple class to store the traces.
 * Used instead of a typedef so additional functionality can be added later.
 */
#ifndef __TRACE_HPP_
#define __TRACE_HPP_

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "Plots.hpp"
#include "PlotsRegister.hpp"
#include "TimingInformation.hpp"

/** A macro defining what kind of NAN to throw */
#ifndef NAN
#include <limits>
#define NAN (numeric_limits<float>::quiet_NaN())
#endif

class TrapezoidalFilterParameters; //!< Forward declaration of class for trap filter

//! Use an alias in this file to make things a bit more readable
namespace {
    typedef class TrapezoidalFilterParameters TFP;
}

//! \brief Store the information for a trace
class Trace : public std::vector<int> {
public:
    /** Default constructor */
    Trace() : std::vector<int>() {
        baselineLow = baselineHigh = pixie::U_DELIMITER;
    }

    /** An automatic conversion for the trace
    * \param [in] x : the trace to store in the class */
    Trace(const std::vector<int> &x) : std::vector<int>(x) {
        baselineLow = baselineHigh = pixie::U_DELIMITER;
    }

    /** calculation of the trapezoidal filter
    * \param [in] filter : the filter for the trace
    * \param [in] parms : the parameters for the filter
    * \param [in] lo : the low range for the filter */
    void TrapezoidalFilter(Trace &filter, const TFP &parms,
                           unsigned int lo = 0) const {
        TrapezoidalFilter(filter, parms, lo, size());
    }


    /**
    * Defines how to implement a trapezoidal filter characterized by two
    * moving sum windows of width risetime separated by a length gaptime.
    * Filter is calculated from channels lo to hi.
    * \param [in] filter : the filter for the trace
    * \param [in] parms : the parameters for the filter
    * \param [in] lo : the low range for the filter
    * \param [in] hi : the high range for the filter */
    void TrapezoidalFilter(Trace &filter, const TFP &parms,
                           unsigned int lo, unsigned int hi) const;

    /** Insert a value into the trace map
    * \param [in] name : the name of the parameter to insert
    * \param [in] value : the value to insert into the map */
    void InsertValue(std::string name, double value) {
        doubleTraceData.insert(make_pair(name,value));
    }

    /** Insert an int value into the trace
    * \param [in] name : the name of the variable to insert
    * \param [in] value : The integer value to insert into the map */
    void InsertValue(std::string name, int value) {
        intTraceData.insert(make_pair(name,value));
    }

    /** Set the double value of a parameter in the trace
    * \param [in] name : the name of the parameter to set
    * \param [in] value : the double value to set the parameter to */
    void SetValue(std::string name, double value) {
        if(doubleTraceData.count(name) > 0)
            doubleTraceData[name] = value;
        else
            InsertValue(name,value);
    }

    /** Set the integer value of a parameter in the trace
    * \param [in] name : the name of the parameter to set
    * \param [in] value : the int value to set the parameter to */
    void SetValue(std::string name, int value) {
        if(intTraceData.count(name) > 0)
            intTraceData[name] = value;
        else
            InsertValue(name,value);
    }

    /** Checks to see if a parameter has a value
    * \param [in] name : the name of the parameter to check for
    * \return true if the value exists in the trace */
    bool HasValue(std::string name) const {
        return (doubleTraceData.count(name) > 0 ||
                intTraceData.count(name) > 0);
    }

    /** Returns the value of the requested parameter
    * \param [in] name : the name of the parameter to get for
    * \return the requested value */
    double GetValue(std::string name) const {
        if(doubleTraceData.count(name) > 0)
            return (*doubleTraceData.find(name)).second;
        if(intTraceData.count(name) > 0)
            return (*intTraceData.find(name)).second;
        return NAN;
    }

    /** \return Returns the waveform found inside the trace */
    std::vector<double> GetWaveform() {
        return waveform;
    };

    /** Performs the baseline calculation
    * \param [in] lo : the low range for the baseline calculation
    * \param [in] numBins : The number of bins for the baseline calculation
    * \return The average value of the baseline in the region */
    double DoBaseline(unsigned int lo = 0, unsigned int numBins = numBinsBaseline);

    /** Performs the neutron-gamma discrimination on the traces
    * \param [in] lo : The low range for the discrimination (referenced from max)
    * \param [in] numBins : the number of bins to calculate the baseline over
    * \return The discrimination value */
    double DoDiscrimination(unsigned int lo, unsigned int numBins);

    /** Calculate the waveform and QDC for the trace
    * \param [in] lo : the lo value to for the QDC
    * \param [in] numBins : the number of bins to calculate the QDC over
    * \return The QDC for the waveform */
    double DoQDC(unsigned int lo, unsigned int numBins);

    /** Calculate information for the maximum value of the trace
    * \param [in] lo : the low value for the calculation
    * \param [in] numBins : the number of bins to look for the max in
    * \return The position of the maximum value in the trace */
    unsigned int FindMaxInfo(unsigned int lo = 10, unsigned int numBins = 15);

    /*! \brief Declares a 1D histogram calls the C++ wrapper for DAMM
    * \param [in] dammId : The histogram number to define
    * \param [in] xSize : The range of the x-axis
    * \param [in] title : The title for the histogram */
    virtual void DeclareHistogram1D(int dammId, int xSize, const char* title) {
        histo.DeclareHistogram1D(dammId, xSize, title);
    }

    /*! \brief Declares a 2D histogram calls the C++ wrapper for DAMM
    * \param [in] dammId : The histogram number to define
    * \param [in] xSize : The range of the x-axis
    * \param [in] ySize : The range of the y-axis
    * \param [in] title : The title of the histogram */
    virtual void DeclareHistogram2D(int dammId, int xSize, int ySize,
                                    const char* title) {
        histo.DeclareHistogram2D(dammId, xSize, ySize, title);
    }

    /*! \brief Implementation of the plot command to interface with the DAMM
    * routines
    *
    * This is also done in the EventProcessor class, redundant?
    * \param [in] dammId : The histogram number to plot into
    * \param [in] val1 : The x value to plot
    * \param [in] val2 : The y value to plot (if 2D histogram)
    * \param [in] val3 : The z value to plot (if 2D histogram)
    * \param [in] name : The name of the histogram */
    virtual void plot(int dammId, double val1, double val2 = -1,
                      double val3 = -1, const char* name="h") const {
        histo.Plot(dammId, val1, val2, val3, name);
    }

    /** plot trace into a 1D histogram
    * \param [in] id : histogram ID to plot into */
    void Plot(int id);
    /** plot trace into row of a 2D histogram
    * \param [in] id : histogram ID to plot into
    * \param [in] row : the row to plot into */
    void Plot(int id, int row);
    /** plot trace absolute value and scaled into a 1D histogram
    * \param [in] id : histogram ID to plot into
    * \param [in] scale : the scaling for the trace */
    void ScalePlot(int id, double scale);
    /** plot trace absolute value and scaled into a 2D histogram
    * \param [in] id : histogram ID to plot into
    * \param [in] row : the row to plot the histogram into
    * \param [in] scale : the scaling for the trace */
    void ScalePlot(int id, int row, double scale);
    /** plot trace with a vertical offset in a 1D histogram
    * \param [in] id : histogram ID to plot into
    * \param [in] offset : the offset for the trace */
    void OffsetPlot(int id, double offset);
    /** plot trace with a vertical offset in a 2D histogram
    * \param [in] id : histogram ID to plot into
    * \param [in] row : the row to plot the trace into
    * \param [in] offset : the offset for the trace*/
    void OffsetPlot(int id, int row, double offset);

private:
    static const unsigned int numBinsBaseline = 15; //!< Number of bins in the baseline
    unsigned int baselineLow; //!< low range for the baseline
    unsigned int baselineHigh;//!< high range for the baseline

    std::vector<double> waveform; //!< The waveform inside the trace

    std::map<std::string, double> doubleTraceData; //!< Trace data stored as doubles
    std::map<std::string, int>    intTraceData;//!< Trace data stored as ints

    /** This field is static so all instances of Trace class have access to
     * the same plots and plots range. */
    static Plots histo;
};

//! \brief Parameters for your typical trapezoidal filter
class TrapezoidalFilterParameters {
public:
    /** Default Constructor */
    TrapezoidalFilterParameters(){};

    /** Constructor taking the L, G, and t as arguements
    * \param [in] gap : The flattop for the filter
    * \param [in] rise : the risetime for the filter
    * \param [in] t : Either the tau or the trigget threshold for the filter */
    TrapezoidalFilterParameters(int gap, int rise, double t = NAN) :
        gapSamples(gap), riseSamples(rise), tau(t) {};

    /** Constructor taking another instance of the TrapzeoidalFilterParameters class
    * \param [in] x : A different instance of the TrapzeoidalFilterParameters class */
    TrapezoidalFilterParameters(const TFP &x) :
        gapSamples(x.gapSamples), riseSamples(x.riseSamples),
        tau(x.tau) {};

    /** Copy constructor for the class
    * \param [in] right : A different instance of the TrapzeoidalFilterParameters class
    * \return the new instance of the TFP */
    const TFP& operator=(const TFP &right) {
        gapSamples = right.gapSamples;
        riseSamples = right.riseSamples;
        tau = right.tau;

        return (*this);
    }

    //! \return the value of the Gap
    Trace::size_type GetGapSamples(void) const {return gapSamples;};

    //! \return the value of the Risetime
    Trace::size_type GetRiseSamples(void) const {return riseSamples;};

    //! \return the length of the filter
    Trace::size_type GetSize(void) const {return 2*riseSamples + gapSamples;};

    //! \return the value of tau
    double GetTau(void) const {return tau;};
private:
    Trace::size_type gapSamples; //!< number of samples in the gap
    Trace::size_type riseSamples; //!< number of samples in the rise time

    double tau; //!< Tau of the energy filter
};

extern const Trace emptyTrace; //!< Instance of an empty trace for people to grab
#endif // __TRACE_H_
