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

#include "BarDetector.hpp"
#include "BarBuilder.hpp"
#include "DammPlotIds.hpp"
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
    const int DD_TQDCAVEVSTOF     = 5;//!< QDC Ave vs. ToF
	const int DD_TQDCAVEVSCORTOF  = 6;//!< Ave QDC vs. Cor ToF
	const int DD_CORRELATED_TOF   = 7;//!< ToF Correlated w/ Beam
	const int DD_QDCAVEVSSTARTQDCSUM = 8;//!< Average VANDLE QDC vs. Start QDC Sum
	const int DD_TOFVSSTARTQDCSUM    = 9;//!< ToF vs. Start QDC Sum
	const int DD_GAMMAENERGYVSTOF  = 10;//!< Gamma Energy vs. ToF
	const int DD_TQDCAVEVSTOF_VETO = 11;//!< QDC vs. ToF - Vetoed
	const int DD_TOFBARS_VETO      = 12;//!< ToF - Vetoed

	const int D_DEBUGGING    = 0+DEBUGGING_OFFSET;//!< Debugging countable problems
	const int DD_DEBUGGING   = 1+DEBUGGING_OFFSET;//!< 2D Hist to count problems
	const int DD_DEBUGGING0  = 2+DEBUGGING_OFFSET;//!< Generic for debugging
	const int DD_DEBUGGING1  = 3+DEBUGGING_OFFSET;//!< Generic for debugging
	const int DD_DEBUGGING2  = 4+DEBUGGING_OFFSET;//!< Generic for debugging
	const int DD_DEBUGGING3  = 5+DEBUGGING_OFFSET;//!< Generic for debugging
	const int DD_DEBUGGING4  = 6+DEBUGGING_OFFSET;//!< Generic for debugging
	const int DD_DEBUGGING5  = 7+DEBUGGING_OFFSET;//!< Generic for debugging
	const int DD_DEBUGGING6  = 8+DEBUGGING_OFFSET;//!< Generic for debugging
	const int DD_DEBUGGING7  = 9+DEBUGGING_OFFSET;//!< Generic for debugging
	const int DD_DEBUGGING8  = 10+DEBUGGING_OFFSET;//!< Generic for debugging
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::vandle;

VandleProcessor::VandleProcessor(): EventProcessor(dammIds::vandle::OFFSET,
                                                   dammIds::vandle::RANGE,
                                                   "vandle") {
    associatedTypes.insert("vandle");
}

VandleProcessor::VandleProcessor(const std::vector<std::string> &typeList):
    EventProcessor(dammIds::vandle::OFFSET, dammIds::vandle::RANGE, "vandle") {
    associatedTypes.insert("vandle");
    hasSmall_ = hasMed_ = hasBig_ = false;
    for(vector<string>::const_iterator it = typeList.begin();
    it != typeList.end(); it++) {
        string type = (*it);
        if(type == "small")
            hasSmall_ = true;
        if(type == "medium")
            hasMed_ = true;
        if(type == "big")
            hasBig_ = true;
    }
}

