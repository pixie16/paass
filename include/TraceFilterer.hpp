/** \file TraceFilterer.hpp
 * \brief Declaration of analysis class which does the filtering of traces.
 * \author David Miller
 * \date January 2011
 */

#ifndef __TRACEFILTERER_HPP_
#define __TRACEFILTERER_HPP_

#include <string>

#include "Trace.hpp"
#include "TracePlotter.hpp"

class TraceFilterer : public TracePlotter {
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
    
    bool useThirdFilter;

    struct PulseInfo {
    public:
	Trace::size_type time;
	double energy;
	bool isFound;

	PulseInfo(void);
	PulseInfo(Trace::size_type theTime, double theEnergy);
    };
    PulseInfo pulse;

    virtual const PulseInfo& FindPulse(Trace::iterator begin, Trace::iterator end);
 public:
    TraceFilterer();
    TraceFilterer(int offset, int range);
    virtual ~TraceFilterer();

    virtual bool Init(const std::string &filterFileName = "filter.txt");
    virtual void DeclarePlots(void);
    virtual void Analyze(Trace &trace, 
			 const std::string &type, const std::string &subtype);
};

#endif // __TRACEFILTERER_HPP_
