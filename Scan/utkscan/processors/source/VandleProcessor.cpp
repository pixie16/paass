/** \file VandleProcessor.cpp
 *\brief Processes information for VANDLE
 *
 *Processes information from the VANDLE Bars, allows for
 *beta-gamma-neutron correlations. The prototype for this
 *code was written by M. Madurga.
 *
 *\author S. V. Paulauskas
 *\date 26 July 2010
 */
#include <fstream>
#include <iostream>

#include <cmath>

#include "BarBuilder.hpp"
#include "DetectorDriver.hpp"
#include "TimingMapBuilder.hpp"
#include "VandleProcessor.hpp"

namespace dammIds {
    namespace vandle {
        const unsigned int BIG_OFFSET = 20; //!< Offset for big bars
        const unsigned int MED_OFFSET = 40;//!< Offset for medium bars
        const unsigned int DEBUGGING_OFFSET = 60;//!< Offset for debugging hists

        const int DD_TQDCBARS = 0;//!< QDC for the bars
        const int DD_TIMEDIFFBARS = 1;//!< time difference in the bars
        const int DD_TOFBARS = 2;//!< time of flight for the bars
        const int DD_CORTOFBARS =3;//!< corrected time of flight
        const int DD_TQDCAVEVSTOF = 4;//!< Ave QDC vs. ToF
        const int DD_TQDCAVEVSCORTOF = 5;//!< Ave QDC vs. Cor ToF
        const int DD_GAMMAENERGYVSTOF = 6;//!< Gamma Energy vs. ToF
        const int DD_TQDCAVEVSTOF_VETO = 7;//!< QDC vs. ToF - Vetoed
        const int DD_TOFBARS_VETO = 8;//!< ToF - Vetoed

        const int D_DIAGNOSTIC = 0 + DEBUGGING_OFFSET;//!< Diagnostic Infor
        const int DD_DEBUGGING =
                1 + DEBUGGING_OFFSET;//!< 2D Hist to count problems
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::vandle;

enum DIAGNOSTICS {
    PROCESS_CALLED, NO_VANDLES, ONE_END, NO_BARS_BUILT, NUMBER_OF_BARS,
    REJECTED_BARS
};

VandleProcessor::VandleProcessor() :
        EventProcessor(OFFSET, RANGE, "VandleProcessor") {
    associatedTypes.insert("vandle");
}

VandleProcessor::VandleProcessor(const std::vector<std::string> &typeList,
                                 const double &res, const double &offset,
                                 const unsigned int &numStarts) :
        EventProcessor(OFFSET, RANGE, "VandleProcessor") {
    associatedTypes.insert("vandle");
    plotMult_ = res;
    plotOffset_ = offset;
    numStarts_ = numStarts;

    hasSmall_ = hasMed_ = hasBig_ = false;
    for (vector<string>::const_iterator it = typeList.begin();
         it != typeList.end(); it++) {
        if ((*it) == "small")
            hasSmall_ = true;
        if ((*it) == "medium")
            hasMed_ = true;
        if ((*it) == "big")
            hasBig_ = true;
    }
    if (typeList.size() == 0)
        hasSmall_ = true;
}

void VandleProcessor::DeclarePlots(void) {
    if (hasSmall_) {
        DeclareHistogram2D(DD_TQDCBARS, SD, S8,
                           "Det Loc vs Trace QDC - Left Even - Right Odd");
        DeclareHistogram2D(DD_TIMEDIFFBARS, SB, S6,
                           "Bars vs. Time Differences");
        DeclareHistogram2D(DD_TOFBARS, SC, S8,
                           "Bar vs. Time of Flight");
        DeclareHistogram2D(DD_CORTOFBARS, SC, S8,
                           "Bar vs  Cor Time of Flight");
        DeclareHistogram2D(DD_TQDCAVEVSTOF, SC, SD,
                           "<E> vs. TOF(0.5ns/bin)");
        DeclareHistogram2D(DD_TQDCAVEVSCORTOF, SC, SD,
                           "<E> vs. CorTOF(0.5ns/bin)");
        DeclareHistogram2D(DD_GAMMAENERGYVSTOF, SC, S9,
                           "C-ToF vs. E_gamma");
    }
    if (hasBig_) {
        DeclareHistogram2D(DD_TQDCBARS + BIG_OFFSET, SD, S6,
                           "Det Loc vs Trace QDC");
        DeclareHistogram2D(DD_TIMEDIFFBARS + BIG_OFFSET, SB, S6,
                           "Bars vs. Time Differences");
        DeclareHistogram2D(DD_TOFBARS + BIG_OFFSET, SC, S6,
                           "Bar vs. Time of Flight");
        DeclareHistogram2D(DD_CORTOFBARS + BIG_OFFSET, SC, S6,
                           "Bar vs  Cor Time of Flight");
        DeclareHistogram2D(DD_TQDCAVEVSTOF + BIG_OFFSET, SC, SD,
                           "<E> vs. TOF(0.5ns/bin)");
        DeclareHistogram2D(DD_TQDCAVEVSCORTOF + BIG_OFFSET, SC, SD,
                           "<E> vs. CorTOF(0.5ns/bin)");
        DeclareHistogram2D(DD_GAMMAENERGYVSTOF + BIG_OFFSET, SC, S9,
                           "C-ToF vs. E_gamma");
    }
    if (hasMed_) {
        DeclareHistogram2D(DD_TQDCBARS + MED_OFFSET, SD, S6,
                           "Det Loc vs Trace QDC");
        DeclareHistogram2D(DD_TIMEDIFFBARS + MED_OFFSET, SB, S6,
                           "Bars vs. Time Differences");
        DeclareHistogram2D(DD_TOFBARS + MED_OFFSET, SC, S6,
                           "Bar vs. Time of Flight");
        DeclareHistogram2D(DD_CORTOFBARS + MED_OFFSET, SC, S6,
                           "Bar vs  Cor Time of Flight");
        DeclareHistogram2D(DD_TQDCAVEVSTOF + MED_OFFSET, SC, SD,
                           "<E> vs. TOF(0.5ns/bin)");
        DeclareHistogram2D(DD_TQDCAVEVSCORTOF + MED_OFFSET, SC, SD,
                           "<E> vs. CorTOF(0.5ns/bin)");
        DeclareHistogram2D(DD_GAMMAENERGYVSTOF + MED_OFFSET, SC, S9,
                           "C-ToF vs. E_gamma");
    }

    DeclareHistogram1D(D_DIAGNOSTIC, S5, "1D Debugging");
    DeclareHistogram2D(DD_DEBUGGING, S8, S8, "2D Debugging");
}

bool VandleProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;
    ClearMaps();

