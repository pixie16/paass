/** \file TracePlotter.h
 *  \brief Header file for the TracePlotter class
 *   
 *  David Miller, Jan 2011
 */

#ifndef __TRACEPLOTTER_H_
#define __TRACEPLOTTER_H_

#include <string>

#include "TraceAnalyzer.h"

#include "damm_plotids.h"

class Trace;

class TracePlotter : public TraceAnalyzer {
 protected:
    static const int traceBins;
    static const int numTraces;
 public:
    TracePlotter();
    ~TracePlotter();    
    
    virtual void DeclarePlots(void);
    virtual void Analyze(Trace &trace, 
			 const std::string &type, const std::string &subtype);
};

#endif // __TRACEPLOTTER_H_
