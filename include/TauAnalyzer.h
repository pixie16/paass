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
  private:
  std::string type;
  std::string subtype;
  public:
    TauAnalyzer();
    TauAnalyzer(const std::string &aType, const std::string &aSubtype);

    ~TauAnalyzer();
    virtual void Analyze(Trace &trace, const std::string &aType, const std::string &aSubtype);
};

#endif // __TAUANALYZER_H_
