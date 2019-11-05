/** \file WaveformAnalyzer.hpp
 * \brief Class to analyze traces.
 * \author S. V. Paulauskas
 */
#ifndef __WAVEFORMANALYZER_HPP_
#define __WAVEFORMANALYZER_HPP_

#include <set>
#include <string>

#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

//! Class to waveform analysis
class WaveformAnalyzer : public TraceAnalyzer {
public:
    /** Default Constructor */
    WaveformAnalyzer(const std::set<std::string> &ignoredTypes);

    /** Default destructor */
    ~WaveformAnalyzer() {}

    /** Declare the plots */
    void DeclarePlots(void) const {}

    /** Do the analysis on traces
    * \param [in] trace : the trace to analyze
    * \param [in] type : the detector type
    * \param [in] subtype : detector subtype 
    * \param [in] tags : the map of the tags for the channel */
    void Analyze(Trace &trace, const ChannelConfiguration &cfg);

     //precheck of the individual analyzer's ignore list
    bool IsIgnoredDetector(const ChannelConfiguration &id);

private:
    std::set<std::string> ignoredTypes_;
    int extremeBaselineRejectCounter_;
};

#endif // __WAVEFORMANALYZER_HPP_
