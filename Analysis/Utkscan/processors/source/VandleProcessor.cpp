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
#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "RawEvent.hpp"
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
        const int DD_CORTOFBARS = 3;//!< corrected time of flight
        const int DD_TQDCAVEVSTOF = 4;//!< Ave QDC vs. ToF
        const int DD_TQDCAVEVSCORTOF = 5;//!< Ave QDC vs. Cor ToF
        const int DD_GAMMAENERGYVSTOF = 6;//!< Gamma Energy vs. ToF
        const int DD_TQDCAVEVSTOF_VETO = 7;//!< QDC vs. ToF - Vetoed
        const int DD_TOFBARS_VETO = 8;//!< ToF - Vetoed

        const int D_DEBUGGING = 0 + DEBUGGING_OFFSET;//!< Debugging countable problems
        const int DD_DEBUGGING = 1 + DEBUGGING_OFFSET;//!< 2D Hist to count problems
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::vandle;

VandleProcessor::VandleProcessor() : EventProcessor(OFFSET, RANGE, "VandleProcessor") {
    associatedTypes.insert("vandle");
}

VandleProcessor::VandleProcessor(const std::vector<std::string> &typeList, const double &res, const double &offset,
                                 const unsigned int &numStarts, const double &compression/*=1.0*/) :
        EventProcessor(OFFSET,RANGE,"VandleProcessor") {

#ifdef useroot
    std::string outputPath = Globals::get()->GetOutputPath();
    std::string treefilename = outputPath + output_name + "_vandleTree.root";
    TFile_tree = new TFile(treefilename.c_str(),"recreate");

    data_summary_tree = new TTree("data_summary_tree","data_summary_tree");

    data_summary_tree->Branch("evtNumber",&evtNumber);
    data_summary_tree->Branch("output_name",&output_name);

    data_summary_tree->Branch("vandle_subtype",&vandle_subtype);
    data_summary_tree->Branch("vandle_BarQDC",&vandle_BarQDC);
    data_summary_tree->Branch("vandle_lQDC",&vandle_lQDC);
    data_summary_tree->Branch("vandle_rQDC",&vandle_rQDC);
    data_summary_tree->Branch("vandle_QDCPos",&vandle_QDCPos);
    data_summary_tree->Branch("vandle_TOF",&vandle_TOF);
    data_summary_tree->Branch("vandle_lSnR",&vandle_lSnR);
    data_summary_tree->Branch("vandle_rSnR",&vandle_rSnR);
    data_summary_tree->Branch("vandle_lAmp",&vandle_lAmp);
    data_summary_tree->Branch("vandle_rAmp",&vandle_rAmp);
    data_summary_tree->Branch("vandle_lMaxAmpPos",&vandle_lMaxAmpPos);
    data_summary_tree->Branch("vandle_rMaxAmpPos",&vandle_rMaxAmpPos);
    data_summary_tree->Branch("vandle_lAveBaseline",&vandle_lAveBaseline);
    data_summary_tree->Branch("vandle_rAveBaseline",&vandle_rAveBaseline);
    data_summary_tree->Branch("vandle_barNum",&vandle_barNum);
    data_summary_tree->Branch("vandle_TAvg",&vandle_TAvg);
    data_summary_tree->Branch("vandle_Corrected_TAvg",&vandle_Corrected_TAvg);
    data_summary_tree->Branch("vandle_TDiff",&vandle_TDiff);
    data_summary_tree->Branch("vandle_Corrected_TDiff",&vandle_Corrected_TDiff);
    data_summary_tree->Branch("vandle_ltrace",&vandle_ltrace);
    data_summary_tree->Branch("vandle_rtrace",&vandle_rtrace);

    data_summary_tree->Branch("beta_BarQDC",&beta_BarQDC);
    data_summary_tree->Branch("beta_lQDC",&beta_lQDC);
    data_summary_tree->Branch("beta_rQDC",&beta_rQDC);
    data_summary_tree->Branch("beta_lSnR",&beta_lSnR);
    data_summary_tree->Branch("beta_rSnR",&beta_rSnR);
    data_summary_tree->Branch("beta_lAmp",&beta_lAmp);
    data_summary_tree->Branch("beta_rAmp",&beta_rAmp);
    data_summary_tree->Branch("beta_lMaxAmpPos",&beta_lMaxAmpPos);
    data_summary_tree->Branch("beta_rMaxAmpPos",&beta_rMaxAmpPos);
    data_summary_tree->Branch("beta_lAveBaseline",&vandle_lAveBaseline);
    data_summary_tree->Branch("beta_rAveBaseline",&vandle_rAveBaseline);
    data_summary_tree->Branch("beta_barNum",&beta_barNum);
    data_summary_tree->Branch("beta_TAvg",&beta_TAvg);
    data_summary_tree->Branch("beta_Corrected_TAvg",&beta_Corrected_TAvg);
    data_summary_tree->Branch("beta_TDiff",&beta_TDiff);
    data_summary_tree->Branch("beta_Corrected_TDiff",&beta_Corrected_TDiff);
    data_summary_tree->Branch("beta_ltrace",&beta_ltrace);
    data_summary_tree->Branch("beta_rtrace",&beta_rtrace);
#endif

    associatedTypes.insert("vandle");
    plotMult_ = res;
    plotOffset_ = offset;
    numStarts_ = numStarts;
    qdcComp_ = compression;

    if(typeList.empty())
        requestedTypes_.insert("small");
    else
        requestedTypes_ = set<string>(typeList.begin(), typeList.end());
}

