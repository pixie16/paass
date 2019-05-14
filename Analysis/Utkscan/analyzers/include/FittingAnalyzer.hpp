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

#include "set"
#include <string>

#include "TimingDriver.hpp"
#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

//! Analyzer that handles the fitting of traces for High Resolution Timing
class FittingAnalyzer : public TraceAnalyzer {
public:
    ///Default Constructor
    FittingAnalyzer(const std::string &s, const std::set<std::string> &tokens);

    /** Default Destructor */
    ~FittingAnalyzer();

    /** Analyzes the traces
     * \param [in] trace : the trace to analyze
     * \param [in] detType : the detector type we have
     * \param [in] detSubtype : the subtype of the detector 
     * \param [in] tagMap : the map of tags for the channel */
    void Analyze(Trace &trace, const ChannelConfiguration &cfg);

private:
    TimingDriver *driver_;
    std::set<std::string> ignoredTypes_;
};

#endif // __FITTINGANALYZER_HPP_
// David is awesome.
