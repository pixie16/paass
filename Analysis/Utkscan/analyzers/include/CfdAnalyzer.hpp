/** \file CfdAnalyzer.hpp
 * \brief Class to use Cfd on traces
 * \author S. V. Paulauskas
 * \date 22 July 2011
 */
#ifndef __CFDANALYZER_HPP_
#define __CFDANALYZER_HPP_

#include "TimingDriver.hpp"
#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

//! Class to analyze traces using a digital CFD
class CfdAnalyzer : public TraceAnalyzer {
   public:
    /** Default constructor taking an argument*/
    CfdAnalyzer(const std::string &s, const std::set<std::string> &ignoredTypes);

    /** Default Destructor */
    ~CfdAnalyzer(){};

    /** Declare the plots */
    void DeclarePlots(void) const {};

    /** Do the analysis on traces
    * \param [in] trace : the trace to analyze
    * \param [in] detType : the detector type
    * \param [in] detSubtype : detector subtype 
    * \param [in] tagMap : the map of tags for the channel */
    void Analyze(Trace &trace, const ChannelConfiguration &cfg);

   private:
    std::set<std::string> ignoredTypes_;
    TimingDriver *driver_;
};

#endif