void VandleProcessor::DeclarePlots(void) {
    for(set<string>::iterator it = requestedTypes_.begin(); it != requestedTypes_.end(); it++) {
        unsigned int offset = ReturnOffset(*it);
        DeclareHistogram2D(DD_TQDCBARS + offset, SD, S8, "Det Loc vs Trace QDC - Left Even - Right Odd");
        DeclareHistogram2D(DD_TIMEDIFFBARS + offset, SB, S6, "Bars vs. Time Differences");
        DeclareHistogram2D(DD_TOFBARS + offset, SC, S8, "Bar vs. Time of Flight");
        DeclareHistogram2D(DD_CORTOFBARS + offset, SC, S8, "Bar vs  Cor Time of Flight");
        DeclareHistogram2D(DD_TQDCAVEVSTOF + offset, SC, SD, "<E> vs. TOF(0.5ns/bin)");
        DeclareHistogram2D(DD_TQDCAVEVSCORTOF + offset, SC, SD, "<E> vs. CorTOF(0.5ns/bin)");
        DeclareHistogram2D(DD_GAMMAENERGYVSTOF + offset, SC, S9, "C-ToF vs. E_gamma");
    }
    DeclareHistogram1D(D_DEBUGGING, S5, "1D Debugging");
    DeclareHistogram2D(DD_DEBUGGING, S8, S8, "2D Debugging");
}

bool VandleProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    bars_.clear();
    starts_.clear();

    static const vector<ChanEvent *> &events = event.GetSummary("vandle")->GetList();

    if (events.empty() || events.size() < 2) {
        if (events.empty())
            plot(D_DEBUGGING, 27);
        if (events.size() < 2)
            plot(D_DEBUGGING, 2);
        return false;
    }

    BarBuilder billy(events);
    billy.BuildBars();
    bars_ = billy.GetBarMap();

    if (bars_.empty()) {
        plot(D_DEBUGGING, 25);
        return false;
    }

    FillVandleOnlyHists();
    return true;
}

