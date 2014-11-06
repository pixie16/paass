/** \file DoubleTraceAnalyzer.hpp
 * \brief Header file for the TraceAnalyzer class
 * \author S. Liddick
 * \date 02 July 2007
 */
#ifndef __DOUBLETRACEANALYZER_HPP_
#define __DOUBLETRACEANALYZER_HPP_

#include <string>
#include <sys/times.h>

#include "Trace.hpp"
#include "TraceFilterer.hpp"

/** \brief Analyzer to handle double traces from RMS style experiments
 *
 * Trace class implements a quick online trapezoidal filter for the
 * identification of double pulses with relatively little computation.
 */
class DoubleTraceAnalyzer : public TraceFilterer {
protected:
    static int numDoubleTraces; ///< number of double traces found

public:
    /** Constructor taking all of the arguments for the filter
     * \param [in] energyScaleFactor : scaling for the energy
     * \param [in] fast_rise : the rise time for the fast filter
     * \param [in] fast_gap : the gap for the fast filter
     * \param [in] fast_threshold : the threshold for the first decay
     * \param [in] energy_rise : the energy filter risetime
     * \param [in] energy_gap : the gap for the energy filter
     * \param [in] slow_rise : the slow rise time
     * \param [in] slow_gap : the gap for the slow filter
     * \param [in] slow_threshold : the threshold for the second decay */
    DoubleTraceAnalyzer(double energyScaleFactor,
                        short fast_rise, short fast_gap, short fast_threshold,
                        short energy_rise, short energy_gap,
                        short slow_rise, short slow_gap, short slow_threshold);
    /** Default Destructor */
    virtual ~DoubleTraceAnalyzer() {};

    /** Initialize the processor
     * \return true if init went well */
    virtual bool Init(void) {
        return TraceFilterer::Init();
    }
    /** Declare plots for the analyzer */
    virtual void DeclarePlots(void);
    /** Do the analysis on traces
    * \param [in] trace : the trace to analyze
    * \param [in] type : the detector type
    * \param [in] subtype : detector subtype */
    virtual void Analyze(Trace &trace, const std::string &type,
                         const std::string &subtype);
};
#endif // __DOUBLETRACEANALYZER_HPP_
