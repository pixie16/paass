/** \file TraceFilterer.hpp
 * \brief Declaration of analysis class which does the filtering of traces.
 * \author David Miller
 * \date January 2011
 */

#ifndef __TRACEFILTERER_HPP_
#define __TRACEFILTERER_HPP_

#include <string>

#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

class TraceFilterer : public TraceAnalyzer {
    public:
        TraceFilterer(double energyScaleFactor_,
                    short fast_rise, short fast_gap, short fast_threshold,
                    short energy_rise, short energy_gap,
                    short slow_rise, short slow_gap, short slow_threshold);
        virtual ~TraceFilterer();

        virtual bool Init(const std::string &filterFileName = "filter.txt");
        virtual void DeclarePlots(void);
        virtual void Analyze(Trace &trace, 
                const std::string &type, const std::string &subtype);
    protected:    
        static const int energyBins;
        double energyScaleFactor_;

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
            Trace::size_type time;
            double energy;
            bool isFound;

            PulseInfo(void);
            PulseInfo(Trace::size_type theTime, double theEnergy);
        };

        PulseInfo pulse;

        virtual const PulseInfo& FindPulse(Trace::iterator begin, 
                                        Trace::iterator end);
};

#endif // __TRACEFILTERER_HPP_
