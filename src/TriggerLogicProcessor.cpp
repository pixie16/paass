/** \file TriggerLogicProcessor.cpp
 *
 * make a nifty trigger/logic graph
 * derived from MtcProcessor.cpp
 *
 */
#include <string>
#include <vector>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "TriggerLogicProcessor.hpp"

using namespace std;
using namespace dammIds::logic;
using namespace dammIds::triggerlogic;

namespace dammIds {
    namespace triggerlogic {
        const int DD_RUNTIME_LOGIC = 80;//!< Plot of run time logic
    }
}

TriggerLogicProcessor::TriggerLogicProcessor(void) :
    LogicProcessor(dammIds::triggerlogic::OFFSET, dammIds::triggerlogic::RANGE) {
    name = "triggerlogic";
    plotSize = SA;
}

void TriggerLogicProcessor::DeclarePlots(void) {

    LogicProcessor::DeclarePlots();

    DeclareHistogram2D(DD_RUNTIME_LOGIC, plotSize, plotSize,
                       "runtime logic [1ms]");
    for(int i=1; i < MAX_LOGIC; i++)
        DeclareHistogram2D(DD_RUNTIME_LOGIC+i, plotSize,
                           plotSize, "runtime logic [1ms]");

}

bool TriggerLogicProcessor::Process(RawEvent &event) {
    const double logicPlotResolution = 1e-3 / Globals::get()->clockInSeconds();
    const long maxBin = plotSize * plotSize;

    LogicProcessor::Process(event);

    using namespace dammIds::triggerlogic;
    using namespace dammIds::logic;

    static DetectorSummary *stopsSummary    = event.GetSummary("logic:stop");
    static DetectorSummary *triggersSummary = event.GetSummary("logic:trigger");

    static const vector<ChanEvent*> &stops    = stopsSummary->GetList();
    static const vector<ChanEvent*> &triggers = triggersSummary->GetList();
    static int firstTimeBin = -1;

    for(vector<ChanEvent*>::const_iterator it = stops.begin();
    it != stops.end(); it++) {
        ChanEvent *chan = *it;

        unsigned int loc = chan->GetChanID().GetLocation();

        int timeBin = int(chan->GetTime() / logicPlotResolution);
        int startTimeBin = 0;

        if(!isnan(lastStartTime.at(loc))) {
            startTimeBin = int(lastStartTime.at(loc) / logicPlotResolution);
            if(firstTimeBin == -1) {
                firstTimeBin = startTimeBin;
            }
        } else if(firstTimeBin == -1) {
            firstTimeBin = startTimeBin;
        }
        startTimeBin = max(0, startTimeBin - firstTimeBin);
        timeBin -= firstTimeBin;

        for(int bin=startTimeBin; bin < timeBin; bin++) {
            int row = bin / plotSize;
            int col = bin % plotSize;
            plot(DD_RUNTIME_LOGIC, col, row, loc + 1);
            plot(DD_RUNTIME_LOGIC + loc, col, row, 1);
        }
    }
    for(vector<ChanEvent*>::const_iterator it = triggers.begin();
        it != triggers.end(); it++) {
        int timeBin = int((*it)->GetTime() / logicPlotResolution);
        timeBin -= firstTimeBin;
        if(timeBin >= maxBin || timeBin < 0)
            continue;

        int row = timeBin / plotSize;
        int col = timeBin % plotSize;

        plot(DD_RUNTIME_LOGIC, col, row, 20);
        for(int i=1; i < MAX_LOGIC; i++)
            plot(DD_RUNTIME_LOGIC + i, col, row, 5);
    }

    EndProcess();
    return true;
}
