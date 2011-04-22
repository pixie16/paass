/**    \file DoubleTraceAnalyzer.cpp
 *     \brief Identifies double traces.
 *
 *     Implements a quick online trapezoidal filtering mechanism
 *     for the identification of double pulses
 *
 *     - SNL - 7-2-07 - created
 *     - SNL - 2-4-08 - Add plotting spectra
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>

#include <cstdlib>

#include "damm_plotids.h"
#include "RandomPool.h"
#include "Trace.h"
#include "DoubleTraceAnalyzer.h"

using namespace std;

// external pool of random numbers defined in RandomPool.cpp
extern RandomPool randoms;

/**
 * Set default values for time and energy
 */
DoubleTraceAnalyzer::DoubleTraceAnalyzer() : 
    TraceFilterer()
{
    time2 = 0;
    energy2 = 0.;
    numDoubleTraces = 0;
}


DoubleTraceAnalyzer::~DoubleTraceAnalyzer() 
{
    // do nothing
}

void DoubleTraceAnalyzer::DeclarePlots() const
{
    using namespace dammIds::trace;

//    TraceAnalyzer::DeclarePlots();
    TraceFilterer::DeclarePlots();

    const int energyBins2 = SA;
    const int timeBins = SA;

    DeclareHistogram1D(D_ENERGY2, energyBins, "E2 from traces");

    DeclareHistogram2D(DD_DOUBLE_TRACE, traceBins, numTraces, "double traces");
    DeclareHistogram2D(DD_ENERGY2__TDIFF, energyBins2, timeBins, "E2 vs DT", 2);
    DeclareHistogram2D(DD_ENERGY2__ENERGY1, energyBins2, energyBins2, "E2 vs E1", 2);
}

/**
 *   Detect a second crossing of the fast filter corresponding to a piled-up
 *     trace and deduce its energy
 */
void DoubleTraceAnalyzer::Analyze(Trace &trace, 
				  const string &type, const string &subtype)
{
    TraceFilterer::Analyze(trace, type, subtype);
    
    if ( trace.HasValue("filterTime") && level >= 10 ) {
	// trace filterer found a first pulse
	Trace::iterator iThr = fastFilter.begin() + time;
	Trace::iterator iHigh = fastFilter.end();
	
	// points at which to sample the slow filter	
	Trace::size_type sample, sample2; 
	
	// find the trailing edge
	advance(iThr, fastParms.GetGapSamples());
	iThr = find_if(iThr, iHigh, bind2nd(less<Trace::value_type>(), 
					    fastThreshold));
	// advance(iThr, fastParms.GetSize());
     	advance(iThr, fastParms.GetRiseSamples());

	// find a second crossing point
	time2 = 0;
	energy2 = 0.;
  
	while (iThr < iHigh) {
	    iThr = find_if(iThr, iHigh, 
			   bind2nd(greater<Trace::value_type>(), 
				   fastThreshold));
	    if (iThr == iHigh)
		break;

	    time2 = iThr - fastFilter.begin();
	    // sample = time2 + (thirdParms.GetSize() - fastParms.GetSize()) / 2;
	    sample = time2 + (thirdParms.GetRiseSamples() + thirdParms.GetGapSamples()
			     - fastParms.GetRiseSamples() - fastParms.GetGapSamples() );
	    sample2 = time2 - fastParms.GetSize();
	    
	    if (sample >= thirdFilter.size() || 
		thirdFilter[sample] - thirdFilter[sample2] < slowThreshold) {
		iThr++; continue;
	    }
	    sample = time2 + (energyParms.GetRiseSamples() + energyParms.GetGapSamples()
			     - fastParms.GetRiseSamples() - fastParms.GetGapSamples() );

	    energy2 = energyFilter[sample] - energyFilter[sample2];
	    energy2 += randoms.Get();
	    // scale to the integration time
	    energy2 /= energyParms.GetRiseSamples();
	    
	    trace.SetValue("filterTime2", (int)time2);
	    trace.SetValue("filterEnergy2", energy2);
	    break;
	} // while searching for double trace

	// now plot stuff
	if (iThr != iHigh) {
	    using namespace dammIds::trace;

	    // plot the double pulse stuff
	    for (Trace::size_type i=0; i < trace.size(); i++) {
		plot(DD_DOUBLE_TRACE, i, numDoubleTraces, trace[i]);
	    }
	    // cacluated values at end of traces
	    // plot(DD_DOUBLE_TRACE, trace.size() + 10, numDoubleTraces, energy)
	    // plot(DD_DOUBLE_TRACE, trace.size() + 11, numDoubleTraces, time)
	    // plot(DD_DOUBLE_TRACE, trace.size() + 12, numDoubleTraces, energy2);
	    // plot(DD_DOUBLE_TRACE, trace.size() + 13, numDoubleTraces, time2)	    
	    plot(D_ENERGY2, energy2);
	    plot(DD_ENERGY2__TDIFF, energy2, time2 - time);
	    plot(DD_ENERGY2__ENERGY1, energy2, energy);

	    numDoubleTraces++;
	} // if found double trace
    } // sufficient analysis level

    EndAnalyze(trace);
}
