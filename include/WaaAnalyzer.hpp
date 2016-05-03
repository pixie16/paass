/** \file WaaAnalyzer.hpp
 * \brief Class to perform Weighted Average algorithm on waveformms
 * \author S. V. Paulauskas
 * \date August 13, 2013
 */
#ifndef __WAAANALYZER_HPP_
#define __WAAANALYZER_HPP_

#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

class WaaAnalyzer : public TraceAnalyzer {
 public:
    /** Default Constructor */
    WaaAnalyzer();
    /** Default Destructor */
    ~WaaAnalyzer() {};
    /** Declare plots for the analyzer */
    virtual void DeclarePlots(void);
    /** Analyzes the traces
     * \param [in] trace : the trace to analyze
     * \param [in] detType : the detector type we have
     * \param [in] detSubtype : the subtype of the detector
     * \param [in] tagMap : Map of tags associated with trace*/
    virtual void Analyze(Trace &trace, const std::string &detType,
                         const std::string &detSubtype,
                         const std::map<std::string, int> & tagMap);
 private:
};
#endif // __WAAANALYZER_HPP_
// David is awesome.
