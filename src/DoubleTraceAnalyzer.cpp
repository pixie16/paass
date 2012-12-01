/** \file DoubleTraceAnalyzer.cpp
 * \brief Identifies double traces.
 *
 * Implements a quick online trapezoidal filtering mechanism
 * for the identification of double pulses
 *
 * \author S. Liddick
 * \date 02 July 2007
 * 
 * <STRONG>Modified : </STRONG> SNL - 2-4-08 - Add plotting spectra
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <numeric>

#include <cstdlib>

#include "DammPlotIds.hpp"
#include "RandomPool.hpp"
#include "Trace.hpp"
#include "DoubleTraceAnalyzer.hpp"

using namespace std;
using namespace dammIds::trace;

/**
 * Set default values for time and energy
 */
DoubleTraceAnalyzer::DoubleTraceAnalyzer() : 
    TraceFilterer(doubletrace::OFFSET, doubletrace::RANGE)
{
    time2 = 0;
    energy2 = 0.;
    numDoubleTraces = 0;
}


DoubleTraceAnalyzer::~DoubleTraceAnalyzer() 
{
    // do nothing
}

void DoubleTraceAnalyzer::DeclarePlots()
{
    using namespace dammIds::trace;

//    TraceAnalyzer::DeclarePlots();
    TraceFilterer::DeclarePlots();

    const int energyBins2 = SA;
    const int timeBins = SA;

    DeclareHistogram1D(D_ENERGY2, energyBins, "E2 from traces");

    DeclareHistogram2D(DD_DOUBLE_TRACE, traceBins, numTraces, "double traces");
    histo.DeclareHistogram2D(DD_ENERGY2__TDIFF, energyBins2, timeBins, "E2 vs DT", 2);
    histo.DeclareHistogram2D(DD_ENERGY2__ENERGY1, energyBins2, energyBins2, "E2 vs E1", 2);

    DeclareHistogram2D(DD_TRIPLE_TRACE, traceBins, numTraces, "interesting traces");
    DeclareHistogram2D(DD_TRIPLE_TRACE_FILTER1, traceBins, numTraces,
		       "interesting traces (fast filter)");
    DeclareHistogram2D(DD_TRIPLE_TRACE_FILTER2, traceBins, numTraces,
		       "interesting traces (energy filter)");
    DeclareHistogram2D(DD_TRIPLE_TRACE_FILTER3, traceBins, numTraces,
		       "interesting traces (3rd filter)");
}

/**
 *   Detect a second crossing of the fast filter corresponding to a piled-up
 *     trace and deduce its energy
 */
void DoubleTraceAnalyzer::Analyze(Trace &trace, 
				  const string &type, const string &subtype)
{    
    if (subtype == "top" || subtype == "bottom")
	return;

    TraceFilterer::Analyze(trace, type, subtype);
    // class to see when the fast filter falls below threshold
    static binder2nd< less<Trace::value_type> > recrossesThreshold
	(less<Trace::value_type>(), fastThreshold);

    if ( pulse.isFound && level >= 10 ) {
	// cout << "Double trace #" << numTracesAnalyzed << " for type " << type << ":" << subtype << endl;
	// trace filterer found a first pulse

	Trace::iterator iThr = fastFilter.begin() + pulse.time;
	Trace::iterator iHigh = fastFilter.end();

	vector<PulseInfo> pulseVec;
	// put the original pulse in the vector
	pulseVec.push_back(pulse);
	const size_t pulseLimit = 50; // maximum number of pulses to find

	while (iThr < iHigh) {
	    // find the trailing edge (use rise samples?)
	    advance(iThr, fastParms.GetGapSamples());
	    iThr = find_if(iThr, iHigh, recrossesThreshold);					
	    // advance(iThr, fastParms.GetSize());
	    advance(iThr, fastParms.GetRiseSamples());

	    FindPulse(iThr, iHigh);
	    if (pulse.isFound) {
		pulseVec.push_back(pulse);
		iThr = fastFilter.begin() + pulse.time;
	    } else break;
	    if (pulseVec.size() > pulseLimit) {
		cout << "Too many pulses, limit = " << pulseLimit << ", breaking out." << endl;
		EndAnalyze(); // update timing
		return;
	    }
	} // while searching for multiple traces
	
	trace.SetValue("numPulses", (int)pulseVec.size());

	// now plot stuff
	if ( pulseVec.size() > 1 ) {
	    using namespace dammIds::trace;

	    // fill the trace info
	    // first pulse info is set in TraceFilterer
	    for (Trace::size_type i=1; i < pulseVec.size(); i++) {
		stringstream str;
		// the first pulse in the vector is the SECOND pulse in the trace
		str << "filterEnergy" << i+1;
		trace.SetValue(str.str(), pulseVec[i].energy);
		str.str(""); // clear the string
		str << "filterTime" << i+1;
		trace.SetValue(str.str(), (int)pulseVec[i].time);
	    }
	    
	    // plot the double pulse stuff
	    trace.Plot(DD_DOUBLE_TRACE, numDoubleTraces);
	    if (pulseVec.size() > 2) {
		static int numTripleTraces = 0;
		cout << "Found triple trace " << numTripleTraces 
		     << ", num pulses = " << pulseVec.size()
		     << ", sigma baseline = " << trace.GetValue("sigmaBaseline") << endl;
		trace.Plot(DD_TRIPLE_TRACE, numTripleTraces);
		fastFilter.ScalePlot(DD_TRIPLE_TRACE_FILTER1, numTripleTraces, fastParms.GetRiseSamples());
		energyFilter.ScalePlot(DD_TRIPLE_TRACE_FILTER2, numTripleTraces, energyParms.GetRiseSamples());
		if (useThirdFilter)
		    thirdFilter.ScalePlot(DD_TRIPLE_TRACE_FILTER3, numTripleTraces, thirdParms.GetRiseSamples());
		numTripleTraces++;
	    }

	    plot(D_ENERGY2, pulseVec[1].energy);
	    plot(DD_ENERGY2__TDIFF, pulseVec[1].energy, pulseVec[1].time - pulseVec[0].time);
	    plot(DD_ENERGY2__ENERGY1, pulseVec[1].energy, pulseVec[0].energy);

	    numDoubleTraces++;
	} // if found double trace
    } // sufficient analysis level

    EndAnalyze(trace);
}