bool VandleProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    plot(D_DEBUGGING, 30);

    geSummary_ = event.GetSummary("clover");

    static const vector<ChanEvent *> &betaStarts = event.GetSummary("beta_scint:beta")->GetList();
    static const vector<ChanEvent *> &liquidStarts = event.GetSummary("liquid:scint:start")->GetList();

    vector<ChanEvent *> startEvents;
    startEvents.insert(startEvents.end(), betaStarts.begin(), betaStarts.end());
    startEvents.insert(startEvents.end(), liquidStarts.begin(), liquidStarts.end());

    TimingMapBuilder bldStarts(startEvents);
    starts_ = bldStarts.GetMap();

    static const vector<ChanEvent *> &doubleBetaStarts = event.GetSummary("beta:double:start")->GetList();
    BarBuilder startBars(doubleBetaStarts);
    startBars.BuildBars();
    barStarts_ = startBars.GetBarMap();

    for (BarMap::iterator it = bars_.begin(); it != bars_.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;

        if (!bar.GetHasEvent())
            continue;

        if (!doubleBetaStarts.empty())
            AnalyzeBarStarts(bar, barId.first);
        else
            AnalyzeStarts(bar, barId.first);
    }

    EndProcess();
    return true;
}

void VandleProcessor::AnalyzeBarStarts(const BarDetector &bar, unsigned int &barLoc) {
        for (BarMap::iterator itStart = barStarts_.begin(); itStart != barStarts_.end(); itStart++) {
            unsigned int startLoc = (*itStart).first.first;
            BarDetector start = (*itStart).second;
            double tof = bar.GetCorTimeAve() - start.GetCorTimeAve() + bar.GetCalibration().GetTofOffset(startLoc);
            double corTof = CorrectTOF(tof, bar.GetFlightPath(), bar.GetCalibration().GetZ0());

#ifdef useroot
            vandle_subtype=bar.GetType();
            vandle_lSnR=bar.GetLeftSide().GetTrace().GetSignalToNoiseRatio();
            vandle_rSnR=bar.GetRightSide().GetTrace().GetSignalToNoiseRatio();
            vandle_lAmp=bar.GetLeftSide().GetMaximumValue();
            vandle_rAmp=bar.GetRightSide().GetMaximumValue();
            vandle_lMaxAmpPos=bar.GetLeftSide().GetMaximumPosition();
            vandle_rMaxAmpPos=bar.GetRightSide().GetMaximumPosition();
            vandle_lAveBaseline=bar.GetLeftSide().GetAveBaseline();
            vandle_rAveBaseline=bar.GetRightSide().GetAveBaseline();
            vandle_BarQDC=bar.GetQdc();
            vandle_QDCPos=bar.GetQdcPosition();
            vandle_lQDC=bar.GetLeftSide().GetTraceQdc();
            vandle_rQDC=bar.GetRightSide().GetTraceQdc();
            vandle_TOF=tof;
            vandle_barNum=barLoc;
            vandle_TAvg=bar.GetTimeAverage();
            vandle_Corrected_TAvg=bar.GetCorTimeAve();
            vandle_TDiff=bar.GetTimeDifference();
            vandle_Corrected_TDiff=bar.GetCorTimeDiff();
            vandle_ltrace=bar.GetLeftSide().GetTrace();
            vandle_rtrace=bar.GetRightSide().GetTrace();

            beta_lSnR=start.GetLeftSide().GetTrace().GetSignalToNoiseRatio();
            beta_rSnR=start.GetRightSide().GetTrace().GetSignalToNoiseRatio();
            beta_lAmp=start.GetLeftSide().GetMaximumValue();
            beta_rAmp=start.GetRightSide().GetMaximumValue();
            beta_lMaxAmpPos=start.GetLeftSide().GetMaximumPosition();
            beta_rMaxAmpPos=start.GetRightSide().GetMaximumPosition();
            beta_lAveBaseline=start.GetLeftSide().GetAveBaseline();
            beta_rAveBaseline=start.GetRightSide().GetAveBaseline();
            beta_BarQDC=start.GetQdc();
            beta_lQDC=bar.GetLeftSide().GetTraceQdc();
            beta_rQDC=bar.GetRightSide().GetTraceQdc();
            beta_barNum=startLoc;
            beta_TAvg=start.GetTimeAverage();
            beta_Corrected_TAvg=start.GetCorTimeAve();
            beta_TDiff=start.GetTimeDifference();
            beta_Corrected_TDiff=start.GetCorTimeDiff();
            beta_ltrace=start.GetLeftSide().GetTrace();
            beta_rtrace=start.GetRightSide().GetTrace();
            // printf("evtNumber:%d \n",evtNumber);

            data_summary_tree->Fill();
#endif
            PlotTofHistograms(tof, corTof, bar.GetQdc(), barLoc * numStarts_ + startLoc, ReturnOffset(bar.GetType()));
        }
}

