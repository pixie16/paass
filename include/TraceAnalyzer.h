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

using std::string;
using std::vector;

/** \brief quick online trace analysis
 *
 * Trace class implements a quick online trapezoidal fiter for the
 * identification of double pulses with relatively little computation.
 */

class TraceAnalyzer {
 private:
    // things associated with timing
    double userTime;         ///< user time used by this class
    double systemTime;       ///< system time used by this class
    double clocksPerSecond;  ///< frequency of system clock

    vector<double> average;  ///< trace average
    vector<int> v40;         ///< filter with rise= 4, gap= 4
    vector<int> v50;         ///< filter with rise= 10, gap= 10
    vector<int> v60;         ///< filter with rise= 10, gap= 20
    vector<int> v70;         ///< filter not used
    
    vector<int> flt;         ///< vector used in filter function
    
    int thrtime1;            ///< time of E1 pulse
    int thrtime2;            ///< time of E2 pulse
    double e1;               ///< energy of E1 pulse
    double e2;               ///< energy of E2 pulse
    int rownum850;           ///< rownumber of DAMM spectrum 850
    int rownum870;           ///< rownumber of DAMM spectrum 870
    
 public:
    int Init();
    void DeclarePlots(void) const;
    int Analyze(const vector<int> &, vector<double> &, 
		const string &, const string &);
    vector<int> Filter(vector<int> &, int , int , int , int );
    void FilterFill(const vector<int> &, vector<int> &, int, int, int, int);
    void TracePlot(const vector<int> &, const vector<double> &);
    
    TraceAnalyzer();
    ~TraceAnalyzer();
};

#endif // __TRACEANALYZER_H_
