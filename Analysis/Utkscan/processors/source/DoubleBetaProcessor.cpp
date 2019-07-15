/** \file DoubleBetaProcessor.cpp
 *\brief A DoubleBeta processor class that can be used to analyze double
 * beta detectors.
 *\author S. V. Paulauskas
 *\date October 26, 2014
 */
#include "BarBuilder.hpp"
#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "DoubleBetaProcessor.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "TimingMapBuilder.hpp"
#include "TreeCorrelator.hpp"

namespace dammIds {
    namespace doublebeta {
        const int DD_SINGLESQDC = 0;//!< ID for the singles QDC
        const int DD_QDC = 1; //!< ID for the Bar QDC of the double beta detector
        const int DD_TDIFF = 2;//!< ID to plot the Time Difference between ends
        const int DD_PP = 3;//!< ID to plot the phase-phase for your favorite bar (0)
        const int DD_QDCTDIFF = 4;//!< QDC vs. TDiff for your favorite bar (0)
        const int DD_BETAMAXXVAL = 5;//!< Max Value in your favorite beta PMT
    }
}

using namespace std;
using namespace dammIds::doublebeta;

DoubleBetaProcessor::DoubleBetaProcessor() :
        EventProcessor(OFFSET, RANGE, "DoubleBetaProcessor") {
    associatedTypes.insert("beta");
}

void DoubleBetaProcessor::DeclarePlots(void) {
    DeclareHistogram2D(DD_QDC, SD, S3, "Location vs. Coincident QDC");
    DeclareHistogram2D(DD_TDIFF, SB, S3, "Location vs. Time Difference");
    DeclareHistogram2D(DD_PP, SC, SC,"Phase vs. Phase - Bar 0 Only");
    DeclareHistogram2D(DD_QDCTDIFF, SC, SE,"TimeDiff vs. Coincident QDC");
    DeclareHistogram2D(DD_BETAMAXXVAL,SD,S3,"Max Value in the Trace");
}

bool DoubleBetaProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return (false);
    lrtbars_.clear();
    bars_.clear();


    static const vector<ChanEvent *> &events =
            event.GetSummary("beta:double")->GetList();

    BarBuilder builder(events);
    builder.BuildBars();

    lrtbars_ = builder.GetLrtBarMap();
    bars_ = builder.GetBarMap();

    double resolution = 2;
    double offset = 1500;

    for (auto it = lrtbars_.begin(); it != lrtbars_.end(); it++) {
        stringstream place;
        place << "DoubleBeta" << (*it).first;
        EventData data((*it).second.first, (*it).second.second, (*it).first);
        TreeCorrelator::get()->place(place.str())->activate(data);
        if (DetectorDriver::get()->GetSysRootOutput()){
            DBstruc.isLowResBeta = true;
            DBstruc.detNum = (*it).first;
            DBstruc.energy = (*it).second.second;
            //record time in ns, LRTBarMap uses the Walkcorrected Time which uses GetTime()
            // Note we lose the ChanID() structure when we build the lowres bars. So we assume they are on the rev default which may or not be right. This probably requires a change to the low res bar building.
            //TODO fix the low res bars to add GetChanID() compatibility / make lrtbars_ more straight forward to access
            DBstruc.timeAvg = (*it).second.first * Globals::get()->GetAdcClockInSeconds() *1.0e9; 
            pixie_tree_event_->doublebeta_vec_.emplace_back(DBstruc);
            DBstruc = processor_struct::DOUBLEBETA_DEFAULT_STRUCT;
        }
    }

    for (BarMap::const_iterator it = bars_.begin(); it != bars_.end(); it++) {
        unsigned int barNum = (*it).first.first;
        int modulefreq = (*it).second.GetLeftSide().GetChanID().GetModFreq(); // We are assuming that the left and  right sides are on the same module (or at least the same speed of module)
        plot(DD_QDC, (*it).second.GetLeftSide().GetTraceQdc(), barNum * 2);
        plot(DD_QDC, (*it).second.GetRightSide().GetTraceQdc(), barNum * 2 + 1);
        plot(DD_TDIFF, (*it).second.GetTimeDifference()*resolution + offset, barNum);
        plot(DD_BETAMAXXVAL,(*it).second.GetLeftSide().GetMaximumValue(),barNum*2);
        plot(DD_BETAMAXXVAL,(*it).second.GetRightSide().GetMaximumValue(),barNum*2+1);

        if (DetectorDriver::get()->GetSysRootOutput()){
            DBstruc.isHighResBeta = true;
            DBstruc.detNum = barNum;
            DBstruc.timeAvg = (*it).second.GetTimeAverage();
            DBstruc.timeDiff = (*it).second.GetTimeDifference();
            DBstruc.timeL = (*it).second.GetLeftSide().GetTimeSansCfd() * Globals::get()->GetClockInSeconds(modulefreq) * 1e9; //store ns;
            DBstruc.timeR = (*it).second.GetRightSide().GetTimeSansCfd() * Globals::get()->GetClockInSeconds(modulefreq) * 1e9; //store ns;
            DBstruc.barQdc = (*it).second.GetQdc();
            DBstruc.tMaxValR = (*it).second.GetRightSide().GetMaximumValue();
            DBstruc.tMaxValL = (*it).second.GetLeftSide().GetMaximumValue();
            pixie_tree_event_->doublebeta_vec_.emplace_back(DBstruc);
            DBstruc = processor_struct::DOUBLEBETA_DEFAULT_STRUCT;
        }

        if(barNum == 0) {
            plot(DD_PP, (*it).second.GetLeftSide().GetPhaseInNs()*resolution,(*it).second.GetRightSide().GetPhaseInNs()*resolution);
            plot(DD_QDCTDIFF, (*it).second.GetTimeDifference()*resolution+offset,(*it).second.GetQdc());
        }
    }
    return (true);
}

bool DoubleBetaProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);
    EndProcess();
    return (true);
}
