/** \file WaveformAnalyzer.hpp
 * \brief Class to analyze traces. 
 */
#ifndef __WAVEFORMANALYZER_HPP_
#define __WAVEFORMANALYZER_HPP_

#include "TimingInformation.hpp"
#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

class WaveformAnalyzer : public TraceAnalyzer, 
			 public TimingInformation
{
 public:
    WaveformAnalyzer(); 
    virtual void DeclarePlots(void) const;
    virtual void Analyze(Trace &, const std::string &,
			 const std::string &);
    virtual ~WaveformAnalyzer() {};
};
#endif // __WAVEFORMANALYZER_HPP_
