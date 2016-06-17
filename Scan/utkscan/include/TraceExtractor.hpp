/** \file TraceExtractor.hpp
 *  \brief Header file for the TraceExtractor class
 *
 *  \author David Miller
 *  \date January 2011
 */
#ifndef __TRACEEXTRACTOR_HPP_
#define __TRACEEXTRACTOR_HPP_
#include <string>

#include "TraceAnalyzer.hpp"

#include "DammPlotIds.hpp"

class Trace;

//! \brief A class to extract traces from events
class TraceExtractor : public TraceAnalyzer {
public:
    /** Default Constructor */
    TraceExtractor(){};

    /** Constructor taking the type and subtype to plot
    * \param [in] aType : a type to plot the traces for
    * \param [in] aSubtype : a subtype to plot the traces for 
    * \param [in] aTag : the tag for what we want to plot */
    TraceExtractor(const std::string &aType, const std::string &aSubtype,
		   const std::string &aTag = "");

    /** Default Destructor */
    ~TraceExtractor() {};

    /** Declare the plots for the analyzer */
    virtual void DeclarePlots(void);

    /** The main analysis driver
    * \param [in] trace : the trace to analyze
    * \param [in] aType : the type being analyze
    * \param [in] aSubtype : the subtype begin analyzed 
    * \param [in] tags : the map of tags for the channel */
    virtual void Analyze(Trace &trace, const std::string &aType,
                         const std::string &aSubtype, 
			 const std::map<std::string,int> &tags);
protected:
    static const unsigned int traceBins; //!< The number of bins for the trace length
    static const unsigned int numTraces; //!< The number of traces to analyze

    std::string type; //!< the detector type
    std::string subtype; //!< The detector subtype
    std::string tag; //!< The tags for the detector
};
#endif // __TRACEEXTRACTOR_HPP_
