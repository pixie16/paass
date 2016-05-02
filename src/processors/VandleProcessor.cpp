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
#include "GetArguments.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "TimingMapBuilder.hpp"
#include "VandleProcessor.hpp"

namespace dammIds {
    namespace vandle {
        const unsigned int BIG_OFFSET  = 20; //!< Offset for big bars
        const unsigned int MED_OFFSET  = 40;//!< Offset for medium bars
        const unsigned int DEBUGGING_OFFSET = 60;//!< Offset for debugging hists

        const int DD_TQDCBARS         = 0;//!< QDC for the bars
        const int DD_MAXIMUMBARS      = 1;//!< Maximum values for the bars
        const int DD_TIMEDIFFBARS     = 2;//!< time difference in the bars
        const int DD_TOFBARS          = 3;//!< time of flight for the bars
        const int DD_CORTOFBARS       = 4;//!< corrected time of flight
        const int DD_TQDCAVEVSTOF     = 5;//!< Ave QDC vs. ToF
        const int DD_TQDCAVEVSCORTOF  = 6;//!< Ave QDC vs. Cor ToF
        const int DD_CORRELATED_TOF   = 7;//!< ToF Correlated w/ Beam
        const int DD_QDCAVEVSSTARTQDCSUM = 8;//!< Average VANDLE QDC vs. Start QDC Sum
        const int DD_TOFVSSTARTQDCSUM    = 9;//!< ToF vs. Start QDC Sum
        const int DD_GAMMAENERGYVSTOF  = 10;//!< Gamma Energy vs. ToF
        const int DD_TQDCAVEVSTOF_VETO = 11;//!< QDC vs. ToF - Vetoed
        const int DD_TOFBARS_VETO      = 12;//!< ToF - Vetoed