    static const vector<ChanEvent *> &events =
            event.GetSummary("vandle")->GetList();

    if (events.empty() || events.size() == 1) {
        if (events.empty())
            plot(D_DIAGNOSTIC, NO_VANDLES);
        if (events.size() == 1)
            plot(D_DIAGNOSTIC, ONE_END);
        return (false);
    }

    BarBuilder billy(events);
    billy.BuildBars();
    bars_ = billy.GetBarMap();

    if (bars_.empty()) {
        plot(D_DIAGNOSTIC, NO_BARS_BUILT);
        return (false);
    }

    FillVandleOnlyHists();
    return (true);
}

bool VandleProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);
    plot(D_DIAGNOSTIC, PROCESS_CALLED);

    geSummary_ = event.GetSummary("ge");

    static const vector<ChanEvent *> &betaStarts =
            event.GetSummary("beta_scint:beta")->GetList();
    static const vector<ChanEvent *> &liquidStarts =
            event.GetSummary("liquid:scint:start")->GetList();
    static const vector<ChanEvent *> &pspmtStarts =
            event.GetSummary("pspmt:dynode:start")->GetList();

    vector<ChanEvent *> startEvents;
    startEvents.insert(startEvents.end(),
                       betaStarts.begin(), betaStarts.end());
    startEvents.insert(startEvents.end(),
                       liquidStarts.begin(), liquidStarts.end());
    startEvents.insert(startEvents.end(),
                       pspmtStarts.begin(), pspmtStarts.end());

    TimingMapBuilder bldStarts(startEvents);
    starts_ = bldStarts.GetMap();

    static const vector<ChanEvent *> &doubleBetaStarts =
            event.GetSummary("beta:double:start")->GetList();
    BarBuilder startBars(doubleBetaStarts);
    startBars.BuildBars();
    barStarts_ = startBars.GetBarMap();

    if (!doubleBetaStarts.empty())
        AnalyzeBarStarts();
    else
        AnalyzeStarts();

    EndProcess();
    return (true);
}

