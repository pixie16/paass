/** \file WaveformAnalyzer.hpp
 * \brief Class to analyze traces.
 * \author S. V. Paulauskas
 */
#ifndef __WAVEFORMANALYZER_HPP_
#define __WAVEFORMANALYZER_HPP_

#include "Globals.hpp"
#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

//! Class to waveform analysis
class WaveformAnalyzer : public TraceAnalyzer {
public:
    /** Default Constructor */
    WaveformAnalyzer();
    /** Default destructor */
    ~WaveformAnalyzer() {}
    /** Declare the plots */
    virtual void DeclarePlots(void) const {}
    /** Do the analysis on traces
    * \param [in] trace : the trace to analyze
    * \param [in] type : the detector type
    * \param [in] subtype : detector subtype 
    * \param [in] tags : the map of the tags for the channel */
    virtual void Analyze(Trace &trace, const std::string &type,
                         const std::string &subtype,
                         const std::map<std::string, int> & tags);
private:
    double mean_; //!< The mean of the baseline
    unsigned int mval_; //!< the maximum value in the trace
    std::pair<Trace::iterator,Trace::iterator> wrng_; //!< the waveform range
    Trace::iterator bhi_; //!< high value for baseline calculation
    Trace *trc_; //!< A pointer to the trace for the class
    Globals *g_; //!< A pointer to the globals class for the class
    
    /** Performs the baseline calculation
    * \param [in] lo : the low range for the baseline calculation
    * \param [in] numBins : The number of bins for the baseline calculation
    * \return The average value of the baseline in the region */
    void CalculateSums();

    /** Performs the neutron-gamma discrimination on the traces
    * \param [in] lo : The low range for the discrimination (referenced from max)
    * \param [in] numBins : the number of bins to calculate the baseline over
    * \return The discrimination value */
    void CalculateDiscrimination(const unsigned int &lo);

    /** Calculate information for the maximum value of the trace
    * \param [in] lo : the low side of the waveform
    * \param [in] hi : the high side of the waveform
    * \param [in] numBins : the number of bins to look for the max in
    * \return The position of the maximum value in the trace */
    bool FindWaveform(const unsigned int &lo, const unsigned int &hi);
};
#endif // __WAVEFORMANALYZER_HPP_
