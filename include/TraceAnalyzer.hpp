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
#include "Trace.hpp"

/** \brief Online trace analysis
 *
 *  Simple class which is the basis for all types of trace analysis
 */
class TraceAnalyzer {
 private:
    tms tmsBegin;             ///< time at which the analyzer began
    double userTime;          ///< user time used by this class
    double systemTime;        ///< system time used by this class
    double clocksPerSecond;   ///< frequency of system clock

 protected:
    int level;                ///< the level of analysis to proceed with
    static int numTracesAnalyzed;    ///< rownumber for DAMM spectrum 850
    std::string name;         ///< name of the analyzer

 public:
     /** Default Constructor */
    TraceAnalyzer();
    /** Default Destructor */
    virtual ~TraceAnalyzer();

    /** Initializes the Analyzer
    * \return True if the init was successful */
    virtual bool Init(void){return(true);};
    /** Declare Plots (empty for now) */
    virtual void DeclarePlots(void){};
    /** Function to analyze a trace online.
    * \param [in] trace: the trace
    * \param [in] type : the type of detector
    * \param [in] subtype : the subtype of the detector
    */
    virtual void Analyze(Trace &trace, const std::string &type,
                         const std::string &subtype);

    /** Function to analyze a trace online.
     * \param [in] trace: the trace
    * \param [in] type : the type of detector
    * \param [in] subtype : the subtype of the detector
    * \param [in] tagMap : takes a map of all the tags that the channel has
    */
    virtual void Analyze(Trace &trace, const std::string &type,
                         const std::string &subtype,
                         const std::map<std::string, int> & tagMap);
    /** End the analysis and record the analyzer level in the trace
    * \param [in] trace : the trace
    */
    void EndAnalyze(Trace &trace);
    /** Finish analysis updating the analyzer timing information */
    void EndAnalyze(void);
    /** Set the level of the trace analysis
    * \param [in] i : the level of the analysis to be done */
    void SetLevel(int i) {level=i;}
    /** \return the level of the trace analysis */
    int  GetLevel() {return level;}
};
#endif // __TRACEANALYZER_HPP_
