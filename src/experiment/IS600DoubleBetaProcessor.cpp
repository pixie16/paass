/** \file IS600DoubleBetaProcessor.cpp
 *\brief A DoubleBeta processor class that can be used to analyze double
 * beta detectors.
 *\author S. V. Paulauskas
 *\date October 26, 2014
 */
#include "BarBuilder.hpp"
#include "DammPlotIds.hpp"
#include "IS600DoubleBetaProcessor.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "TimingMapBuilder.hpp"
#include "TreeCorrelator.hpp"

namespace dammIds {
    namespace doublebeta {
      const int DD_PROTONBETA2TDIFF_VS_BETA2EN = 5;
      const int DD_DEBUGGING6 = 6;
    }
}

using namespace std;
using namespace dammIds::doublebeta;

IS600DoubleBetaProcessor::IS600DoubleBetaProcessor():
  DoubleBetaProcessor() {
    associatedTypes.insert("beta");
}

void IS600DoubleBetaProcessor::DeclarePlots(void) {
    DoubleBetaProcessor::DeclarePlots();
    DeclareHistogram2D(DD_PROTONBETA2TDIFF_VS_BETA2EN, SD, SA, "BetaProton Tdiff vs. Beta Energy");
}

bool IS600DoubleBetaProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return(false);
    if(!DoubleBetaProcessor::PreProcess(event))
	return(false);
    return(true);
}

bool IS600DoubleBetaProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);
    
    static const vector<ChanEvent*> & events =
        event.GetSummary("beta:double")->GetList();
    
    BarBuilder builder(events);
    builder.BuildBars();
    map<unsigned int, pair<double,double> > lrtbars = builder.GetLrtBarMap();
    BarMap betas = builder.GetBarMap();
    double lastProtonTime =  TreeCorrelator::get()->place("logic_t1_0")->last().time;
    
    for(map<unsigned int, pair<double,double> >::iterator it = lrtbars.begin(); 
	it != lrtbars.end(); it++)
	plot(DD_PROTONBETA2TDIFF_VS_BETA2EN, it->second.second, 
	     (it->second.first - lastProtonTime) / 
	     (10e-3/Globals::get()->clockInSeconds()) );
    
    EndProcess();
    return(true);
}
