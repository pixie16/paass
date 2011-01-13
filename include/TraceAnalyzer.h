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
 * Trace class implements a quick online trapezoidal fiter for the
 * identification of double pulses with relatively little computation.
 */

class TraceAnalyzer {
 private:
    // things associated with timing
    double userTime;          ///< user time used by this class
    double systemTime;        ///< system time used by this class
    double clocksPerSecond;   ///< frequency of system clock

    std::vector<double> average;   ///< trace average
    Trace fastFilter;   ///< fast filter of trace
    Trace energyFilter; ///< energy filter of trace
    Trace thirdFilter;  /*< third filter of trace, used as a second
			       * threshold check */

    Trace flt;         ///< vector used in filter function
        
    int t1;                  ///< time of E1 pulse
    int t2;                  ///< time of E2 pulse
    double e1;               ///< energy of E1 pulse
    double e2;               ///< energy of E2 pulse

    int rownum850;           ///< rownumber of DAMM spectrum 850
    int rownum870;           ///< rownumber of DAMM spectrum 870

    int fastRise;            ///< rise time of fast filter (in samples)
    int slowRise1;           ///< rise time of energy filter (in samples)
    int slowRise2;           ///< rise time of slow threshold filter (in samples)
    int fastGap;             ///< gap time of fast filter (in samples)
    int slowGap1;            ///< gap time of energy filter (in samples)
    int slowGap2;            ///< gap time of slow threshold filter (in samples)
    int fastThresh;          ///< threshold of fast filter
    int slowThresh;          ///< threshold of slow filter

    /** default filename containing filter parameters
     */
    static const std::string defaultFilterFile;
 public:
    int Init(const std::string &filterFile=defaultFilterFile);
    void DeclarePlots(void) const;
    int Analyze(const Trace &, const std::string &, const std::string &);
    Trace Filter(Trace &, int , int , int , int );
    void FilterFill(const Trace &, Trace &, int, int, int, int);
    void TracePlot(const Trace &);

    int GetTime(void) const {return t1;}
    int GetSecondTime(void) const {return t2;}
    double GetEnergy(void) const {return e1;}
    double GetSecondEnergy(void) const {return e2;}

    TraceAnalyzer();
    ~TraceAnalyzer();
};

#endif // __TRACEANALYZER_H_
