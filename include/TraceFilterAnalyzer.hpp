/** \file TraceFilterAnalyzer.hpp
 * \brief Declaration of analysis class which does the filtering of traces.
 * \author S. V. Paulauskas; originally D. Miller
 * \date January 2011
 */

#ifndef __TRACEFILTERANALYZER_HPP__
#define __TRACEFILTERANALYZER_HPP__

#include <string>
#include <vector>

#include "Trace.hpp"
#include "TraceAnalyzer.hpp"
#include "TrapFilterParameters.hpp"

//! \brief A class to perform trapezoidal filters on the traces
class TraceFilterAnalyzer : public TraceAnalyzer {
public:
    /** Default Constructor */
    TraceFilterAnalyzer(){};

    /** Constructor taking the trigger and energy filter parameters
     * \param [in] t : trigger filter parameters
     * \param [in] e : energy filter parameters*/
    TraceFilterAnalyzer(const TrapFilterParameters &t,
			const TrapFilterParameters &e);

    /** Default Destructor */
    virtual ~TraceFilterAnalyzer(){};

    /** Declare the plots for the Analyzer */
    virtual void DeclarePlots(void);

    /** The analyzer method to do the analysis
     * \param [in] trace : the trace to analyze
     * \param [in] type : the detector type
     * \param [in] subtype : the detector subtype */
    virtual void Analyze(Trace &trace, const std::string &type,
                         const std::string &subtype,
                         const std::map<std::string,int> &tagmap);
private:
    TrapFilterParameters trigPars_; //!< Trigger filter parameters
    TrapFilterParameters enPars_; //!< energy filter parametersf
    std::vector<double> fastFilter;   //!< fast filter of trace
    std::vector<double> energyFilter; //!< slow filter of trace

};
#endif // __TRACEFILTERER_HPP_
