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
        const unsigned int SMALLNOCAL_OFFSET = 100;//!< Offset for Unclaibrated hists
        const unsigned int MEDNOCALTYPE_OFFSET = 110;//!< Offset for Unclaibrated hists MEDIUM BARS
        const unsigned int BIGNOCALTYPE_OFFSET = 120;//!< Offset for Unclaibrated hists LARGE BARS

        const int DD_TQDCBARS = 0;//!< QDC for the bars
        const int DD_MAXIMUMBARS  = 1;//!< Maximum values for the bars
        const int DD_TIMEDIFFBARS = 2;//!< time difference in the bars
        const int DD_TOFBARS = 3;//!< time of flight for the bars
        const int DD_CORTOFBARS = 4;//!< corrected time of flight
        const int DD_TQDCAVEVSTOF = 5;//!< Ave QDC vs. ToF
        const int DD_TQDCAVEVSCORTOF = 6;//!< Ave QDC vs. Cor ToF
        const int DD_GAMMAENERGYVSTOF = 7;//!< Gamma Energy vs. ToF
        const int DD_TQDCAVEVSTOF_VETO = 8;//!< QDC vs. ToF - Vetoed
        const int DD_TOFBARS_VETO = 9;//!< ToF - Vetoed

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
                                 const unsigned int &numStarts, const double &compression/*=1.0*/,const bool &root) :
        EventProcessor(OFFSET,RANGE,"VandleProcessor") {

    SaveRoot=root;
#ifdef useroot

    std::string outputPath = Globals::get()->GetOutputPath();
    std::string treefilename = outputPath + output_name + "_vandleTree.root";
    if (SaveRoot) {
        TFile_tree = new TFile(treefilename.c_str(), "recreate");

        data_summary_tree = new TTree("data_summary_tree", "data_summary_tree");

        data_summary_tree->Branch("evtNumber", &evtNumber);
        data_summary_tree->Branch("output_name", &output_name);

        data_summary_tree->Branch("vandle_subtype", &vandle_subtype);
        data_summary_tree->Branch("vandle_BarQDC", &vandle_BarQDC);
        data_summary_tree->Branch("vandle_lQDC", &vandle_lQDC);
        data_summary_tree->Branch("vandle_rQDC", &vandle_rQDC);
        data_summary_tree->Branch("vandle_QDCPos", &vandle_QDCPos);
        data_summary_tree->Branch("vandle_TOF", &vandle_TOF);
        data_summary_tree->Branch("vandle_lSnR", &vandle_lSnR);
        data_summary_tree->Branch("vandle_rSnR", &vandle_rSnR);
        data_summary_tree->Branch("vandle_lAmp", &vandle_lAmp);
        data_summary_tree->Branch("vandle_rAmp", &vandle_rAmp);
        data_summary_tree->Branch("vandle_lMaxAmpPos", &vandle_lMaxAmpPos);
        data_summary_tree->Branch("vandle_rMaxAmpPos", &vandle_rMaxAmpPos);
        data_summary_tree->Branch("vandle_lAveBaseline", &vandle_lAveBaseline);
        data_summary_tree->Branch("vandle_rAveBaseline", &vandle_rAveBaseline);
        data_summary_tree->Branch("vandle_barNum", &vandle_barNum);
        data_summary_tree->Branch("vandle_TAvg", &vandle_TAvg);
        data_summary_tree->Branch("vandle_Corrected_TAvg", &vandle_Corrected_TAvg);
        data_summary_tree->Branch("vandle_TDiff", &vandle_TDiff);
        data_summary_tree->Branch("vandle_Corrected_TDiff", &vandle_Corrected_TDiff);
        data_summary_tree->Branch("vandle_ltrace", &vandle_ltrace);
        data_summary_tree->Branch("vandle_rtrace", &vandle_rtrace);

        data_summary_tree->Branch("beta_BarQDC", &beta_BarQDC);
        data_summary_tree->Branch("beta_lQDC", &beta_lQDC);
        data_summary_tree->Branch("beta_rQDC", &beta_rQDC);
        data_summary_tree->Branch("beta_lSnR", &beta_lSnR);
        data_summary_tree->Branch("beta_rSnR", &beta_rSnR);
        data_summary_tree->Branch("beta_lAmp", &beta_lAmp);
        data_summary_tree->Branch("beta_rAmp", &beta_rAmp);
        data_summary_tree->Branch("beta_lMaxAmpPos", &beta_lMaxAmpPos);
        data_summary_tree->Branch("beta_rMaxAmpPos", &beta_rMaxAmpPos);
        data_summary_tree->Branch("beta_lAveBaseline", &vandle_lAveBaseline);
        data_summary_tree->Branch("beta_rAveBaseline", &vandle_rAveBaseline);
        data_summary_tree->Branch("beta_barNum", &beta_barNum);
        data_summary_tree->Branch("beta_TAvg", &beta_TAvg);
        data_summary_tree->Branch("beta_Corrected_TAvg", &beta_Corrected_TAvg);
        data_summary_tree->Branch("beta_TDiff", &beta_TDiff);
        data_summary_tree->Branch("beta_Corrected_TDiff", &beta_Corrected_TDiff);
        data_summary_tree->Branch("beta_ltrace", &beta_ltrace);
        data_summary_tree->Branch("beta_rtrace", &beta_rtrace);

#endif
    }

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

VandleProcessor::~VandleProcessor() {
#ifdef useroot
    if(SaveRoot) {
        TFile_tree->Write();
        TFile_tree->Close();
    };
    delete (TFile_tree);
#endif
}


void VandleProcessor::DeclarePlots(void) {
    for(set<string>::iterator it = requestedTypes_.begin(); it != requestedTypes_.end(); it++) {
        pair<unsigned int,unsigned int> offset = ReturnOffset(*it);
        DeclareHistogram2D(DD_TQDCBARS + offset.first, SD, S8, "Det Loc vs Trace QDC - Left Even - Right Odd");
        DeclareHistogram2D(DD_MAXIMUMBARS + offset.first, SD, S8,"Det Loc vs Maximum - Left Even - Right Odd");
        DeclareHistogram2D(DD_TIMEDIFFBARS + offset.first, SB, S6, "Bars vs. Time Differences");
        DeclareHistogram2D(DD_TOFBARS + offset.first, SC, S8, "Bar vs. Time of Flight");
        DeclareHistogram2D(DD_CORTOFBARS + offset.first, SC, S8, "Bar vs  Cor Time of Flight");
        DeclareHistogram2D(DD_TQDCAVEVSTOF + offset.first, SC, SD, "<E> vs. TOF(0.5ns/bin)");
        DeclareHistogram2D(DD_TQDCAVEVSCORTOF + offset.first, SC, SD, "<E> vs. CorTOF(0.5ns/bin)");
        DeclareHistogram2D(DD_GAMMAENERGYVSTOF + offset.first, SC, S9, "C-ToF vs. E_gamma");

        DeclareHistogram2D(DD_TIMEDIFFBARS + offset.second, SB, S6, "UNCALIBRATED: Bars vs. Time Differences (0.5ns/bin)");
        DeclareHistogram2D(DD_TOFBARS + offset.second, SC, S8, "UNCALIBRATED: Bar vs. Time of Flight (0.5ns/bin)");
        DeclareHistogram2D(DD_TQDCAVEVSTOF+ offset.second, SC, S8, "UNCALIBRATED: <E> vs. TOF (0.5ns/bin)");
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

            bool caled = ( bar.GetCalibration().GetZ0() != 0 );
            double tof = bar.GetCorTimeAve() - start.GetCorTimeAve() + bar.GetCalibration().GetTofOffset(startLoc);
            double barcorTof = CorrectTOF(tof, bar.GetFlightPath(), bar.GetCalibration().GetZ0());
            double AvgcorTof = CorrectTOF(tof, bar.GetFlightPath(), 100);
            double NCtof = bar.GetCorTimeAve() - start.GetCorTimeAve() ;

            PlotTofHistograms(tof, AvgcorTof,NCtof, bar.GetQdc(), barLoc * numStarts_ + startLoc,
                              ReturnOffset(bar.GetType()),caled);
            if(SaveRoot) {
                FillVandleRoot(bar, tof, AvgcorTof, barcorTof, NCtof, barLoc);
                FillBetaRoot(start, startLoc);
                data_summary_tree->Fill();
            }
        }
}

void VandleProcessor::AnalyzeStarts(const BarDetector &bar, unsigned int &barLoc) {
        for (TimingMap::iterator itStart = starts_.begin(); itStart != starts_.end(); itStart++) {
            if (!(*itStart).second.GetIsValid())
                continue;

            unsigned int startLoc = (*itStart).first.first;
            HighResTimingData start = (*itStart).second;

            bool caled = ( bar.GetCalibration().GetZ0() != 0 );
            double tof = bar.GetCorTimeAve() - start.GetWalkCorrectedTime() + bar.GetCalibration().GetTofOffset(startLoc);
            double barcorTof = CorrectTOF(tof, bar.GetFlightPath(), bar.GetCalibration().GetZ0());
            double AvgcorTof = CorrectTOF(tof, bar.GetFlightPath(), 100);
            double NCtof =bar.GetCorTimeAve() - start.GetWalkCorrectedTime() ;

            PlotTofHistograms(tof, AvgcorTof,NCtof, bar.GetQdc(), barLoc * numStarts_ + startLoc,
                              ReturnOffset(bar.GetType()),caled);

            if(SaveRoot) {
                FillVandleRoot(bar, tof, AvgcorTof, barcorTof, NCtof, barLoc);
                FillBetaRoot(start, startLoc);
                data_summary_tree->Fill();
            }
        }
}

void VandleProcessor::PlotTofHistograms(const double &tof, const double &cortof,const double &NCtof, const double &qdc,
                                        const unsigned int &barPlusStartLoc,
                                        const pair<unsigned int, unsigned int>&offset ,bool &calibrated) {

    plot(DD_TOFBARS + offset.first, tof * plotMult_ + plotOffset_, barPlusStartLoc);
    plot(DD_CORTOFBARS + offset.first, cortof * plotMult_ + plotOffset_, barPlusStartLoc);

    //making these histograms only plot if t cal exists. since no tcal causes strange looking behavior.
    if (calibrated) {
        plot(DD_TQDCAVEVSTOF + offset.first, tof * plotMult_ + plotOffset_, qdc / qdcComp_);
        plot(DD_TQDCAVEVSCORTOF + offset.first, cortof * plotMult_ + plotOffset_, qdc / qdcComp_);
    }
    /*Plotting the Non Time Calibrated Histograms;
    *The ones above would be the same as these when no time calibration is present in the Config.xml
     * This allows for easier timecals since we dont have to rerun the data before we can run timecal.bash
    */
    plot(DD_TOFBARS + offset.second, NCtof* plotMult_ + plotOffset_, barPlusStartLoc);
    plot(DD_TQDCAVEVSTOF + offset.second, NCtof* plotMult_ + plotOffset_, qdc / qdcComp_);



    if (geSummary_) {
        if (geSummary_->GetMult() > 0) {
            const vector<ChanEvent *> &geList = geSummary_->GetList();
            for (vector<ChanEvent *>::const_iterator itGe = geList.begin(); itGe != geList.end(); itGe++)
                plot(DD_GAMMAENERGYVSTOF + offset.first, (*itGe)->GetCalibratedEnergy(), tof);
        } else {
            plot(DD_TQDCAVEVSTOF_VETO + offset.first, tof, qdc / qdcComp_);
            plot(DD_TOFBARS_VETO + offset.first, tof, barPlusStartLoc);
        }
    }
}

void VandleProcessor::FillVandleOnlyHists(void) {
    for (BarMap::const_iterator it = bars_.begin(); it != bars_.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        unsigned int OFFSET = ReturnOffset(barId.second).first;
        unsigned int NOCALOFFSET = ReturnOffset(barId.second).second;
        double NoCalTDiff = (*it).second.GetLeftSide().GetHighResTimeInNs()-(*it).second.GetRightSide().GetHighResTimeInNs();
        
        plot(DD_MAXIMUMBARS + OFFSET,(*it).second.GetLeftSide().GetMaximumValue(), barId.first*2);
        plot(DD_MAXIMUMBARS + OFFSET,(*it).second.GetRightSide().GetMaximumValue(), barId.first*2+1);

        plot(DD_TQDCBARS + OFFSET, (*it).second.GetLeftSide().GetTraceQdc() / qdcComp_, barId.first * 2);
        plot(DD_TQDCBARS + OFFSET, (*it).second.GetRightSide().GetTraceQdc() / qdcComp_, barId.first * 2 + 1);

        plot(DD_TIMEDIFFBARS + OFFSET, (*it).second.GetTimeDifference() * plotMult_ + plotOffset_, barId.first);
        plot(DD_TIMEDIFFBARS + NOCALOFFSET, NoCalTDiff* plotMult_ + plotOffset_, barId.first);
    }
}

std::pair<unsigned int, unsigned int> VandleProcessor::ReturnOffset(const std::string &type) {
    if (type == "small")
        return (make_pair((unsigned int )0,SMALLNOCAL_OFFSET));
    if (type == "big")
        return (make_pair(BIG_OFFSET,BIGNOCALTYPE_OFFSET));
    if (type == "medium")
        return (make_pair(MED_OFFSET,MEDNOCALTYPE_OFFSET));
    return make_pair(numeric_limits<unsigned int>::max(),numeric_limits<unsigned int>::max());
}

void VandleProcessor::FillVandleRoot(const BarDetector &bar, const double &tof,const double &aCortof,
                                     const double &bCortof, const double &NCtof, unsigned int &barNum) {

#ifdef useroot

    vandle_subtype = bar.GetType();
    vandle_lSnR = bar.GetLeftSide().GetTrace().GetSignalToNoiseRatio();
    vandle_rSnR = bar.GetRightSide().GetTrace().GetSignalToNoiseRatio();
    vandle_lAmp = bar.GetLeftSide().GetMaximumValue();
    vandle_rAmp = bar.GetRightSide().GetMaximumValue();
    vandle_lMaxAmpPos = bar.GetLeftSide().GetMaximumPosition();
    vandle_rMaxAmpPos = bar.GetRightSide().GetMaximumPosition();
    vandle_lAveBaseline = bar.GetLeftSide().GetAveBaseline();
    vandle_rAveBaseline = bar.GetRightSide().GetAveBaseline();
    vandle_BarQDC = bar.GetQdc();
    vandle_QDCPos = bar.GetQdcPosition();
    vandle_lQDC = bar.GetLeftSide().GetTraceQdc();
    vandle_rQDC = bar.GetRightSide().GetTraceQdc();
    vandle_TOF = tof;
    vandle_barNum = barNum;
    vandle_TAvg = bar.GetTimeAverage();
    vandle_Corrected_TAvg = bar.GetCorTimeAve();
    vandle_TDiff = bar.GetTimeDifference();
    vandle_Corrected_TDiff = bar.GetCorTimeDiff();
    vandle_ltrace = bar.GetLeftSide().GetTrace();
    vandle_rtrace = bar.GetRightSide().GetTrace();
#endif
}
void VandleProcessor::FillBetaRoot(const HighResTimingData &start, unsigned int &startNum) {
#ifdef useroot
    beta_lSnR = start.GetTrace().GetSignalToNoiseRatio();
    beta_lAmp = start.GetMaximumValue();
    beta_lMaxAmpPos = start.GetMaximumPosition();
    beta_lAveBaseline = start.GetAveBaseline();
    beta_lQDC = start.GetTraceQdc();
    beta_TAvg = start.GetTime();
    beta_Corrected_TAvg = start.GetWalkCorrectedTime();
    beta_ltrace = start.GetTrace();

    beta_barNum = startNum;
    // printf("evtNumber:%d \n",evtNumber);

#endif
}
void VandleProcessor::FillBetaRoot(const BarDetector &start, unsigned int &startNum) {
#ifdef useroot
        beta_lSnR = start.GetLeftSide().GetTrace().GetSignalToNoiseRatio();
        beta_rSnR = start.GetRightSide().GetTrace().GetSignalToNoiseRatio();
        beta_lAmp = start.GetLeftSide().GetMaximumValue();
        beta_rAmp = start.GetRightSide().GetMaximumValue();
        beta_lMaxAmpPos = start.GetLeftSide().GetMaximumPosition();
        beta_rMaxAmpPos = start.GetRightSide().GetMaximumPosition();
        beta_lAveBaseline = start.GetLeftSide().GetAveBaseline();
        beta_rAveBaseline = start.GetRightSide().GetAveBaseline();
        beta_BarQDC = start.GetQdc();
        beta_lQDC = start.GetLeftSide().GetTraceQdc();
        beta_rQDC = start.GetRightSide().GetTraceQdc();
        beta_barNum = startNum;
        beta_TAvg = start.GetTimeAverage();
        beta_Corrected_TAvg = start.GetCorTimeAve();
        beta_TDiff = start.GetTimeDifference();
        beta_Corrected_TDiff = start.GetCorTimeDiff();
        beta_ltrace = start.GetLeftSide().GetTrace();
        beta_rtrace = start.GetRightSide().GetTrace();
// printf("evtNumber:%d \n",evtNumber);

#endif
}