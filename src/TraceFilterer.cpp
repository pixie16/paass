/** \file  TraceFilterer.cpp
 *  \brief Implements the analysis of traces using trapezoidal filters
 *
 *  This trace plots the filtered traces as well as energy and timing info
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>

#include "damm_plotids.h"
#include "RandomPool.h"
#include "Trace.h"
#include "TraceFilterer.h"

using namespace std;

const string TraceFilterer::defaultFilterFile = "filter.txt";
const int TraceFilterer::energyBins = SC;
const double TraceFilterer::energyScaleFactor = 2.547; //< multiply the energy filter sums by this to gain match to raw spectra

extern RandomPool randoms;

TraceFilterer::TraceFilterer() : 
    TracePlotter(), fastParms(5,5), 
    energyParms(100,100), thirdParms(20,10)
{
    //? this uses some legacy values for third parms, are they appropriate
    name = "Filterer";
    
    fastThreshold = fastParms.GetRiseSamples()  * 3;
    slowThreshold = thirdParms.GetRiseSamples() * 2;
}

TraceFilterer::~TraceFilterer()

{
    // do nothing
}

bool TraceFilterer::Init(const string &filterFile)
{
    const int maxTraceLength = 6400;

    TracePlotter::Init();

    fastFilter.reserve(maxTraceLength);
    energyFilter.reserve(maxTraceLength);
    thirdFilter.reserve(maxTraceLength);

    // read in the filter parameters
    ifstream in(filterFile.c_str());
    if (!in) {
	cout << "Failed to open the filter parameter file" << endl;
	cout << "  Using default values instead" << endl;
	return true;
    }

    while (!in.eof()) {
	if ( isdigit(in.peek()) ) {
	    Trace::size_type rise, gap;
	    
	    cout << "Trace analysis parameters are: " << endl;
	    in >> rise >> gap >> fastThreshold;
	    cout << "  Fast filter: " << rise << " " << gap 
		 << " " << fastThreshold << endl;
	    fastParms = TrapezoidalFilterParameters(gap, rise);
	    in >> rise >> gap;
	    cout << "  Energy filter: " << rise << " " << gap << endl;
	    energyParms = TrapezoidalFilterParameters(gap, rise);
	    cout << "  Third filter: " << rise << " " << gap 
		 << " " << slowThreshold << endl;
	    in >> rise >> gap >> slowThreshold;	    
	    thirdParms = TrapezoidalFilterParameters(gap, rise);
	    
	    // scale thresholds by the length of integration (i.e. rise time)
	    fastThreshold *= fastParms.GetRiseSamples();
	    slowThreshold *= thirdParms.GetRiseSamples();
	} else {
	    // assume this is a comment
	    in.ignore(1000, '\n');
	}
    }

    if (in.fail()) {
	cerr << "Problem reading filter parameters file" << endl;
	return true;
    }

    return true;
}


void TraceFilterer::DeclarePlots(void) const
{
    using namespace dammIds::trace;

    TracePlotter::DeclarePlots();

    DeclareHistogram2D(DD_FILTER1, traceBins, numTraces, "fast filter");
    DeclareHistogram2D(DD_FILTER2, traceBins, numTraces, "energy filter");
    DeclareHistogram2D(DD_FILTER3, traceBins, numTraces, "3rd filter");

    DeclareHistogram1D(D_ENERGY1, energyBins, "E1 from trace"); 
}

void TraceFilterer::Analyze(Trace &trace,
			    const string &type, const string &subtype)
{
    using namespace dammIds::trace;
    TracePlotter::Analyze(trace, type, subtype);

    if (level >= 5) {
	fastFilter.clear();
	energyFilter.clear();
	thirdFilter.clear();

	// determine trace filters, these are trapezoidal filters characterized
	//   by a risetime and a gaptime and a range of the filter 
	trace.TrapezoidalFilter(fastFilter, fastParms);
	trace.TrapezoidalFilter(energyFilter, energyParms);
	trace.TrapezoidalFilter(thirdFilter, thirdParms);

	Trace::size_type sample; // point at which to sample the slow filter
	// find the point at which the trace crosses the fast threshold

	Trace::iterator iThr = fastFilter.begin();
	Trace::iterator iHigh = fastFilter.end();

	time = 0;
	energy = 0;

	while (iThr < iHigh) {
	    iThr = find_if(iThr, iHigh, 
			   bind2nd(greater<Trace::value_type>(), fastThreshold));
	    if (iThr == iHigh)
		break;
	    time = iThr - fastFilter.begin();
	    // sample the slow filter in the middle of its size
	    //	    sample = time + (thirdParms.GetSize() - fastParms.GetSize()) / 2;
	    sample = time + (thirdParms.GetRiseSamples() + thirdParms.GetGapSamples()
			     - fastParms.GetRiseSamples() - fastParms.GetGapSamples() );
	    if (sample >= thirdFilter.size() ||
		thirdFilter[sample] < slowThreshold) {
		iThr++; 
		continue;
	    }
	    // sample = time + (energyParms.GetSize() - fastParms.GetSize()) / 2;
	    sample = time + (energyParms.GetRiseSamples() + energyParms.GetGapSamples()
			     - fastParms.GetRiseSamples() - fastParms.GetGapSamples() );
	    if (sample < energyFilter.size())
		energy = energyFilter[sample] + randoms.Get();
	    // scale to the integration time
	    energy /= energyParms.GetRiseSamples();
	    energy *= energyScaleFactor;

	    trace.SetValue("filterTime", (int)time);
	    trace.SetValue("filterEnergy", energy);
	    break;
	}

	// now plot some stuff
	/* quick plot function still needs scaling and handling of negative numbers
	fastFilter.Plot(DD_FILTER1, numTracesAnalyzed);
	energyFilter.Plot(DD_FILTER2, numTracesAnalyzed);
	thirdFilter.Plot(DD_FILTER3, numTracesAnalyzed);
	*/
	for (Trace::size_type i = 0; i < trace.size(); i++) {
	    if (i < fastFilter.size())
		plot(DD_FILTER1, i, numTracesAnalyzed, 
		     abs(fastFilter[i]) / fastParms.GetRiseSamples() );
	    if (i < energyFilter.size())
		plot(DD_FILTER2, i, numTracesAnalyzed, 
		     abs(energyFilter[i]) / energyParms.GetRiseSamples() );
	    if (i < thirdFilter.size())
		plot(DD_FILTER3, i, numTracesAnalyzed, 
		     abs(thirdFilter[i]) / thirdParms.GetRiseSamples() );

	}
	// calculated values at end of traces
	// plot(DD_TRACE, trace.size() + 10, numTracesAnalyzed, energy);
	// plot(DD_TRACE, trace.size() + 11, numTracesAnalyzed, time);
	plot(D_ENERGY1, energy);
    } // sufficient analysis level

    EndAnalyze(trace);
}
