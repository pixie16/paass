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

/** \brief quick online trace analysis
 *
 * Trace class implements a quick online trapezoidal fiter for the
 * identification of double pulses with relatively little computation.
 */

class DoubleTraceAnalyzer : public TraceFilterer
{
 protected:
    Trace::size_type time2; ///< time of second pulse
    double energy2;         ///< energy of second pulse
    
    int numDoubleTraces; ///< number of double traces found
 public:
    DoubleTraceAnalyzer();
    virtual ~DoubleTraceAnalyzer();
    
    virtual bool Init(void) {return TraceFilterer::Init();}
    virtual void DeclarePlots(void);
    virtual void Analyze(Trace &, const std::string &, const std::string &);
};

#endif // __DOUBLETRACEANALYZER_HPP_
