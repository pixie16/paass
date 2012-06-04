/** \file WaveformProcessor.hpp
 * 
 * Class for handling Scintillator traces. 
 */

#ifndef __WAVEFORMANALYZER_HPP_
#define __WAVEFORMANALYZER_HPP_

#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

class WaveformAnalyzer : public TraceAnalyzer
{
 public:
    struct FitData {
	size_t n;
	double * y;
	double * sigma;
	float WID;
	float DKAY;
    };

    WaveformAnalyzer(); // no virtual c'tors
    virtual void DeclarePlots(void);
    virtual void Analyze(Trace &, const std::string &,
			 const std::string &);
    
    virtual ~WaveformAnalyzer() {/* do nothing */};
};

#endif // __WAVEFORMANALYZER_HPP_
