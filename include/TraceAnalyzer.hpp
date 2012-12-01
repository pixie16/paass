/** \file TraceAnalyzer.hpp
 * \brief Header file for the TraceAnalyzer class
 * \author S. Liddick 
 * \date 02 July 2007
 */

#ifndef __TRACEANALYZER_HPP_
#define __TRACEANALYZER_HPP_

#include <string>
#include <sys/times.h>

#include "Plots.hpp"

class Trace;

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
    int numTracesAnalyzed;    ///< rownumber for DAMM spectrum 850
    std::string name;         ///< name of the analyzer

    Plots histo;
    virtual void plot(int dammId, double val1, double val2 = -1, double val3 = -1, 
                      const char* name="h") {
        histo.Plot(dammId, val1, val2, val3, name);
    }
    virtual void DeclareHistogram1D(int dammId, int xSize, const char* title) {
        histo.DeclareHistogram1D(dammId, xSize, title);
    }
    virtual void DeclareHistogram2D(int dammId, int xSize, int ySize, const char* title) {
        histo.DeclareHistogram2D(dammId, xSize, ySize, title);
    }

 public:
    TraceAnalyzer();
    TraceAnalyzer(int offset, int range);
    virtual ~TraceAnalyzer();
    
    virtual bool Init(void);
    virtual void DeclarePlots(void);
    virtual void Analyze(Trace &trace, const std::string &type, 
                         const std::string &subtype);
    void EndAnalyze(Trace &trace);
    void EndAnalyze(void);
    void SetLevel(int i) {level=i;}
    int  GetLevel() {return level;}
};

#endif // __TRACEANALYZER_HPP_
