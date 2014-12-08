/** \file TraceFilterer.hpp
 * \brief Declaration of analysis class which does the filtering of traces.
 *
 *  I (SVP) Will be overhauling this class with a new implementation of the
 *  filters at some point.
 * \author David Miller
 * \date January 2011
 */

#ifndef __TRACEFILTERER_HPP_
#define __TRACEFILTERER_HPP_

#include <string>

#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

//! \brief A class to perform trapezoidal filters on the traces
class TraceFilterer : public TraceAnalyzer {
public:
    TraceFilterer(){};

    /** Constructor taking all of the necessary parameters
     * \param [in] energyScaleFactor_ : the scaling to match filtered energy to pixie
     * \param [in] fast_rise : the rise time of the fast filter
     * \param [in] fast_gap : the flattop of the fast filter
     * \param [in] fast_threshold : the threshold of the fast filter
     * \param [in] energy_rise : the risetime of the energy filter
     * \param [in] energy_gap : The flattop of the energy filter
     * \param [in] slow_rise : The rise time of the second energy filter
     * \param [in] slow_gap : the flattop of the second energy filter
     * \param [in] slow_threshold : the threshold for the second energy filter */
    TraceFilterer(double energyScaleFactor_,
                  short fast_rise, short fast_gap, short fast_threshold,
                  short energy_rise, short energy_gap,
                  short slow_rise, short slow_gap, short slow_threshold);

    /** Default Destructor */
    virtual ~TraceFilterer(){};

    /** Initialize the analyzer
    * \param [in] filterFileName : the name of the file containing the filter parameters
    * \returns True if the initialization was successful. */
    virtual bool Init(const std::string &filterFileName = "Config.xml");

    /** Declare the plots for the Analyzer */
    virtual void DeclarePlots(void);

    /** The analyzer method to do the analysis
     * \param [in] trace : the trace to analyze
     * \param [in] type : the detector type
     * \param [in] subtype : the detector subtype */
    virtual void Analyze(Trace &trace,
                         const std::string &type, const std::string &subtype);
protected:
    static const int energyBins; //!< Number of energy bins
    double energyScaleFactor_; //!< energy scaling factor

    TrapezoidalFilterParameters  fastParms; //!< Fast (Trigger) filter parameters
    Trace::value_type            fastThreshold; //!< Threshold for fast filter
    TrapezoidalFilterParameters  energyParms; //!< Slow (Energy) filter parameters
    TrapezoidalFilterParameters  thirdParms; //!< Parameters for second fast filter
    Trace::value_type            slowThreshold; //!< Threshold for fast filter

    Trace fastFilter;   ///< fast filter of trace
    Trace energyFilter; ///< slow filter of trace
    Trace thirdFilter;  ///< second slow filter of trace

    bool useThirdFilter; //!< True if we want to use the third filter

    //! Structure to hold information about the pulse
    struct PulseInfo {
        Trace::size_type time; //!< the time of the pulse
        double energy; //!< energy of the pulse
        bool isFound; //!< true if a pulse is found

        /** Constructor for the pulse info */
        PulseInfo(void){isFound = false;};
        /** Constructor taking default arguments
        * \param [in] theTime : time of the pulse
        * \param [in] theEnergy : the energy of the pulse */
        PulseInfo(Trace::size_type theTime, double theEnergy){isFound = true;};
    };

    PulseInfo pulse; //!< Instance of the PulseInfo class

    /** Find the pulse in the trace
     * \param [in] begin : an iterator to the beginning of the trace
     * \param [in] end : an iterator to the end of the trace
     * \return A PulseInfo structure containing the calculated info */
    virtual const PulseInfo& FindPulse(Trace::iterator begin,
                                       Trace::iterator end);
};
#endif // __TRACEFILTERER_HPP_
