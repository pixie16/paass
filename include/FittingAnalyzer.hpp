/** \file FittingAnalyzer.hpp
 * \brief Class to fit functions to waveforms
 */
#ifndef __FITTINGANALYZER_HPP_
#define __FITTINGANALYZER_HPP_

#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

class FittingAnalyzer : public TraceAnalyzer
{
 public:
    FittingAnalyzer();
    virtual ~FittingAnalyzer() {};

    virtual void DeclarePlots(void);
    virtual void Analyze(Trace &, const std::string &, const std::string &);
 
    struct FitData{
	size_t n;
	double * y;
	double * sigma;
	double beta,gamma,qdc;
    };
};
#endif // __FITTINGANALYZER_HPP_
// David is awesome.
