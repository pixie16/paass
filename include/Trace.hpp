/** \file Trace.hpp
 * \brief A simple class to store the traces.
 *
 * Used instead of a typedef so additional functionality can be added later.
 */

#ifndef __TRACE_HPP_
#define __TRACE_HPP_ 1

#include <map>
#include <string>
#include <vector>

#include <cmath>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "Plots.hpp"
#include "PlotsRegister.hpp"
#include "TimingInformation.hpp"

#ifndef NAN
#include <limits>
#define NAN (numeric_limits<float>::quiet_NaN())
#endif

class TrapezoidalFilterParameters;

// use an alias in this file to make things a bit more readable
namespace {
    typedef class TrapezoidalFilterParameters TFP;
}

/**
   Store the information for a trace
 */
class Trace : public std::vector<int>
{
 private:
    static const unsigned int numBinsBaseline = 15;
    unsigned int baselineLow; 
    unsigned int baselineHigh;

    std::vector<double> waveform;

    std::map<std::string, double> doubleTraceData;
    std::map<std::string, int>    intTraceData;

    /** This field is static so all instances of Trace class have access to 
     * the same plots and plots range. */
    static Plots histo; 

 public:
     
    Trace() : std::vector<int>()
	{baselineLow = baselineHigh = U_DELIMITER; };
    // an automatic conversion
    Trace(const std::vector<int> &x) : std::vector<int>(x) {
        baselineLow = baselineHigh = U_DELIMITER;
    }

    void TrapezoidalFilter(Trace &filter, const TFP &parms,
			   unsigned int lo = 0) const {
        TrapezoidalFilter( filter, parms, lo, size() );
    }
    void TrapezoidalFilter(Trace &filter, const TFP &parms,
			   unsigned int lo, unsigned int hi) const;

    void InsertValue(std::string name, double value) {
        doubleTraceData.insert(make_pair(name,value));
    }

    void InsertValue(std::string name, int value) {
        intTraceData.insert(make_pair(name,value));	
    }

    void SetValue(std::string name, double value) {
        if (doubleTraceData.count(name) > 0) 
            doubleTraceData[name] = value;
        else
            InsertValue(name,value);
    }

    void SetValue(std::string name, int value) {
        if (intTraceData.count(name) > 0)
            intTraceData[name] = value;
        else
            InsertValue(name,value);
    }

    bool HasValue(std::string name) const {
        return (doubleTraceData.count(name) > 0 ||
            intTraceData.count(name) > 0);
    }

    double GetValue(std::string name) const {
        if (doubleTraceData.count(name) > 0)
            return (*doubleTraceData.find(name)).second;
        if (intTraceData.count(name) > 0)
            return (*intTraceData.find(name)).second;
        return NAN;
    }

    std::vector<double> GetWaveform(){return waveform;}; 

    //To allow access to the variables related to timing
    //This is needed in order to calculate the baseline over the 
    //range that immediately preceedes the trace. -SVP
    TimingInformation constants;
    
    double DoBaseline(unsigned int lo = 0, unsigned int numBins = numBinsBaseline);
    double DoDiscrimination(unsigned int lo, unsigned int numBins);
    double DoQDC(unsigned int lo, unsigned int numBins);
    
    unsigned int FindMaxInfo(void);
    
    void Plot(int id);           //< plot trace into a 1D histogram
    void Plot(int id, int row);  //< plot trace into row of a 2D histogram
    void ScalePlot(int id, double scale); //< plot trace absolute value and scaled into a 1D histogram
    void ScalePlot(int id, int row, double scale); //< plot trace absolute value and scaled into a 2D histogram
    void OffsetPlot(int id, double offset); // plot trace with a vertical offset in a 1D histogram
    void OffsetPlot(int id, int row, double offset); //plot trace with a vertical offset in a 2D histogram
};

/** Parameters for your typical trapezoidal filter */
class TrapezoidalFilterParameters
{
 private:
    Trace::size_type gapSamples;
    Trace::size_type riseSamples;
    
    double tau;
 public:
    TrapezoidalFilterParameters(int gap, int rise, double t = NAN) :
	gapSamples(gap), riseSamples(rise), tau(t) {};
    TrapezoidalFilterParameters(const TFP &x) :
	gapSamples(x.gapSamples), riseSamples(x.riseSamples),
	tau(x.tau) {};
    const TFP& operator=(const TFP &right) {
	gapSamples = right.gapSamples;
	riseSamples = right.riseSamples;
	tau = right.tau;

	return (*this);
    }
    Trace::size_type GetGapSamples(void) const  {return gapSamples;}
    Trace::size_type GetRiseSamples(void) const {return riseSamples;}
    Trace::size_type GetSize(void) const  
	{return 2*riseSamples + gapSamples;}
    double GetTau(void) const      {return tau;}
};

extern const Trace emptyTrace;

#endif // __TRACE_H_
