/** \file TraceFilterer.h
 *  \brief Declaration of analysis class which does the filtering of traces.
 *
 *  David Miller, January 2011
 */

#ifndef __TRACEFILTERER_H_
#define __TRACEFILTERER_H_

#include <string>

#include "Trace.h"
#include "TracePlotter.h"

class Trace;

class TraceFilterer : public TracePlotter {
 private:
    static const std::string defaultFilterFile;

 protected:    
    static const int energyBins;
    static const double energyScaleFactor;

    TrapezoidalFilterParameters  fastParms;
    Trace::value_type            fastThreshold;
    TrapezoidalFilterParameters  energyParms;
    TrapezoidalFilterParameters  thirdParms;
    Trace::value_type            slowThreshold;

    Trace fastFilter;   ///< fast filter of trace
    Trace energyFilter; ///< slow filter of trace
    Trace thirdFilter;  ///< second slow filter of trace
    
    Trace::size_type time;
    double energy;
 public:
    TraceFilterer();
    virtual ~TraceFilterer();

    virtual bool Init(const std::string &filterFile = defaultFilterFile);
    virtual void DeclarePlots(void) const;
    virtual void Analyze(Trace &trace, 
			 const std::string &type, const std::string &subtype);
};

#endif // __TRACEFILTERER_H_
