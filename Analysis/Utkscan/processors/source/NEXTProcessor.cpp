/** \file NEXTProcessor.cpp
 *\brief Processes information for NEXT
 *
 *Processes information from the NEXT Bars, allows for
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
#include "NEXTBuilder.hpp"
#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "RawEvent.hpp"
#include "TimingMapBuilder.hpp"
#include "NEXTProcessor.hpp"

namespace dammIds {
    namespace next {
        const unsigned int DEBUGGING_OFFSET = 60;//!< Offset for debugging hists
        const unsigned int NOCAL_OFFSET = 100;//!< Offset for Uncalibrated hists
        const unsigned int IMAGE_OFFSET = 50;//!< Offest for producing PSPMT images

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
        const int DD_QDCTOF_DECAY = 10;//!< QDC vs. ToF rough decay gated
        const int DD_TDIFF_DECAY = 11;//!< TDIFF rough decay gated
        const int DD_TOFBARS_DECAY = 12;//!< Tof per bar rough decay gated

        const int D_DEBUGGING = 0 + DEBUGGING_OFFSET;//!< Debugging countable problems
        const int DD_DEBUGGING = 1 + DEBUGGING_OFFSET;//!< 2D Hist to count problems
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::next;

NEXTProcessor::NEXTProcessor() : EventProcessor(OFFSET, RANGE, "NEXTProcessor") {
    associatedTypes.insert("next");
}

NEXTProcessor::NEXTProcessor(const std::vector<std::string> &typeList, const double &res, const double &offset,
                                 const unsigned int &numStarts, const double &compression/*=1.0*/,const double &qdcmin,
                                 const double &tofcut, const double &idealFP) :
        EventProcessor(OFFSET,RANGE,"NEXTProcessor") {
    associatedTypes.insert("next");
    plotMult_ = res;
    plotOffset_ = offset;
    numStarts_ = numStarts;
    qdcComp_ = compression;
    qdcmin_ = qdcmin;
    tofcut_ = tofcut;
    idealFP_ = idealFP;

   /* if(typeList.empty())
        requestedTypes_.insert("small");
    else
        requestedTypes_ = set<string>(typeList.begin(), typeList.end()); */
}

void NEXTProcessor::DeclarePlots(void) {
    for(set<string>::iterator it = requestedTypes_.begin(); it != requestedTypes_.end(); it++) {
        pair<unsigned int,unsigned int> offset = ReturnOffset(*it);
        DeclareHistogram2D(DD_TQDCBARS + offset.first, SD, S8, "Det Loc vs Trace QDC - Right Even - Left Odd");
        DeclareHistogram2D(DD_MAXIMUMBARS + offset.first, SD, S8,"Det Loc vs Maximum - Right Even - Left Odd");
        DeclareHistogram2D(DD_TIMEDIFFBARS + offset.first, SB, S6, "Bars vs. Time Differences");
        DeclareHistogram2D(DD_TOFBARS + offset.first, SC, S6, "Bar vs. Time of Flight");
        DeclareHistogram2D(DD_CORTOFBARS + offset.first, SC, S6, "Bar vs  Cor Time of Flight");
        DeclareHistogram2D(DD_TQDCAVEVSTOF + offset.first, SC, SD, "<E> vs. TOF(0.5ns/bin)");
        DeclareHistogram2D(DD_TQDCAVEVSCORTOF + offset.first, SC, SD, "<E> vs. CorTOF(0.5ns/bin)");
        DeclareHistogram2D(DD_GAMMAENERGYVSTOF + offset.first, SC, S9, "C-ToF vs. E_gamma");

        DeclareHistogram2D(DD_TIMEDIFFBARS + offset.second, SB, S7, "UNCALIBRATED: Bars vs. Time Differences (0.5ns/bin)");
        DeclareHistogram2D(DD_TOFBARS + offset.second, SC, S7, "UNCALIBRATED: Bar vs. Time of Flight (0.5ns/bin)");
        DeclareHistogram2D(DD_TQDCAVEVSTOF+ offset.second, SC, SD, "UNCALIBRATED: <E> vs. TOF (0.5ns/bin)");

        DeclareHistogram2D(DD_QDCTOF_DECAY+offset.first,SC,SD, "<E> vs. TOF (0.5ns/bin) Rough Decay Gated ");
        DeclareHistogram2D(DD_TOFBARS_DECAY+offset.first,SC,S6, "Bar vs. TOF (0.5ns/bin) Rough Decay Gated ");
        DeclareHistogram2D(DD_TDIFF_DECAY+offset.first,SC,S6, "Bar vs. TDiff(0.5ns/bin) Rough Decay Gated ");
    }

    DeclareHistogram1D(D_DEBUGGING, S5, "1D Debugging");
    DeclareHistogram2D(DD_DEBUGGING, S8, S8, "2D Debugging");
}

