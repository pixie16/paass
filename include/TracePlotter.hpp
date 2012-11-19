/** \file TracePlotter.hpp
 * \brief Header file for the TracePlotter class
 * \author David Miller
 * \date January 2011
 */

#ifndef __TRACEPLOTTER_HPP_
#define __TRACEPLOTTER_HPP_

#include <string>

#include "TraceAnalyzer.hpp"
#include "DammPlotIds.hpp"

class Trace;

class TracePlotter : public TraceAnalyzer {
 protected:
    static const int traceBins;
    static const int numTraces;
 public:
    TracePlotter();
    TracePlotter(int offset, int range);
    ~TracePlotter();    
    
    virtual void DeclarePlots(void);
    virtual void Analyze(Trace &trace, 
			 const std::string &type, const std::string &subtype);
};

#endif // __TRACEPLOTTER_HPP_
