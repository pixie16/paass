/** \file TracePlotter.hpp
 *  \brief Header file for the TracePlotter class
 *   
 *  David Miller, Jan 2011
 */

#ifndef __TRACEEXTRACTER_HPP_
#define __TRACEEXTRACTER_HPP_

#include <string>

#include "TraceAnalyzer.hpp"

#include "DammPlotIds.hpp"

class Trace;

class TraceExtracter : public TraceAnalyzer {
 protected:
    static const int traceBins;
    static const int numTraces;

    std::string type;
    std::string subtype;
 public:
    TraceExtracter(const std::string &aType, const std::string &aSubtype);
    ~TraceExtracter();    
    
    virtual void DeclarePlots(void);
    virtual void Analyze(Trace &trace, 
			 const std::string &type, const std::string &subtype);
};

#endif // __TRACEEXTRACTER_HPP_