bool NEXTProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    mods_.clear();
    starts_.clear();

    static const vector<ChanEvent *> &dEvents = event.GetSummary("next:dynode")->GetList();
    static const vector<ChanEvent *> &aEvents = event.GetSummary("next:anode")->GetList();

    if (dEvents.empty() || dEvents.size() < 2) {
     /*   if (events.empty())
            plot(D_DEBUGGING, 27);
        if (events.size() < 2)
            plot(D_DEBUGGING, 2);
       */
      return false;
    }

    if (aEvents.empty () || aEvents.size() < 8){
        return false;
    }

    NEXTBuilder sMod(dEvents,aEvents);
    sMod.BuildModule();
    mods_ = sMod.GetNEXTMap();


    if (mods_.empty()) {
     //   plot(D_DEBUGGING, 25);
        return false;
    }

    //FillNEXTOnlyHists();
    return true;
}

bool NEXTProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    plot(D_DEBUGGING, 30);

    geSummary_ = event.GetSummary("clover");

    static const vector<ChanEvent *> &betaStarts = event.GetSummary("beta_scint:beta")->GetList();
    static const vector<ChanEvent *> &liquidStarts = event.GetSummary("liquid:scint:start")->GetList();
    static const vector<ChanEvent *> &pspmtStarts = event.GetSummary("pspmt:dynode_high:start")->GetList();

    static const vector<ChanEvent *> &LIonVeto =  event.GetSummary("pspmt:veto")->GetList();
    static const vector<ChanEvent *> &IondE=  event.GetSummary("pspmt:ion")->GetList();

    vector<ChanEvent *> startEvents;
    startEvents.insert(startEvents.end(), betaStarts.begin(), betaStarts.end());
    startEvents.insert(startEvents.end(), liquidStarts.begin(), liquidStarts.end());
    startEvents.insert(startEvents.end(),pspmtStarts.begin(),pspmtStarts.end());

    TimingMapBuilder bldStarts(startEvents);
    starts_ = bldStarts.GetMap();

    static const vector<ChanEvent *> &doubleBetaStarts = event.GetSummary("beta:double:start")->GetList();
    BarBuilder startBars(doubleBetaStarts);
    startBars.BuildBars();
    barStarts_ = startBars.GetBarMap();

    if (DetectorDriver::get()->GetSysRootOutput()){
        nexts.vMulti = (int)mods_.size();
    }

    RDecay_=false;
    if (LIonVeto.empty() && IondE.empty()){
    RDecay_=true;
    }

    for (NEXTMap::iterator it = mods_.begin(); it != mods_.end(); it++) {
        TimingDefs::TimingIdentifier nextId = (*it).first;
        NEXTDetector mod = (*it).second;

        if (!mod.GetHasEvent())
            continue;

        if (!doubleBetaStarts.empty())
            AnalyzeModStarts(mod, modId.first);
        else
            AnalyzeStarts(mod, modId.first);

    }

    EndProcess();
    return true;
}