        const int D_DEBUGGING    = 0+DEBUGGING_OFFSET;//!< Debugging countable problems
        const int DD_DEBUGGING   = 1+DEBUGGING_OFFSET;//!< 2D Hist to count problems
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::vandle;

VandleProcessor::VandleProcessor(): 
    EventProcessor(OFFSET, RANGE, "VandleProcessor") {
    associatedTypes.insert("vandle");
}

VandleProcessor::VandleProcessor(const std::vector<std::string> &typeList,
                                 const double &res, const double &offset,
                                 const unsigned int &numStarts):
    EventProcessor(OFFSET, RANGE, "VandleProcessor") {
    associatedTypes.insert("vandle");
    plotMult_ = res;
    plotOffset_ = offset;
    numStarts_ = numStarts;

    hasSmall_ = hasMed_ = hasBig_ = false;
    for(vector<string>::const_iterator it = typeList.begin();
    it != typeList.end(); it++) {
        if((*it) == "small")
            hasSmall_ = true;
        if((*it) == "medium")
            hasMed_ = true;
        if((*it) == "big")
            hasBig_ = true;
    }
    if(typeList.size() == 0)
        hasSmall_ = true;
}

void VandleProcessor::DeclarePlots(void) {
    if(hasSmall_) {
        DeclareHistogram2D(DD_TQDCBARS, SD, S8,
        "Det Loc vs Trace QDC - Left Even - Right Odd");
//        DeclareHistogram2D(DD_MAXIMUMBARS, SD, S8,
//        "Det Loc vs Maximum - Left Even - Right Odd");
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
//        DeclareHistogram2D(DD_CORRELATED_TOF, SC, SC,
//        "Correlated TOF");
//        DeclareHistogram2D(DD_QDCAVEVSSTARTQDCSUM, SC, SD,
//        "<E> VANDLE vs. <E> BETA - SUMMED");
//        DeclareHistogram2D(DD_TOFVSSTARTQDCSUM, SC, SD,
//        "TOF VANDLE vs. <E> BETA - SUMMED");
        DeclareHistogram2D(DD_GAMMAENERGYVSTOF, SC, S9,
        "C-ToF vs. E_gamma");
//        DeclareHistogram2D(DD_TQDCAVEVSTOF_VETO, SC, SD,
//        "<E> VANDLE vs. CorTOF VANDLE - Gamma Veto");
//        DeclareHistogram2D(DD_TOFBARS_VETO, SC, S9,
//        "Bar vs CorTOF - Gamma Veto");
    }
    if(hasBig_) {
        DeclareHistogram2D(DD_TQDCBARS+BIG_OFFSET, SD, S6,
			   "Det Loc vs Trace QDC");
	//        DeclareHistogram2D(DD_MAXIMUMBARS+BIG_OFFSET, SD, S8,
	//        "Det Loc vs Maximum");
        DeclareHistogram2D(DD_TIMEDIFFBARS+BIG_OFFSET, SB, S6,
			   "Bars vs. Time Differences");
        DeclareHistogram2D(DD_TOFBARS+BIG_OFFSET, SC, S6,
			   "Bar vs. Time of Flight");
        DeclareHistogram2D(DD_CORTOFBARS+BIG_OFFSET, SC, S6,
			   "Bar vs  Cor Time of Flight");
        DeclareHistogram2D(DD_TQDCAVEVSTOF+BIG_OFFSET, SC, SD,
			   "<E> vs. TOF(0.5ns/bin)");
        DeclareHistogram2D(DD_TQDCAVEVSCORTOF+BIG_OFFSET, SC, SD,
			   "<E> vs. CorTOF(0.5ns/bin)");
	//        DeclareHistogram2D(DD_CORRELATED_TOF+BIG_OFFSET, SC, SC,
//        "Correlated TOF");
//        DeclareHistogram2D(DD_QDCAVEVSSTARTQDCSUM+BIG_OFFSET, SC, SD,
//        "<E> VANDLE vs. <E> BETA - SUMMED");
//        DeclareHistogram2D(DD_TOFVSSTARTQDCSUM+BIG_OFFSET, SC, SD,
//        "TOF VANDLE vs. <E> BETA - SUMMED");
        DeclareHistogram2D(DD_GAMMAENERGYVSTOF+BIG_OFFSET, SC, S9,
			   "C-ToF vs. E_gamma");
	//        DeclareHistogram2D(DD_TQDCAVEVSTOF_VETO+BIG_OFFSET, SC, SD,
	//        "<E> VANDLE vs. CorTOF VANDLE - Gamma Veto");
	//        DeclareHistogram2D(DD_TOFBARS_VETO+BIG_OFFSET, SC, S9,
	//        "Bar vs CorTOF - Gamma Veto");
    }
    if(hasMed_) {
        DeclareHistogram2D(DD_TQDCBARS+MED_OFFSET, SD, S6,
			   "Det Loc vs Trace QDC");
//        DeclareHistogram2D(DD_MAXIMUMBARS+MED_OFFSET, SD, S8,
//        "Det Loc vs Maximum");
        DeclareHistogram2D(DD_TIMEDIFFBARS+MED_OFFSET, SB, S6,
			   "Bars vs. Time Differences");
        DeclareHistogram2D(DD_TOFBARS+MED_OFFSET, SC, S6,
        "Bar vs. Time of Flight");
        DeclareHistogram2D(DD_CORTOFBARS+MED_OFFSET, SC, S6,
        "Bar vs  Cor Time of Flight");
        DeclareHistogram2D(DD_TQDCAVEVSTOF+MED_OFFSET, SC, SD,
        "<E> vs. TOF(0.5ns/bin)");
        DeclareHistogram2D(DD_TQDCAVEVSCORTOF+MED_OFFSET, SC, SD,
        "<E> vs. CorTOF(0.5ns/bin)");
//        DeclareHistogram2D(DD_CORRELATED_TOF+MED_OFFSET, SC, SC,
//        "Correlated TOF");
//        DeclareHistogram2D(DD_QDCAVEVSSTARTQDCSUM+MED_OFFSET, SC, SD,
//        "<E> VANDLE vs. <E> BETA - SUMMED");
//        DeclareHistogram2D(DD_TOFVSSTARTQDCSUM+MED_OFFSET, SC, SD,
//        "TOF VANDLE vs. <E> BETA - SUMMED");
        DeclareHistogram2D(DD_GAMMAENERGYVSTOF+MED_OFFSET, SC, S9,
        "C-ToF vs. E_gamma");
//        DeclareHistogram2D(DD_TQDCAVEVSTOF_VETO+MED_OFFSET, SC, SD,
//        "<E> VANDLE vs. CorTOF VANDLE - Gamma Veto");
//        DeclareHistogram2D(DD_TOFBARS_VETO+MED_OFFSET, SC, S9,
//        "Bar vs CorTOF - Gamma Veto");
    }

    DeclareHistogram1D(D_DEBUGGING, S5, "1D Debugging");
    DeclareHistogram2D(DD_DEBUGGING, S8, S8, "2D Debugging");
}

bool VandleProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;
    ClearMaps();
    
