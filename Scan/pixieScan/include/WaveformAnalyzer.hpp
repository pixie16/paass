/** \file WaveformAnalyzer.hpp
 * \brief Class to analyze traces.
 */
#ifndef __WAVEFORMANALYZER_HPP_
#define __WAVEFORMANALYZER_HPP_

#include "HighResTimingData.hpp"
#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

//! Class to handle the HRT and waveform analysis
class WaveformAnalyzer : public TraceAnalyzer {
public:
    /** Default Constructor */
    WaveformAnalyzer();
    /** Default destructor */
    ~WaveformAnalyzer() {};
    /** Declare the plots */
    virtual void DeclarePlots(void) const {};
    /** Do the analysis on traces
    * \param [in] trace : the trace to analyze
    * \param [in] detType : the detector type
    * \param [in] detSubtype : detector subtype 
    * \param [in] tagMap : the map of the tags for the channel */
    virtual void Analyze(Trace &trace, const std::string &detType,
                         const std::string &detSubtype,
                         const std::map<std::string, int> & tagMap);
private:
    /** \return True if the detector type given is unknown to the waveform
     * analysis.
     * \param [in] type : The type of detector to look for */
    bool CheckIfUnknown(const std::string &type);
    std::vector<std::string> knownTypes_;//!< The types known for waveform analysis
};
#endif // __WAVEFORMANALYZER_HPP_
