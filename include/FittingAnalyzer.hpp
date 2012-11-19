/** \file FittingAnalyzer.hpp
 * \brief Class to fit functions to waveforms
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
	double beta,gamma,qdc;
    };
 private:
    void LoadMask(void);
    void OutputFittedInformation(const std::vector<double> &waveform, 
				 const std::vector<double> &fitPars);
    double ApplyMask(const std::vector<double> &waveform, 
		     const double &qdc, const double &maxval, 
		     const double &sigma);
    double CalcFittedFunction(double &x, 
			      const std::vector<double> &fitPars);
    double CalcWalk(const double &maxValue, const std::string &type, 
		    const std::string &subType);
};
#endif // __FITTINGANALYZER_HPP_
// David is awesome.