void VandleProcessor::DeclarePlots(void) {
    if(hasSmall_) {
        DeclareHistogram2D(DD_TQDCBARS, SD, S7,
        "Det Loc vs Trace QDC");
//        DeclareHistogram2D(DD_MAXIMUMBARS, SD, S7,
//        "Det Loc vs Maximum");
        DeclareHistogram2D(DD_TIMEDIFFBARS, S9, S7,
        "Bars vs. Time Differences");
        DeclareHistogram2D(DD_TOFBARS, SC, S7,
        "Bar vs. Time of Flight");
        DeclareHistogram2D(DD_CORTOFBARS, SC, S7,
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
        DeclareHistogram2D(DD_TQDCBARS+BIG_OFFSET, SD, S7,
        "Det Loc vs Trace QDC");
//        DeclareHistogram2D(DD_MAXIMUMBARS+BIG_OFFSET, SD, S7,
//        "Det Loc vs Maximum");
        DeclareHistogram2D(DD_TIMEDIFFBARS+BIG_OFFSET, S9, S7,
        "Bars vs. Time Differences");
        DeclareHistogram2D(DD_TOFBARS+BIG_OFFSET, SC, S7,
        "Bar vs. Time of Flight");
        DeclareHistogram2D(DD_CORTOFBARS+BIG_OFFSET, SC, S7,
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
        DeclareHistogram2D(DD_TQDCBARS+MED_OFFSET, SD, S7,
        "Det Loc vs Trace QDC");
//        DeclareHistogram2D(DD_MAXIMUMBARS+MED_OFFSET, SD, S7,
//        "Det Loc vs Maximum");
        DeclareHistogram2D(DD_TIMEDIFFBARS+MED_OFFSET, S9, S7,
        "Bars vs. Time Differences");
        DeclareHistogram2D(DD_TOFBARS+MED_OFFSET, SC, S7,
        "Bar vs. Time of Flight");
        DeclareHistogram2D(DD_CORTOFBARS+MED_OFFSET, SC, S7,
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
    DeclareHistogram2D(DD_DEBUGGING, S7, S7, "2D Debugging");
//    DeclareHistogram2D(DD_DEBUGGING0, SA, SA, "TOFL vs. TDIFF");
//    DeclareHistogram2D(DD_DEBUGGING1, S9, SD, "TOFR vs. TDIFF");
//    DeclareHistogram2D(DD_DEBUGGING2, SD, SD, "CorTOF vs. TDIFF");
//    DeclareHistogram2D(DD_DEBUGGING4, S9, SC, "TOFL vs. QDCRATIO");
//    DeclareHistogram2D(DD_DEBUGGING5, SC, SC, "TOFR vs. QDCRATIO");
//    DeclareHistogram2D(DD_DEBUGGING6, SC, SC, "TOF vs. QDCRATIO");
//    DeclareHistogram2D(DD_DEBUGGING7, SC, SC, "CorTOF vs. QDCRATIO");
//    DeclareHistogram2D(DD_DEBUGGING8, SC, SC, "testTOF vs. QDCRATIO");
}

bool VandleProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    ClearMaps();

    static const vector<ChanEvent*> &events =
        event.GetSummary("vandle")->GetList();

    if(events.empty()) {
        plot(D_DEBUGGING, 27);
        return(false);
    }

    BarBuilder billy(events);
    bars_ = billy.GetBarMap();

    if(bars_.empty()) {
        plot(D_DEBUGGING, 25);
        return(false);
    }

    FillBasicHists();
    return(true);
}

bool VandleProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);

    hasDecay_ =
        (event.GetCorrelator().GetCondition() == Correlator::VALID_DECAY);
    if(hasDecay_)
        decayTime_ = event.GetCorrelator().GetDecayTime() *
                Globals::get()->clockInSeconds();
    plot(D_DEBUGGING, 30);

    static const vector<ChanEvent*> &betaStarts =
        event.GetSummary("beta_scint:beta:start")->GetList();
    static const vector<ChanEvent*> &liquidStarts =
        event.GetSummary("liquid_scint:liquid:start")->GetList();

    geSummary_ = event.GetSummary("ge");

    vector<ChanEvent*> startEvents;
    startEvents.insert(startEvents.end(),
		       betaStarts.begin(), betaStarts.end());
    startEvents.insert(startEvents.end(),
		       liquidStarts.begin(), liquidStarts.end());

    TimingMapBuilder bldStarts(startEvents);
    starts_ = bldStarts.GetMap();

    AnalyzeData();

    EndProcess();
    return(true);
}

void VandleProcessor::AnalyzeData(void) {
    for (BarMap::iterator it = bars_.begin(); it !=  bars_.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;

        if(!bar.GetHasEvent())
            continue;

        unsigned int histTypeOffset = -1;
        if(barId.second == "small")
            histTypeOffset = 0;
        if(barId.second == "big")
            histTypeOffset = BIG_OFFSET;
        if(barId.second == "medium")
            histTypeOffset = MED_OFFSET;

        const int resMult = 2;
        const int resOffset = 200;
        unsigned int barLoc = barId.first;

        TimingCalibration cal = bar.GetCalibration();

        plot(DD_DEBUGGING0, bar.GetQdcPosition()*resMult+resOffset,
            bar.GetTimeDifference()*resMult+resOffset);
        plot(DD_TIMEDIFFBARS+histTypeOffset,
            bar.GetTimeDifference()*resMult+resOffset, barLoc);

        for(TimingMap::iterator itStart = starts_.begin();
        itStart != starts_.end(); itStart++) {
            if(!(*itStart).second.GetIsValidData())
                continue;

            unsigned int startLoc = (*itStart).first.first;
            unsigned int barPlusStartLoc = barLoc*2 + startLoc;
            HighResTimingData start = (*itStart).second;

            double tofOffset;
            if(startLoc == 0)
                tofOffset = cal.GetTofOffset0();
            else
                tofOffset = cal.GetTofOffset1();

            double TOF = bar.GetWalkCorTimeAve() -
                start.GetWalkCorrectedTime() + tofOffset;

            double corTOF =
                CorrectTOF(TOF, bar.GetFlightPath(), cal.GetZ0());

            plot(DD_TOFBARS+histTypeOffset, TOF*resMult+resOffset, barPlusStartLoc);
            plot(DD_TQDCAVEVSTOF+histTypeOffset, TOF*resMult+resOffset, bar.GetQdc());

            plot(DD_CORTOFBARS, corTOF*resMult+resOffset, barPlusStartLoc);
            plot(DD_TQDCAVEVSCORTOF+histTypeOffset, corTOF*resMult+resOffset,
                 bar.GetQdc());

            if (geSummary_) {
                if (geSummary_->GetMult() > 0) {
                    const vector<ChanEvent *> &geList = geSummary_->GetList();
                    for (vector<ChanEvent *>::const_iterator itGe = geList.begin();
                        itGe != geList.end(); itGe++) {
                        double calEnergy = (*itGe)->GetCalEnergy();
                        plot(DD_GAMMAENERGYVSTOF+histTypeOffset, calEnergy, TOF);
                    }
                } else {
                    plot(DD_TQDCAVEVSTOF_VETO+histTypeOffset, TOF, bar.GetQdc());
                    plot(DD_TOFBARS_VETO+histTypeOffset, TOF, barPlusStartLoc);
                }
            }
        } // for(TimingMap::iterator itStart
    } //(BarMap::iterator itBar
} //void VandleProcessor::AnalyzeData

void VandleProcessor::ClearMaps(void) {
    bars_.clear();
    starts_.clear();
}

void VandleProcessor::FillBasicHists(void) {
    unsigned int OFFSET;
    for(BarMap::const_iterator it = bars_.begin(); it != bars_.end(); it++) {
        string type = (*it).first.second;
        if(type == "big")
            OFFSET = BIG_OFFSET;
        else if(type == "medium")
            OFFSET = MED_OFFSET;
        else
            OFFSET = 0;
        unsigned int num = (*it).first.first;
        plot(DD_TQDCBARS + OFFSET,
             (*it).second.GetLeftSide().GetTraceQdc(), num * 2);
        plot(DD_MAXIMUMBARS + OFFSET,
             (*it).second.GetLeftSide().GetMaximumValue(), num * 2);
        plot(DD_TQDCBARS + OFFSET,
             (*it).second.GetRightSide().GetTraceQdc(), num * 2 + 1);
        plot(DD_MAXIMUMBARS + OFFSET,
             (*it).second.GetRightSide().GetMaximumValue(), num * 2 + 1);
    }
}
