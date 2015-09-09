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
    DeclareHistogram1D(DD_DEBUGGING6, SE, "Vandle Multiplicity");
}

bool IS600DoubleBetaProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return(false);

    if(!DoubleBetaProcessor::PreProcess(event))
      return(false);
}

bool IS600DoubleBetaProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);

    double clockInSeconds = Globals::get()->clockInSeconds();
    // plot with 10 ms bins
    const double plotResolution = 10e-3 / clockInSeconds;

    static const vector<ChanEvent*> & events =
        event.GetSummary("beta:double")->GetList();

    double lastProtonTime =  TreeCorrelator::get()->place("mtc_t1_0")->last().time;

    double energy2 = 0., energy3 = 0., time2 = 0., time3 = 0.;

    for(vector<ChanEvent*>::const_iterator it = events.begin();
	it != events.end(); it++) {
      unsigned int loc = (*it)->GetChanID().GetLocation();
      if(loc == 0 || loc == 1)
	continue;
      else if (loc == 2) {
	energy2 = (*it)->GetEnergy();
	time2 = (*it)->GetTime();
      } else if (loc == 3) {
	energy3 = (*it)->GetEnergy();
      }
    }

    plot(DD_PROTONBETA2TDIFF_VS_BETA2EN, energy2, (time2 - lastProtonTime) / plotResolution);

    static const vector<ChanEvent*> &labr3Evts =
      event.GetSummary("generic:labr3")->GetList();
      
      for(vector<ChanEvent*>::const_iterator it = labr3Evts.begin();
	  it != labr3Evts.end(); it++)
	plot(DD_DEBUGGING6, (*it)->GetEnergy());

    return(true);
}
