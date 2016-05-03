/** \file FittingAnalyzer.hpp
 * \brief Class to fit functions to waveforms
 *
 * Obtains the phase of a waveform using a Chi^2 fitting algorithm
 * implemented through the GSL libraries.
 *
 * \author S. V. Paulauskas
 * \date 22 July 2011
 */
#ifndef __FITTINGANALYZER_HPP_
#define __FITTINGANALYZER_HPP_

#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

//! Analyzer that handles the fitting of traces for High Resolution Timing
class FittingAnalyzer : public TraceAnalyzer {
public:
    /** Default Constructor */
    FittingAnalyzer();
    /** Default Destructor */
    ~FittingAnalyzer() {};
    /** Declare plots for the analyzer */
    virtual void DeclarePlots(void);
    /** Analyzes the traces
     * \param [in] trace : the trace to analyze
     * \param [in] detType : the detector type we have
     * \param [in] detSubtype : the subtype of the detector */
    virtual void Analyze(Trace &trace, const std::string &detType,
                         const std::string &detSubtype,
                         const std::map<std::string, int> & tagMap);
    //! Structure necessary for the GSL fitting routines
    struct FitData {
        size_t n;//!< size of the fitting parameters
        double * y;//!< ydata to fit
        double * sigma;//!< weight of the points
        double beta; //!< the beta parameter for the fit
        double gamma; //!< the gamma parameter for the fit
        double qdc;//!< the QDC for the fit
    };
};
#endif // __FITTINGANALYZER_HPP_
// David is awesome.