void VandleProcessor::AnalyzeBarStarts(void) {
    for (BarMap::iterator it = bars_.begin(); it != bars_.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;

        if (!bar.GetHasEvent())
            continue;

        unsigned int histTypeOffset = ReturnOffset(bar.GetType());
        unsigned int barLoc = barId.first;
        const TimingCalibration cal = bar.GetCalibration();

        for (BarMap::iterator itStart = barStarts_.begin();
             itStart != barStarts_.end(); itStart++) {
            unsigned int startLoc = (*itStart).first.first;
            unsigned int barPlusStartLoc = barLoc * numStarts_ + startLoc;

            BarDetector start = (*itStart).second;

            double tof = bar.GetCorTimeAve() -
                         start.GetCorTimeAve() + cal.GetTofOffset(startLoc);

            double corTof =
                    CorrectTOF(tof, bar.GetFlightPath(), cal.GetZ0());

            plot(DD_TOFBARS + histTypeOffset, tof * plotMult_ + plotOffset_,
                 barPlusStartLoc);
            plot(DD_CORTOFBARS, corTof * plotMult_ + plotOffset_,
                 barPlusStartLoc);

            if (cal.GetTofOffset(startLoc) != 0) {
                plot(DD_TQDCAVEVSTOF + histTypeOffset,
                     tof * plotMult_ + plotOffset_,
                     bar.GetQdc());
                plot(DD_TQDCAVEVSCORTOF + histTypeOffset,
                     corTof * plotMult_ + plotOffset_, bar.GetQdc());
            }

            if (geSummary_) {
                if (geSummary_->GetMult() > 0) {
                    const vector<ChanEvent *> &geList = geSummary_->GetList();
                    for (vector<ChanEvent *>::const_iterator itGe = geList.begin();
                         itGe != geList.end(); itGe++) {
                        double calEnergy = (*itGe)->GetCalEnergy();
                        plot(DD_GAMMAENERGYVSTOF + histTypeOffset, calEnergy,
                             tof);
                    }
                } else {
                    plot(DD_TQDCAVEVSTOF_VETO + histTypeOffset, tof,
                         bar.GetQdc());
                    plot(DD_TOFBARS_VETO + histTypeOffset, tof,
                         barPlusStartLoc);
                }
            }
        } // for(TimingMap::iterator itStart
    } //(BarMap::iterator itBar
} //void VandleProcessor::AnalyzeData

void VandleProcessor::AnalyzeStarts(void) {
    for (BarMap::iterator it = bars_.begin(); it != bars_.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;

        if (!bar.GetHasEvent())
            continue;

        unsigned int histTypeOffset = ReturnOffset(bar.GetType());
        unsigned int barLoc = barId.first;
        const TimingCalibration cal = bar.GetCalibration();

        for (TimingMap::iterator itStart = starts_.begin();
             itStart != starts_.end(); itStart++) {
            if (!(*itStart).second.GetIsValid())
                continue;

            unsigned int startLoc = (*itStart).first.first;
            unsigned int barPlusStartLoc = barLoc * numStarts_ + startLoc;
            HighResTimingData start = (*itStart).second;

            double tof = bar.GetCorTimeAve() -
                         start.GetCorrectedTime() + cal.GetTofOffset(startLoc);

            double corTof =
                    CorrectTOF(tof, bar.GetFlightPath(), cal.GetZ0());

            plot(DD_TOFBARS + histTypeOffset, tof * plotMult_ + plotOffset_,
                 barPlusStartLoc);
            plot(DD_TQDCAVEVSTOF + histTypeOffset,
                 tof * plotMult_ + plotOffset_, bar.GetQdc());

            plot(DD_CORTOFBARS, corTof * plotMult_ + plotOffset_,
                 barPlusStartLoc);
            plot(DD_TQDCAVEVSCORTOF + histTypeOffset,
                 corTof * plotMult_ + plotOffset_,
                 bar.GetQdc());

            if (geSummary_) {
                if (geSummary_->GetMult() > 0) {
                    const vector<ChanEvent *> &geList = geSummary_->GetList();
                    for (vector<ChanEvent *>::const_iterator itGe = geList.begin();
                         itGe != geList.end(); itGe++) {
                        double calEnergy = (*itGe)->GetCalEnergy();
                        plot(DD_GAMMAENERGYVSTOF + histTypeOffset, calEnergy,
                             tof);
                    }
                } else {
                    plot(DD_TQDCAVEVSTOF_VETO + histTypeOffset, tof,
                         bar.GetQdc());
                    plot(DD_TOFBARS_VETO + histTypeOffset, tof,
                         barPlusStartLoc);
                }
            }
        } // for(TimingMap::iterator itStart
    } //(BarMap::iterator itBar
} //void VandleProcessor::AnalyzeData

void VandleProcessor::ClearMaps(void) {
    bars_.clear();
    starts_.clear();
}

void VandleProcessor::FillVandleOnlyHists(void) {
    for (BarMap::const_iterator it = bars_.begin(); it != bars_.end(); it++) {
        plot(D_DIAGNOSTIC, NUMBER_OF_BARS);

        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;
        unsigned int OFFSET = ReturnOffset(barId.second);

        if (!bar.GetHasEvent()) {
            plot(D_DIAGNOSTIC, REJECTED_BARS);
            continue;
        }

        plot(DD_TQDCBARS + OFFSET,
             bar.GetLeftSide().GetTraceQdc(), barId.first * 2);
        plot(DD_TQDCBARS + OFFSET,
             bar.GetRightSide().GetTraceQdc(), barId.first * 2 + 1);
        plot(DD_TIMEDIFFBARS + OFFSET,
             bar.GetTimeDifference() * plotMult_ + plotOffset_, barId.first);
    }
}

unsigned int VandleProcessor::ReturnOffset(const std::string &type) {
    if (type == "small")
        return (0);
    if (type == "big")
        return (BIG_OFFSET);
    if (type == "medium")
        return (MED_OFFSET);
    return (-1);
}
