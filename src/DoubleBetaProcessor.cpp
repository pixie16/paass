/** \file DoubleBetaProcessor.cpp
 *\brief A DoubleBeta processor class that can be used to analyze double
 * beta detectors.
 *\author S. V. Paulauskas
 *\date October 26, 2014
 */
#include "BarBuilder.hpp"
#include "DammPlotIds.hpp"
#include "DoubleBetaProcessor.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "TimingMapBuilder.hpp"


namespace dammIds {
    namespace doublebeta {
        const int DD_SINGLESQDC = 0;//!< ID for the singles QDC
        const int DD_QDC  = 1; //!< ID for the Bar QDC of the double beta detector
        const int DD_TDIFF = 2;//!< ID to plot the Time Difference between ends
        const int DD_PP = 3;//!< ID to plot the phase-phase for your favorite bar (0)
        const int DD_QDCTDIFF = 4;//!< QDC vs. TDiff for your favorite bar (0)
    }
}

using namespace std;
using namespace dammIds::doublebeta;

DoubleBetaProcessor::DoubleBetaProcessor():
    EventProcessor(dammIds::doublebeta::OFFSET, dammIds::doublebeta::RANGE,
                   "Double Beta") {
    associatedTypes.insert("beta");
}

void DoubleBetaProcessor::DeclarePlots(void) {
    DeclareHistogram2D(DD_SINGLESQDC, SD, S3, "Location vs. Singles QDC");
    DeclareHistogram2D(DD_QDC, SD, S3, "Location vs. Coincident QDC");
    DeclareHistogram2D(DD_TDIFF, SB, S3, "Location vs. Time Difference");
    DeclareHistogram2D(DD_PP, SC, SC,"Phase vs. Phase - Bar 0 Only");
    DeclareHistogram2D(DD_QDCTDIFF, SC, SC,"TimeDiff vs. Coincident QDC");
}

bool DoubleBetaProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return(false);

    static const vector<ChanEvent*> & events =
        event.GetSummary("beta:double")->GetList();

    TimingMapBuilder singlesMap(events);
    TimingMap sngls = singlesMap.GetMap();
    for(TimingMap::iterator it = sngls.begin(); it != sngls.end(); it ++)
        plot(DD_SINGLESQDC,(*it).second.GetTraceQdc(), (*it).first.first);

    BarBuilder builder(events);
    betas_ = builder.GetBarMap();

    double resolution = 2;
    double offset = 1500;

    for(BarMap::const_iterator it = betas_.begin(); it != betas_.end(); it++) {
        unsigned int barNum = (*it).first.first;
        plot(DD_QDC, (*it).second.GetLeftSide().GetTraceQdc(), barNum * 2);
        plot(DD_QDC, (*it).second.GetRightSide().GetTraceQdc(), barNum * 2 + 1);
        plot(DD_TDIFF, (*it).second.GetTimeDifference()*resolution + offset, barNum);
        if(barNum == 0) {
            plot(DD_PP, (*it).second.GetLeftSide().GetPhase()*resolution,
                        (*it).second.GetRightSide().GetPhase()*resolution);
            plot(DD_QDCTDIFF, (*it).second.GetTimeDifference()*resolution+offset,
             (*it).second.GetLeftSide().GetTraceQdc());
        }
    }
    return(true);
}

bool DoubleBetaProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);
    return(true);
}
