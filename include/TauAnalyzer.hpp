/*! \file TauAnalyzer.hpp
 * \brief A trace analyzer which determines the decay time constant of traces
 */
#ifndef __TAUANALYZER_HPP_
#define __TAUANALYZER_HPP_

#include <string>

#include "TraceAnalyzer.hpp"

class TauAnalyzer : public TraceAnalyzer {
public:
    TauAnalyzer();
    TauAnalyzer(const std::string &aType, const std::string &aSubtype);

    ~TauAnalyzer(){};
    virtual void Analyze(Trace &trace, const std::string &aType, const std::string &aSubtype);
private:
    std::string type;
    std::string subtype;
};

#endif // __TAUANALYZER_HPP_
