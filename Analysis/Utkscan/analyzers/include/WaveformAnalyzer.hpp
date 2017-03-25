/** \file WaveformAnalyzer.hpp
 * \brief Class to analyze traces.
 * \author S. V. Paulauskas
 */
#ifndef __WAVEFORMANALYZER_HPP_
#define __WAVEFORMANALYZER_HPP_

#include "Globals.hpp"
#include "Messenger.hpp"
#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

//! Class to waveform analysis
class WaveformAnalyzer : public TraceAnalyzer {
public:
    /** Default Constructor */
    WaveformAnalyzer();

    /** Default destructor */
    ~WaveformAnalyzer() { delete messenger_; }

    /** Declare the plots */
    void DeclarePlots(void) const {}

    /** Do the analysis on traces
    * \param [in] trace : the trace to analyze
    * \param [in] type : the detector type
    * \param [in] subtype : detector subtype 
    * \param [in] tags : the map of the tags for the channel */
    void Analyze(Trace &trace, const std::string &type,
                 const std::string &subtype,
                 const std::map<std::string, int> &tags);

private:
    Messenger *messenger_;//!< A pointer for the messenger class
    std::string TDdet_; //!<string of type:subtype for the tracedelay function
};

#endif // __WAVEFORMANALYZER_HPP_