    static const vector<ChanEvent*> &events =
        event.GetSummary("vandle")->GetList();

    if(events.empty() || events.size() < 2) {
        if(events.empty())
            plot(D_DEBUGGING, 27);
        if(events.size() < 2)
            plot(D_DEBUGGING, 2);
        return(false);
    }

    BarBuilder billy(events);
    billy.BuildBars();
    bars_ = billy.GetBarMap();

    if(bars_.empty()) {
        plot(D_DEBUGGING, 25);
        return(false);
    }

    FillVandleOnlyHists();
    return(true);
}

bool VandleProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);
    plot(D_DEBUGGING, 30);

    //Removing this until it can be updated with the TreeCorrelator
    // hasDecay_ =
    //     (event.GetCorrelator().GetCondition() == Correlator::VALID_DECAY);
    // if(hasDecay_)
    //     decayTime_ = event.GetCorrelator().GetDecayTime() *
    //             Globals::get()->clockInSeconds();

    geSummary_ = event.GetSummary("ge");

    static const vector<ChanEvent*> &betaStarts =
        event.GetSummary("beta_scint:beta")->GetList();
    static const vector<ChanEvent*> &liquidStarts =
        event.GetSummary("liquid:scint:start")->GetList();

    vector<ChanEvent*> startEvents;
    startEvents.insert(startEvents.end(),
		       betaStarts.begin(), betaStarts.end());
    startEvents.insert(startEvents.end(),
		       liquidStarts.begin(), liquidStarts.end());

    TimingMapBuilder bldStarts(startEvents);
    starts_ = bldStarts.GetMap();

    static const vector<ChanEvent*> &doubleBetaStarts =
        event.GetSummary("beta:double:start")->GetList();
    BarBuilder startBars(doubleBetaStarts);
    startBars.BuildBars();
    barStarts_ = startBars.GetBarMap();

    if(!doubleBetaStarts.empty())
        AnalyzeBarStarts();
    else
        AnalyzeStarts();

    EndProcess();
    return(true);
}

void VandleProcessor::AnalyzeBarStarts(void) {
    for (BarMap::iterator it = bars_.begin(); it !=  bars_.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;

        if(!bar.GetHasEvent())
            continue;

        unsigned int histTypeOffset = ReturnOffset(bar.GetType());
        unsigned int barLoc = barId.first;
        const TimingCalibration cal = bar.GetCalibration();

        for(BarMap::iterator itStart = barStarts_.begin();
        itStart != barStarts_.end(); itStart++) {
            unsigned int startLoc = (*itStart).first.first;
            unsigned int barPlusStartLoc = barLoc*numStarts_ + startLoc;

            BarDetector start = (*itStart).second;

            double tof = bar.GetCorTimeAve() -
                start.GetCorTimeAve() + cal.GetTofOffset(startLoc);

            double corTof =
                CorrectTOF(tof, bar.GetFlightPath(), cal.GetZ0());
	    
            plot(DD_TOFBARS+histTypeOffset, tof*plotMult_+plotOffset_,
                 barPlusStartLoc);
            plot(DD_CORTOFBARS, corTof*plotMult_+plotOffset_, barPlusStartLoc);

            if(cal.GetTofOffset(startLoc) != 0) {
                plot(DD_TQDCAVEVSTOF+histTypeOffset, tof*plotMult_+plotOffset_,
                     bar.GetQdc());
                plot(DD_TQDCAVEVSCORTOF+histTypeOffset,
                     corTof*plotMult_+plotOffset_, bar.GetQdc());
            }

            if (geSummary_) {
                if (geSummary_->GetMult() > 0) {
                    const vector<ChanEvent *> &geList = geSummary_->GetList();
                    for (vector<ChanEvent *>::const_iterator itGe = geList.begin();
                        itGe != geList.end(); itGe++) {
                        double calEnergy = (*itGe)->GetCalEnergy();
                        plot(DD_GAMMAENERGYVSTOF+histTypeOffset, calEnergy, tof);
                    }
                } else {
                    plot(DD_TQDCAVEVSTOF_VETO+histTypeOffset, tof, bar.GetQdc());
                    plot(DD_TOFBARS_VETO+histTypeOffset, tof, barPlusStartLoc);
                }
            }
        } // for(TimingMap::iterator itStart
    } //(BarMap::iterator itBar
} //void VandleProcessor::AnalyzeData

