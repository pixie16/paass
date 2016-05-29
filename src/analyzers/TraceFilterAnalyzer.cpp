/** \file  TraceFilterAnalyzer.cpp
 *  \brief Implements the analysis of traces using trapezoidal filters
 * \author S. V. Paulauskas, original D. Miller
 * \date January 2011
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RandomPool.hpp"
#include "TraceFilter.hpp"
#include "TraceFilterAnalyzer.hpp"

using namespace std;
using namespace dammIds::trace::tracefilteranalyzer;

TraceFilterAnalyzer::TraceFilterAnalyzer(const TrapFilterParameters &t,
                                         const TrapFilterParameters &e) :
    TraceAnalyzer() {
    trigPars_ = t;
    enPars_ = e;
    name = "TraceFilterAnalyzer";
}

void TraceFilterAnalyzer::DeclarePlots(void) {
    //! Declare plots within the trace object
    Trace sample_trace = Trace();

    const int traceBins = dammIds::trace::traceBins;
    const unsigned short numTraces = Globals::get()->numTraces();

    sample_trace.DeclareHistogram2D(DD_TRIGGER_FILTER, traceBins, numTraces,
                                    "Trigger Filter");
    // sample_trace.DeclareHistogram2D(DD_ENERGY_FILTER, traceBins, numTraces,
    //                                 "Energy Filter");
    sample_trace.DeclareHistogram2D(DD_REJECTED_TRACE, traceBins, numTraces,
                                    "Rejected Traces");
}

void TraceFilterAnalyzer::Analyze(Trace &trace, const std::string &type,
                                  const std::string &subtype,
                                  const std::map<std::string,int> &tagmap) {
    TraceAnalyzer::Analyze(trace, type, subtype, tagmap);
    Globals *globs = Globals::get();
    static int numTrigFilters = 0;
    static int rejectedTraces = 0;
    static unsigned short numTraces = globs->numTraces();
    pair<TrapFilterParameters, TrapFilterParameters> pars =
	globs->trapFiltPars(type+":"+subtype);

    //Want to put filter clock units of ns/Sample
    TraceFilter filter(Globals::get()->filterClockInSeconds()*1e9,
		       pars.first,pars.second);
    filter.CalcFilters(&trace);
    trace.SetValue("triggerPosition", (int)filter.GetTriggerPosition());

    if (filter.GetTriggerPosition() != 0) {
	vector<double> tfilt = filter.GetTriggerFilter();
	trace.SetValue("baseline", filter.GetBaseline());
	trace.SetValue("filterEnergy", filter.GetEnergy());
	trace.SetTriggerFilter(tfilt);
	trace.SetEnergySums(filter.GetEnergySums());
	
	//500 is an arbitrary offset since DAMM cannot display negative numbers.
	for(vector<double>::iterator it = tfilt.begin(); it != tfilt.end(); it++)
	    trace.plot(DD_TRIGGER_FILTER, (int)(it-tfilt.begin()),
	     	       numTrigFilters, (*it)+500);
	numTrigFilters++;
    } else {
        if (rejectedTraces < numTraces)
            trace.Plot(DD_REJECTED_TRACE, rejectedTraces++);
    }
    EndAnalyze(trace);
}
