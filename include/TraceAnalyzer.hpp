/**   \file TraceAnalyzer.hpp
 *    \brief Header file for the TraceAnalyzer class
 *
 *    SNL - 7-2-07
 */

#ifndef __TRACEANALYZER_HPP_
#define __TRACEANALYZER_HPP_

#include <string>
#include <sys/times.h>

#include "Plots.hpp"
#include "Trace.hpp"
//class Trace;

/** \brief quick online trace analysis
 *
 *  Simple class which is the basis for all types of trace analysis 
 */

class TraceAnalyzer {
 private:

    // things associated with timing
    tms tmsBegin;             ///< time at which the analyzer began
    double userTime;          ///< user time used by this class
    double systemTime;        ///< system time used by this class
    double clocksPerSecond;   ///< frequency of system clock

 protected:
    int level;                ///< the level of analysis to proceed with
    static int numTracesAnalyzed;    ///< rownumber for DAMM spectrum 850
    std::string name;         ///< name of the analyzer

 public:
    TraceAnalyzer();
    virtual ~TraceAnalyzer();
    
    virtual bool Init(void);
    virtual void DeclarePlots(void);
    virtual void Analyze(Trace &trace, 
			 const std::string &type, const std::string &subtype);
    void EndAnalyze(Trace &trace);
    void EndAnalyze(void);
    void SetLevel(int i) {level=i;}
    int  GetLevel() {return level;}
};

#endif // __TRACEANALYZER_HPP_
