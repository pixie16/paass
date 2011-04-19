/* \file TauAnalyzer.h 
 *
 * A trace analyzer which determines the decay time constant of traces
 */

#ifndef __TAUANALYZER_H_
#define __TAUANALYZER_H_

#include <string>

#include "TraceAnalyzer.h"

class TauAnalyzer : public TraceAnalyzer
{
  public:
    TauAnalyzer();
    ~TauAnalyzer();
    virtual void Analyze(Trace &trace, const std::string &type, const std::string &subtype);
};

#endif // __TAUANALYZER_H_
