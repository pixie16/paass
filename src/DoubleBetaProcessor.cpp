/** \file DoubleBetaProcessor.cpp
 *\brief A DoubleBeta processor class that can be used to build your own.
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
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::doublebeta;

DoubleBetaProcessor::DoubleBetaProcessor():
    EventProcessor(dammIds::doublebeta::OFFSET, dammIds::doublebeta::RANGE,
                   "doublebeta") {
    associatedTypes.insert("beta");
}

void DoubleBetaProcessor::DeclarePlots(void) {
    DeclareHistogram2D(DD_SINGLESQDC, SD, S3, "Location vs. Singles QDC");
    DeclareHistogram2D(DD_QDC, SD, S3, "Location vs. QDC");
    DeclareHistogram2D(DD_TDIFF, SB, S3, "Location vs. TimeDifference");
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
    double offset = 1000;

    for(BarMap::const_iterator it = betas_.begin(); it != betas_.end(); it++) {
        unsigned int barNum = (*it).first.first;
        plot(DD_QDC, (*it).second.GetLeftSide().GetTraceQdc(), barNum * 2);
        plot(DD_QDC, (*it).second.GetRightSide().GetTraceQdc(), barNum * 2 + 1);
        plot(DD_TDIFF, (*it).second.GetTimeDifference()*resolution + offset, barNum);
    }
    return(true);
}

bool DoubleBetaProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);
    return(true);
}
