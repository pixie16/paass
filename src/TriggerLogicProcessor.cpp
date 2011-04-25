/** \file TriggerLogicProcessor.cpp
 *
 * make a nifty trigger/logic graph
 * derived from MtcProcessor.cpp
 *
 */

#include <string>
#include <vector>

#include "damm_plotids.h"
#include "param.h"
#include "RawEvent.h"
#include "TriggerLogicProcessor.h"

using namespace std;

TriggerLogicProcessor::TriggerLogicProcessor(void) : LogicProcessor()
{
    name = "triggerlogic";
    plotSize = SA;
}

void TriggerLogicProcessor::DeclarePlots(void) const
{
    using namespace dammIds::triggerlogic;

    LogicProcessor::DeclarePlots();

    DeclareHistogram2D(DD_RUNTIME_LOGIC, plotSize, plotSize, "runtime logic (1ms/bin)");
}

bool TriggerLogicProcessor::Process(RawEvent &event)
{
    const double logicPlotResolution = 1e-3 / pixie::clockInSeconds;
 
    LogicProcessor::Process(event);

    using namespace dammIds::triggerlogic;

    static DetectorSummary *stopsSummary    = event.GetSummary("logic:stop");
    static DetectorSummary *triggersSummary = event.GetSummary("logic:trigger");

    //    static const vector<ChanEvent*> &stops     = sumMap["logic:stop"]->GetList();
    // static const vector<ChanEvent*> &triggers  = sumMap["generic:trigger"]->GetList();
    static const vector<ChanEvent*> &stops    = stopsSummary->GetList();
    static const vector<ChanEvent*> &triggers = triggersSummary->GetList();

    for (vector<ChanEvent*>::const_iterator it = stops.begin();
	 it != stops.end(); it++) {
	ChanEvent *chan = *it;

	unsigned int loc = chan->GetChanID().GetLocation();

	int timeBin      = int(chan->GetTime() / logicPlotResolution);
	int startTimeBin = 0;

	if (!isnan(lastStartTime.at(loc))) {
	  startTimeBin = int(lastStartTime.at(loc) / logicPlotResolution);
	}
	 
	for (int bin=startTimeBin; bin < timeBin; bin++) {
	  int row = bin / plotSize;
	  int col = bin % plotSize;
	  plot(DD_RUNTIME_LOGIC, col, row, loc + 1); // add one since first logic location might be 0
	}
    }
    for (vector<ChanEvent*>::const_iterator it = triggers.begin();
	 it != triggers.end(); it++) {
      int timeBin = int((*it)->GetTime() / logicPlotResolution);

      int row = timeBin / plotSize;
      int col = timeBin % plotSize;

      plot(DD_RUNTIME_LOGIC, col, row, 20);
    }

    EndProcess(); // update processing time
    return true;
}
