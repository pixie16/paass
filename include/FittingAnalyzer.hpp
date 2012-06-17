/** \file FittingAnalyzer.hpp
 * 
 * Class to use Fit on traces
 */
#ifndef __FITTINGANALYZER_HPP_
#define __FITTINGANALYZER_HPP_

#include "TimingInformation.hpp"
#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

class FittingAnalyzer : public TraceAnalyzer,
			public TimingInformation
{
 public:
    FittingAnalyzer();
    virtual void DeclarePlots(void);
    virtual void Analyze(Trace &, const std::string &, const std::string &);
    virtual ~FittingAnalyzer() {};
 
    struct FitData{
	size_t n;
	double * y;
	double * sigma;
	double width;
	double decay;
    };
 private:
    void OutputFittedInformation(void);
    double CalculateFittedFunction(double &x);
    double CalculateReducedChiSquared(const double &dof, 
				      const double &sigmaBaseline);
    double CalculateWalk(const double &maxValue);
    void FreeMemory(void);
    unsigned int counter;
    std::vector<double> aveTrace;
    std::vector<double> fittedParameters;
    std::vector<double> fittedTrace;
};
#endif // __FITTINGANALYZER_HPP_
// David is awesome.