void VandleProcessor::AnalyzeStarts(const BarDetector &bar, unsigned int &barLoc) {
        for (TimingMap::iterator itStart = starts_.begin(); itStart != starts_.end(); itStart++) {
            if (!(*itStart).second.GetIsValid())
                continue;

            unsigned int startLoc = (*itStart).first.first;
            HighResTimingData start = (*itStart).second;

            double tof = bar.GetCorTimeAve() - start.GetWalkCorrectedTime() + bar.GetCalibration().GetTofOffset(startLoc);
            double corTof = CorrectTOF(tof, bar.GetFlightPath(), bar.GetCalibration().GetZ0());

            PlotTofHistograms(tof, corTof, bar.GetQdc(), barLoc * numStarts_ + startLoc, ReturnOffset(bar.GetType()));
        }
}

void VandleProcessor::PlotTofHistograms(const double &tof, const double &cortof, const double &qdc,
                                        const unsigned int &barPlusStartLoc, const unsigned int &offset) {
    plot(DD_TOFBARS + offset, tof * plotMult_ + plotOffset_, barPlusStartLoc);
    plot(DD_TQDCAVEVSTOF + offset, tof * plotMult_ + plotOffset_, qdc / qdcComp_);

    plot(DD_CORTOFBARS + offset, cortof * plotMult_ + plotOffset_, barPlusStartLoc);
    plot(DD_TQDCAVEVSCORTOF + offset, cortof * plotMult_ + plotOffset_, qdc / qdcComp_);

    if (geSummary_) {
        if (geSummary_->GetMult() > 0) {
            const vector<ChanEvent *> &geList = geSummary_->GetList();
            for (vector<ChanEvent *>::const_iterator itGe = geList.begin(); itGe != geList.end(); itGe++)
                plot(DD_GAMMAENERGYVSTOF + offset, (*itGe)->GetCalibratedEnergy(), tof);
        } else {
            plot(DD_TQDCAVEVSTOF_VETO + offset, tof, qdc / qdcComp_);
            plot(DD_TOFBARS_VETO + offset, tof, barPlusStartLoc);
        }
    }
}

void VandleProcessor::FillVandleOnlyHists(void) {
    for (BarMap::const_iterator it = bars_.begin(); it != bars_.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        unsigned int OFFSET = ReturnOffset(barId.second);

        plot(DD_TQDCBARS + OFFSET, (*it).second.GetLeftSide().GetTraceQdc() / qdcComp_, barId.first * 2);
        plot(DD_TQDCBARS + OFFSET, (*it).second.GetRightSide().GetTraceQdc() / qdcComp_, barId.first * 2 + 1);
        plot(DD_TIMEDIFFBARS + OFFSET, (*it).second.GetTimeDifference() * plotMult_ + plotOffset_, barId.first);
    }
}

unsigned int VandleProcessor::ReturnOffset(const std::string &type) {
    if (type == "small")
        return 0;
    if (type == "big")
        return (BIG_OFFSET);
    if (type == "medium")
        return (MED_OFFSET);
    return numeric_limits<unsigned int>::max();
}
