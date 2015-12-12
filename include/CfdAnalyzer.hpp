/** \file CfdAnalyzer.hpp
 * \brief Class to use Cfd on traces
 * \author S. V. Paulauskas
 * \date 22 July 2011
 */
#ifndef __CFDANALYZER_HPP_
#define __CFDANALYZER_HPP_

#include "HighResTimingData.hpp"
#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

//! Class to analyze traces using a digital CFD
class CfdAnalyzer : public TraceAnalyzer {
public:
    /** Default constructor */
    CfdAnalyzer();
    /** Default Destructor */
    ~CfdAnalyzer(){};
    /** Declare the plots */
    virtual void DeclarePlots(void) const {};
    /** Do the analysis on traces
    * \param [in] trace : the trace to analyze
    * \param [in] detType : the detector type
    * \param [in] detSubtype : detector subtype */
    virtual void Analyze(Trace &trace, const std::string &detType,
                         const std::string &detSubtype,
                         const std::map<std::string, int> &tagMap);
};
#endif