void NEXTProcessor::AnalyzeBarStarts(const NEXTDetector &mod, unsigned int &modLoc) {
        for (BarMap::iterator itStart = barStarts_.begin(); itStart != barStarts_.end(); itStart++) {
            unsigned int startLoc = (*itStart).first.first;
            BarDetector start = (*itStart).second;

            bool caled = ( mod.GetCalibration().GetZ0() != 0 );
            double tof = mod.GetCorTimeAve() - start.GetCorTimeAve() + mod.GetCalibration().GetTofOffset(startLoc);
            double corTof = CorrectTOF(tof, mod.GetFlightPath(), idealFP_);
            double NCtof = mod.GetCorTimeAve() - start.GetCorTimeAve() ;

            PlotTofHistograms(tof, corTof,NCtof, mod.GetQdc(), modLoc * numStarts_ + startLoc,
                              ReturnOffset(mod.GetType()),caled);

            if (DetectorDriver::get()->GetSysRootOutput()){
                //Fill Root struct
                nexts.sNum = startLoc;
                nexts.sTime = start.GetTimeAverage();
                nexts.sQdc = start.GetQdc();

                nexts.Zpos = mod.GetAverageZPos();
                nexts.Ypos = mod.GetAverageYPos();
                nexts.qdc = mod.GetQdc();
                nexts.modNum = modLoc;

                nexts.tdiff = mod.GetTimeDifference();
                nexts.tof = tof;
                nexts.corTof = corTof;
                nexts.qdcPos = mod.GetQdcPosition();

                pixie_tree_event_->next_vec_.emplace_back(nexts);
                nexts = processor_struct::NEXTS_DEFAULT_STRUCT;
            }
        }
}

void NEXTProcessor::AnalyzeStarts(const NEXTDetector &mod, unsigned int &modLoc) {
        for (TimingMap::iterator itStart = starts_.begin(); itStart != starts_.end(); itStart++) {
            if (!(*itStart).second.GetIsValid())
                continue;

            unsigned int startLoc = (*itStart).first.first;
            HighResTimingData start = (*itStart).second;

            bool caled = ( mod.GetCalibration().GetZ0() != 0 );
            double tof = mod.GetCorTimeAve() - start.GetWalkCorrectedTime() + mod.GetCalibration().GetTofOffset(startLoc);
            double corTof = CorrectTOF(tof, mod.GetFlightPath(), idealFP_);
            double NCtof =mod.GetCorTimeAve() - start.GetWalkCorrectedTime() ;

            PlotTofHistograms(tof, corTof, NCtof,mod.GetQdc(), modLoc * numStarts_ + startLoc,
                              ReturnOffset(mod.GetType()),caled);
            if (DetectorDriver::get()->GetSysRootOutput()){

                if (tof>= tofcut_ && mod.GetQdc()>qdcmin_) {
                    //Fill Root struct
                    nexts.sNum = startLoc;
                    nexts.sTime = start.GetTimeSansCfd();
                    nexts.sQdc = start.GetTraceQdc();

                    nexts.qdc = mod.GetQdc();
                    nexts.barNum = modLoc;
                    nexts.barType = mod.GetType();
                    nexts.tdiff = mod.GetTimeDifference();
                    nexts.tof = tof;
                    nexts.corTof = corTof;
                    nexts.qdcPos = mod.GetQdcPosition();

                    pixie_tree_event_->next_vec_.emplace_back(nexts);
                    nexts = processor_struct::NEXTS_DEFAULT_STRUCT;
                }
            }
        }
}

void NEXTProcessor::PlotTofHistograms(const double &tof, const double &cortof,const double &NCtof, const double &qdc,
                                        const unsigned int &barPlusStartLoc,
                                        const pair<unsigned int, unsigned int>&offset ,bool &calibrated) {

    //Plotting Normal Histograms
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

    //Plotting Rough Decay Histograms
    if( RDecay_){
        plot(DD_TOFBARS_DECAY + offset.first, tof * plotMult_ + plotOffset_, barPlusStartLoc);
        if (calibrated) //same as before, causes strange behavior when non TCal'd
            plot(DD_QDCTOF_DECAY + offset.first, tof * plotMult_ + plotOffset_, qdc / qdcComp_);
    }


//Plotting OTHER histograms
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

void NEXTProcessor::FillNEXTOnlyHists(void) {
    for (BarMap::const_iterator it = mods_.begin(); it != mods_.end(); it++) {
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

        if (RDecay_)
            plot(DD_TDIFF_DECAY+ OFFSET, (*it).second.GetTimeDifference() * plotMult_ + plotOffset_, barId.first);
    }
}

std::pair<unsigned int, unsigned int> NEXTProcessor::ReturnOffset() {
        return (make_pair(OFFSET,NOCALTYPE_OFFSET));
//    return make_pair(numeric_limits<unsigned int>::max(),numeric_limits<unsigned int>::max());
}
