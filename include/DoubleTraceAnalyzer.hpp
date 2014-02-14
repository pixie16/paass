/**   \file DoubleTraceAnalyzer.hpp
 *    \brief Header file for the TraceAnalyzer class
 *
 *    SNL - 7-2-07
 */

#ifndef __DOUBLETRACEANALYZER_HPP_
#define __DOUBLETRACEANALYZER_HPP_

#include <string>
#include <sys/times.h>

#include "Trace.hpp"
#include "TraceFilterer.hpp"

/** \brief quick online trace analysis
 *
 * Trace class implements a quick online trapezoidal fiter for the
 * identification of double pulses with relatively little computation.
 */

class DoubleTraceAnalyzer : public TraceFilterer
{
 protected:
    static int numDoubleTraces; ///< number of double traces found

 public:
    DoubleTraceAnalyzer(short fast_rise, short fast_gap, short fast_threshold,
                        short energy_rise, short energy_gap,
                        short slow_rise, short slow_gap, short slow_threshold);
    virtual ~DoubleTraceAnalyzer();
    
    virtual bool Init(void) {return TraceFilterer::Init();}
    virtual void DeclarePlots(void);
    virtual void Analyze(Trace &, const std::string &, const std::string &);
};

#endif // __DOUBLETRACEANALYZER_HPP_