void VandleProcessor::AnalyzeStarts(void) {
    for (BarMap::iterator it = bars_.begin(); it !=  bars_.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;

        if(!bar.GetHasEvent())
            continue;

        unsigned int histTypeOffset = ReturnOffset(bar.GetType());
        unsigned int barLoc = barId.first;
        const TimingCalibration cal = bar.GetCalibration();

        for(TimingMap::iterator itStart = starts_.begin();
        itStart != starts_.end(); itStart++) {
            if(!(*itStart).second.GetIsValidData())
                continue;

            unsigned int startLoc = (*itStart).first.first;
            unsigned int barPlusStartLoc = barLoc*numStarts_ + startLoc;
            HighResTimingData start = (*itStart).second;

            double tof = bar.GetCorTimeAve() -
                start.GetCorrectedTime() + cal.GetTofOffset(startLoc);
	    
            double corTof =
                CorrectTOF(tof, bar.GetFlightPath(), cal.GetZ0());

            plot(DD_TOFBARS+histTypeOffset, tof*plotMult_+plotOffset_, barPlusStartLoc);
            plot(DD_TQDCAVEVSTOF+histTypeOffset, tof*plotMult_+plotOffset_, bar.GetQdc());

            plot(DD_CORTOFBARS, corTof*plotMult_+plotOffset_, barPlusStartLoc);
            plot(DD_TQDCAVEVSCORTOF+histTypeOffset, corTof*plotMult_+plotOffset_,
                 bar.GetQdc());

            if (geSummary_) {
                if (geSummary_->GetMult() > 0) {
                    const vector<ChanEvent *> &geList = geSummary_->GetList();
                    for (vector<ChanEvent *>::const_iterator itGe = geList.begin();
                        itGe != geList.end(); itGe++) {
                        double calEnergy = (*itGe)->GetCalEnergy();
                        plot(DD_GAMMAENERGYVSTOF+histTypeOffset, calEnergy, tof);
                    }
                } else {
                    plot(DD_TQDCAVEVSTOF_VETO+histTypeOffset, tof, bar.GetQdc());
                    plot(DD_TOFBARS_VETO+histTypeOffset, tof, barPlusStartLoc);
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
    for(BarMap::const_iterator it = bars_.begin(); it != bars_.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;
        unsigned int OFFSET = ReturnOffset(barId.second);

        plot(DD_TQDCBARS + OFFSET,
             bar.GetLeftSide().GetTraceQdc(), barId.first*2);
        plot(DD_MAXIMUMBARS + OFFSET,
             bar.GetLeftSide().GetMaximumValue(), barId.first*2);
        plot(DD_TQDCBARS + OFFSET,
             bar.GetRightSide().GetTraceQdc(), barId.first*2+1);
        plot(DD_MAXIMUMBARS + OFFSET,
             bar.GetRightSide().GetMaximumValue(), barId.first*2+1);
        plot(DD_TIMEDIFFBARS+OFFSET,
            bar.GetTimeDifference()*plotMult_+plotOffset_, barId.first);
    }
}

unsigned int VandleProcessor::ReturnOffset(const std::string &type) {
    if(type == "small")
        return(0);
    if(type == "big")
        return(BIG_OFFSET);
    if(type == "medium")
        return(MED_OFFSET);
    return(-1);
}
