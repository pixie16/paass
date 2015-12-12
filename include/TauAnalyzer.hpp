/*! \file TauAnalyzer.hpp
 * \brief A trace analyzer which determines the decay time constant of traces
 */
#ifndef __TAUANALYZER_HPP_
#define __TAUANALYZER_HPP_

#include <string>

#include "TraceAnalyzer.hpp"

//! Class to analyze Tau from a trace
class TauAnalyzer : public TraceAnalyzer {
public:
    /** Default Constructor */
    TauAnalyzer();

    /** Constructor taking the type and subtype to plot
    * \param [in] aType : a type to analyze tau from
    * \param [in] aSubtype : a subtype to analyze tau from */
    TauAnalyzer(const std::string &aType, const std::string &aSubtype);

    /** Default Destructor */
    ~TauAnalyzer(){};

    /** The main analysis driver
    * \param [in] trace : the trace to analyze
    * \param [in] aType : the type being analyze
    * \param [in] aSubtype : the subtype begin analyzed */
    virtual void Analyze(Trace &trace, const std::string &aType,
                         const std::string &aSubtype,
                         const std::map<std::string, int> & tagMap);
private:
    std::string type; //!< the detector type
    std::string subtype;//!< the detector subtype
};

#endif // __TAUANALYZER_HPP_
