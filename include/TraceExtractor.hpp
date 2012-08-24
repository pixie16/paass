/** \file TraceExtractor.hpp
 *  \brief Header file for the TraceExtractor class
 *  \author David Miller
 *  \date Jan 2011
 */

#ifndef __TRACEEXTRACTOR_HPP_
#define __TRACEEXTRACTOR_HPP_

#include <string>

#include "TraceAnalyzer.hpp"

#include "DammPlotIds.hpp"

class Trace;

class TraceExtractor : public TraceAnalyzer {
 protected:
    static const int traceBins;
    static const int numTraces;

    std::string type;
    std::string subtype;
 public:
    TraceExtractor(const std::string &aType, const std::string &aSubtype);
    ~TraceExtractor();    
    
    virtual void DeclarePlots(void);
    virtual void Analyze(Trace &trace, 
			 const std::string &type, const std::string &subtype);
};

#endif // __TRACEEXTRACTOR_HPP_
