/**   \file TraceAnalyzer.h
 *    \brief Header file for the TraceAnalyzer class
 *
 *    SNL - 7-2-07
 */

#ifndef __TRACEANALYZER_H_
#define __TRACEANALYZER_H_

#include <string>
#include <vector>

#include <sys/times.h>

#include "Trace.h"

/** \brief quick online trace analysis
 *
 *  Simple class which is the basis for all types of trace analysis 
 */

class TraceAnalyzer {
 private:
    // things associated with timing
    double userTime;          ///< user time used by this class
    double systemTime;        ///< system time used by this class
    double clocksPerSecond;   ///< frequency of system clock

    int numTracesAnalyzed;    ///< rownumber for DAMM spectrum 850
 public:
    virtual bool Init(void);
    virtual void DeclarePlots(void) const;
    virtual void Analyze(const Trace &trace, 
			const std::string &type, const std::string &subtype);
    virtual void TracePlot(const Trace &);

    TraceAnalyzer();
    virtual ~TraceAnalyzer();
};

#endif // __TRACEANALYZER_H_
