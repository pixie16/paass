/**   \file DoubleTraceAnalyzer.h
 *    \brief Header file for the TraceAnalyzer class
 *
 *    SNL - 7-2-07
 */

#ifndef __DOUBLETRACEANALYZER_H_
#define __DOUBLETRACEANALYZER_H_

#include <string>
#include <sys/times.h>

#include "Trace.h"
#include "TraceFilterer.h"

//!!!! THIS NEEDS SOME CONSIDERABLE MODIFICATIONS --- DTM
// -- e.g. using new spiffy TrapezodialFilterParameters class

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
    virtual void DeclarePlots(void) const;
    virtual void Analyze(Trace &, const std::string &, const std::string &);
};

#endif // __DOUBLETRACEANALYZER_H_
