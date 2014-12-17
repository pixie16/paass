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
    const unsigned int BIG_OFFSET  = 30; //!< Offset for big bars
    const unsigned int MED_OFFSET  = 60;//!< Offset for medium bars
    const unsigned int MISC_OFFSET = 90;//!< Offset for misc. hists
    const unsigned int DEBUGGING_OFFSET = 100;//!< Offset for debugging hists

    namespace vandle {
	const int DD_TQDCBARS         = 0;//!< QDC for the bars
	const int DD_MAXIMUMBARS      = 1;//!< Maximum values for the bars
	const int DD_TIMEDIFFBARS     = 2;//!< time difference in the bars
	const int DD_TOFBARS          = 3;//!< time of flight for the bars
	const int DD_CORTOFBARS       = 5;//!< corrected time of flight
	const int D_TOF               = 6;//!< summed time of flight
	const int DD_TQDCAVEVSTDIFF   = 7;//!< Trace QDC vs. Time Difference

	const int DD_TOFVSTDIFF       = 8;//!< ToF vs. Time Difference
	const int DD_MAXRVSTOF        = 9;//!< Right Max Val vs. ToF
	const int DD_MAXLVSTOF        = 10;//!< Left Max Val vs. ToF
	const int DD_TQDCAVEVSTOF     = 11;//!< QDC Ave vs. ToF

	const int DD_CORTOFVSTDIFF     = 12;//!< Corrected ToF vs. TDiff
	const int DD_MAXRVSCORTOF      = 13;//!< Max Right vs. Cor ToF
	const int DD_MAXLVSCORTOF      = 14;//!< Max Left vs. Cor ToF
	const int DD_TQDCAVEVSCORTOF   = 15;//!< Ave QDC vs. Cor ToF
	const int DD_TQDCAVEVSENERGY   = 16;//!< Ave QDC vs. Energy

	const int DD_CORRELATED_TOF      = 17;//!< ToF Correlated w/ Beam

	const int DD_MAXSTART0VSTOF   = 18;//!< Max Start 1 vs. ToF
	const int DD_MAXSTART1VSTOF   = 19;//!< Max Start 2 vs. ToF
	const int DD_MAXSTART0VSCORTOF = 20;//!< Max Start 1 vs. Corrected ToF
	const int DD_MAXSTART1VSCORTOF = 21;//!< Max Start 2 vs. Corrected ToF
	const int DD_TQDCAVEVSSTARTQDCSUM= 22;//!< Average Vandle QDC vs. Start QDC Sum
	const int DD_TOFVSSTARTQDCSUM    = 23;//!< ToF vs. Start QDC Sum

	const int DD_GAMMAENERGYVSTOF = 24;//!< Gamma Energy vs. ToF
	const int DD_TQDCAVEVSTOF_VETO= 25;//!< QDC vs. ToF - Vetoed
	const int DD_TOFBARS_VETO  = 26;//!< ToF - Vetoed

	const int D_PROBLEMS     = 0+MISC_OFFSET;//!< Histogram to Count Problems
	const int DD_PROBLEMS    = 1+MISC_OFFSET;//!< 2D His to count problems

	const int DD_TOFBARBVSBARA      = 2+MISC_OFFSET;//!< ToF Bar A vs. ToF Bar B
	const int DD_GATEDTQDCAVEVSTOF  = 3+MISC_OFFSET;//!< Gated QDC vs. ToF
	const int D_CROSSTALK           = 4+MISC_OFFSET;//!< Cross talk histogram

	const int DD_DEBUGGING0  = 0+DEBUGGING_OFFSET;//!< Generic for debugging
	const int DD_DEBUGGING1  = 1+DEBUGGING_OFFSET;//!< Generic for debugging
	const int DD_DEBUGGING2  = 2+DEBUGGING_OFFSET;//!< Generic for debugging
	const int DD_DEBUGGING3  = 3+DEBUGGING_OFFSET;//!< Generic for debugging

	const int DD_DEBUGGING4  = 4+DEBUGGING_OFFSET;//!< Generic for debugging
	const int DD_DEBUGGING5  = 5+DEBUGGING_OFFSET;//!< Generic for debugging
	const int DD_DEBUGGING6  = 6+DEBUGGING_OFFSET;//!< Generic for debugging
	const int DD_DEBUGGING7  = 7+DEBUGGING_OFFSET;//!< Generic for debugging
	const int DD_DEBUGGING8  = 8+DEBUGGING_OFFSET;//!< Generic for debugging
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::vandle;

VandleProcessor::VandleProcessor(): EventProcessor(dammIds::vandle::OFFSET,
                                                   dammIds::vandle::RANGE,
                                                   "vandle") {
    associatedTypes.insert("vandle");
}

void VandleProcessor::DeclarePlots(void) {
    bool hasSmall   = true;
    bool hasBig     = false;
    const unsigned int numSmallEnds = S7;
    const unsigned int numBigEnds   = S4;

    //Plots used for debugging
    DeclareHistogram1D(D_PROBLEMS, S5, "1D Debugging");
    DeclareHistogram2D(DD_PROBLEMS, S7, S7, "2D Debugging");

    if(hasSmall) {
       //Plots used for the general information about VANDLE
	DeclareHistogram2D(DD_TQDCBARS, SE, numSmallEnds,
			   "Det Loc vs Trace QDC");
       // DeclareHistogram2D(DD_MAXIMUMBARS, SC, S5,
       // 			  "Det Loc vs. Maximum");
       DeclareHistogram2D(DD_TIMEDIFFBARS, S9, numSmallEnds,
			  "Bars vs. Time Differences");
       DeclareHistogram2D(DD_TOFBARS, SC, numSmallEnds,
			  "Bar vs. Time of Flight");
       DeclareHistogram2D(DD_CORTOFBARS, SC, numSmallEnds,
			  "Bar vs  Cor Time of Flight");
       //DeclareHistogram2D(DD_TQDCAVEVSTDIFF, SC, SE,
       //		  "<E> vs. Time Diff(0.5ns/bin)");
       //Plots related to the TOF
       DeclareHistogram2D(DD_TOFVSTDIFF, S9, SC,
			  "TOF vs. TDiff(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXRVSTOF, SD, SC,
       // 			  "MaxR vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXLVSTOF, SD, SC,
       // 			  "MaxL vs. TOF(0.5ns/bin)");
       DeclareHistogram2D(DD_TQDCAVEVSTOF, SC, SE,
			  "<E> vs. TOF(0.5ns/bin)");
       //Plots related to the corTOF
       DeclareHistogram2D(DD_CORTOFVSTDIFF, S9, SC,
			  "corTOF vs. Tdiff(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXRVSCORTOF, SD, SC,
       // 			  "MaxR vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXLVSCORTOF, SD, SC,
       // 			  "MaxL vs. CorTOF(0.5ns/bin)");
       DeclareHistogram2D(DD_TQDCAVEVSCORTOF, SC, SE,
			  "<E> vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_TQDCAVEVSENERGY, SC, SE,
       // 			  "TQDC vs Energy (kev/bin)");
       //Plots related to Correlated times
       DeclareHistogram2D(DD_CORRELATED_TOF, SC, SC,
			  "Correlated TOF");
       //Plots related to the Starts
       // DeclareHistogram2D(DD_MAXSTART0VSTOF, SD, SC,
       // 			  "Max Start0 vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXSTART1VSTOF, SD, SC,
       // 			  "Max Start1 vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXSTART0VSCORTOF, SD, SC,
       // 			  "Max Start0 vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXSTART1VSCORTOF, SD, SC,
       // 			  "Max Start1 vs. CorTOF(0.5ns/bin)");
       DeclareHistogram2D(DD_TQDCAVEVSSTARTQDCSUM, SC, SE,
			  "<E> VANDLE vs. <E> BETA - SUMMED");
       DeclareHistogram2D(DD_TOFVSSTARTQDCSUM, SC, SE,
			  "TOF VANDLE vs. <E> BETA - SUMMED");

       //Plots related to the Ge detectors
       DeclareHistogram2D(DD_GAMMAENERGYVSTOF, SC, S9,
			  "GAMMA ENERGY vs. CorTOF VANDLE");
       DeclareHistogram2D(DD_TQDCAVEVSTOF_VETO, SC, SE,
			  "<E> VANDLE vs. CorTOF VANDLE - Gamma Veto");
       DeclareHistogram2D(DD_TOFBARS_VETO, SC, S9,
			  "Bar vs CorTOF - Gamma Veto");
    }//if (hasSmall)

    if(hasBig) {
       //Plots used for the general information about VANDLE
       DeclareHistogram2D(DD_TQDCBARS+dammIds::BIG_OFFSET, SE, numBigEnds,
			  "Det Loc vs Trace QDC");
       // DeclareHistogram2D(DD_MAXIMUMBARS+dammIds::BIG_OFFSET, SC, numBigEnds,
       // 			  "Det Loc vs. Maximum");
       // DeclareHistogram2D(DD_TIMEDIFFBARS+dammIds::BIG_OFFSET, S9, numBigEnds,
       // 			  "Bars vs. Time Differences");
       // DeclareHistogram2D(DD_TOFBARS+dammIds::BIG_OFFSET, SC, numBigEnds,
       // 			  "Bar vs. Time of Flight");
       // DeclareHistogram2D(DD_CORTOFBARS+dammIds::BIG_OFFSET, SC, numBigEnds,
       // 			  "Bar vs  Cor Time of Flight");
       // DeclareHistogram2D(DD_TQDCAVEVSTDIFF+dammIds::BIG_OFFSET, SC, SD,
       // 			  "<E> vs. Time Diff(0.5ns/bin)");
       //Plots related to the TOF
       // DeclareHistogram2D(DD_TOFVSTDIFF+dammIds::BIG_OFFSET, S9, SC,
       // 			  "TDiff vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXRVSTOF+dammIds::BIG_OFFSET, SD, SC,
       // 			  "MaxR vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXLVSTOF+dammIds::BIG_OFFSET, SD, SC,
       // 			  "MaxL vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_TQDCAVEVSTOF+dammIds::BIG_OFFSET, SC, SD,
       // 			  "<E> vs. TOF(0.5ns/bin)");
       //Plots related to the corTOF
       // DeclareHistogram2D(DD_CORTOFVSTDIFF+dammIds::BIG_OFFSET, S9, SC,
       // 			  "TDiff vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXRVSCORTOF+dammIds::BIG_OFFSET, SD, SC,
       // 			  "MaxR vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXLVSCORTOF+dammIds::BIG_OFFSET, SD, SC,
       // 			  "MaxL vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_TQDCAVEVSCORTOF+dammIds::BIG_OFFSET, SC, SD,
       // 			  "<E> vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_TQDCAVEVSENERGY+dammIds::BIG_OFFSET, SC, SD,
       // 			  "TQDC vs Energy (kev/bin)");
       //Plots related to Correlated times
       // DeclareHistogram2D(DD_CORRELATED_TOF+dammIds::BIG_OFFSET, SC, SC,
       // 			  "Correlated TOF");
       //Plots related to the Starts
       // DeclareHistogram2D(DD_MAXSTART0VSTOF+dammIds::BIG_OFFSET, SD, SC,
       // 			  "Max Start0 vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXSTART1VSTOF+dammIds::BIG_OFFSET, SD, SC,
       // 			  "Max Start1 vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXSTART0VSCORTOF+dammIds::BIG_OFFSET, SD, SC,
       // 			  "Max Start0 vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXSTART1VSCORTOF+dammIds::BIG_OFFSET, SD, SC,
       // 			  "Max Start1 vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_TQDCAVEVSSTARTQDCSUM+dammIds::BIG_OFFSET,
       // 			  SC, SD, "<E> VANDLE vs. <E> BETA - SUMMED");
       // DeclareHistogram2D(DD_TOFVSSTARTQDCSUM+dammIds::BIG_OFFSET, SC, SD,
       // 			  "TOF VANDLE vs. <E> BETA - SUMMED");
       //Plots related to the Ge detectors
       // DeclareHistogram2D(DD_GAMMAENERGYVSTOF+dammIds::BIG_OFFSET,
       // 			  SC, S9, "GAMMA ENERGY vs. CorTOF VANDLE");
       // DeclareHistogram2D(DD_TQDCAVEVSTOF_VETO+dammIds::BIG_OFFSET, SC, SD,
       // 			  "<E> VANDLE vs. CorTOF VANDLE - Gamma Veto");
       // DeclareHistogram2D(DD_TOFBARS_VETO+dammIds::BIG_OFFSET, SC, S9,
       // 			  "Bar vs CorTOF - Gamma Veto");
    }//if (hasBig)

    //Debugging histograms - The titles do not necessarily reflect the contents
    DeclareHistogram2D(DD_DEBUGGING0, SA, SA, "TOFL vs. TDIFF");
    DeclareHistogram2D(DD_DEBUGGING1, S9, SD, "TOFR vs. TDIFF");
    DeclareHistogram2D(DD_DEBUGGING2, SD, SD, "CorTOF vs. TDIFF");
    // DeclareHistogram2D(DD_DEBUGGING3, S9, SC, "TestTOF vs. TDIFF");

    DeclareHistogram2D(DD_DEBUGGING4, S9, SC, "TOFL vs. QDCRATIO");
    DeclareHistogram2D(DD_DEBUGGING5, SC, SC, "TOFR vs. QDCRATIO");
    DeclareHistogram2D(DD_DEBUGGING6, SC, SC, "TOF vs. QDCRATIO");
    DeclareHistogram2D(DD_DEBUGGING7, SC, SC, "CorTOF vs. QDCRATIO");
    DeclareHistogram2D(DD_DEBUGGING8, SC, SC, "testTOF vs. QDCRATIO");

    //Histograms for the CrossTalk Subroutine
    //DeclareHistogram1D(D_CROSSTALK, SC, "CrossTalk Between Two Bars");
    //DeclareHistogram2D(DD_GATEDTQDCAVEVSTOF, SC, SD,
    //"<E> vs. TOF0 (0.5ns/bin) - Gated");
    //DeclareHistogram2D(DD_TOFBARBVSBARA, SC, SC, "TOF Bar1 vs. Bar2");
    //DeclareHistogram2D(, S8, S8, "tdiffA vs. tdiffB");
    //DeclareHistogram1D(, SD, "Muons");
    //DeclareHistogram2D(, S8, S8, "tdiffA vs. tdiffB");
}// Declare Plots

bool VandleProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    ClearMaps();

    static const vector<ChanEvent*> &events =
        event.GetSummary("vandle")->GetList();

    if(events.empty()) {
        plot(D_PROBLEMS, 27);
        return(false);
    }

    BarBuilder billy(events);
    bars_ = billy.GetBarMap();

    if(bars_.empty()) {
        plot(D_PROBLEMS, 25);
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
    plot(D_PROBLEMS, 30);

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

        unsigned int idOffset = -1;

        if(barId.second == "small")
            idOffset = 0;
        else if(barId.second == "big")
            idOffset = dammIds::BIG_OFFSET;

        const int resMult = 2;
        const int resOffset = 200;
        unsigned int barLoc = barId.first;

        TimingCalibration cal = bar.GetCalibration();

        plot(DD_DEBUGGING0, bar.GetQdcPosition()*resMult+resOffset,
            bar.GetTimeDifference()*resMult+resOffset);
        plot(DD_TIMEDIFFBARS+idOffset,
            bar.GetTimeDifference()*resMult+resOffset, barLoc);
        plot(DD_TQDCAVEVSTDIFF+idOffset,
            bar.GetTimeDifference()*resMult+resOffset, bar.GetQdc());

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

            plot(DD_TOFBARS+idOffset, TOF*resMult+resOffset, barPlusStartLoc);
            plot(DD_TOFVSTDIFF+idOffset, bar.GetTimeDifference()*resMult+resOffset,
                 TOF*resMult+resOffset);
            plot(DD_MAXRVSTOF+idOffset, TOF*resMult+resOffset,
                 bar.GetRightSide().GetMaximumValue());
            plot(DD_MAXLVSTOF+idOffset, TOF*resMult+resOffset,
                 bar.GetLeftSide().GetMaximumValue());
            plot(DD_TQDCAVEVSTOF+idOffset, TOF*resMult+resOffset, bar.GetQdc());

            plot(DD_CORTOFBARS, corTOF*resMult+resOffset, barPlusStartLoc);
            plot(DD_CORTOFVSTDIFF+idOffset, bar.GetTimeDifference()*resMult + resOffset,
                 corTOF*resMult+resOffset);
            plot(DD_MAXRVSCORTOF+idOffset, corTOF*resMult+resOffset,
                 bar.GetRightSide().GetMaximumValue());
            plot(DD_MAXLVSCORTOF+idOffset, corTOF*resMult+resOffset,
                 bar.GetLeftSide().GetMaximumValue());
            plot(DD_TQDCAVEVSCORTOF+idOffset, corTOF*resMult+resOffset,
                 bar.GetQdc());

            if(startLoc == 0) {
                plot(DD_MAXSTART0VSTOF+idOffset,
                    TOF*resMult+resOffset, start.GetMaximumValue());
                plot(DD_MAXSTART0VSCORTOF+idOffset,
                    corTOF*resMult+resOffset, start.GetMaximumValue());
            } else if (startLoc == 1) {
                plot(DD_MAXSTART1VSCORTOF+idOffset,
                    corTOF*resMult+resOffset, start.GetMaximumValue());
                plot(DD_MAXSTART1VSCORTOF+idOffset,
                    corTOF*resMult+resOffset, start.GetMaximumValue());
            }

            if (geSummary_) {
                if (geSummary_->GetMult() > 0) {
                    const vector<ChanEvent *> &geList = geSummary_->GetList();
                    for (vector<ChanEvent *>::const_iterator itGe = geList.begin();
                        itGe != geList.end(); itGe++) {
                        double calEnergy = (*itGe)->GetCalEnergy();
                        plot(DD_GAMMAENERGYVSTOF+idOffset, TOF, calEnergy);
                    }
                } else {
                    plot(DD_TQDCAVEVSTOF_VETO+idOffset, TOF, bar.GetQdc());
                    plot(DD_TOFBARS_VETO+idOffset, TOF, barPlusStartLoc);
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
            OFFSET = dammIds::BIG_OFFSET;
        else if(type == "medium")
            OFFSET = dammIds::MED_OFFSET;
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

/*
void VandleProcessor::CrossTalk(void) {
    //This whole routine is stupidly written, it needs cleaned up something fierce.
    if(barMap.size() < 2)
        return;

    for(BarMap::iterator itBarA = barMap.begin();
    itBarA != barMap.end(); itBarA++) {
            BarMap::iterator itTemp = itBarA;
            itTemp++;

            const double &timeAveA = (*itBarA).second.timeAve;
            const unsigned int &locA = (*itBarA).first.first;

            for(BarMap::iterator itBarB = itTemp; itBarB != barMap.end();
            itBarB++) {
                    if((*itBarA).first.second != (*itBarB).first.second)
                        continue;
                    if((*itBarA).first == (*itBarB).first)
                        continue;

                    const double &timeAveB = (*itBarB).second.timeAve;
                    const unsigned int &locB = (*itBarB).first.first;

                    CrossTalkKey bars(locA, locB);
                    crossTalk.insert(make_pair(bars, timeAveB - timeAveA));
            }
    }

    string barType = "small";
    TimingDefs::BarIdentifier barA(0, barType);
    TimingDefs::BarIdentifier barB(1, barType);

    CrossTalkKey barsOfInterest(barA.first, barB.first);

    CrossTalkMap::iterator itBars =
        crossTalk.find(barsOfInterest);

    const int resMult = 2; //!<set resolution of histograms
    const int resOffset = 200; //!< set offset of histograms

    if(itBars != crossTalk.end())
        plot(D_CROSSTALK, (*itBars).second * resMult + resOffset);

    BarMap::iterator itBarA = barMap.find(barA);
    BarMap::iterator itBarB = barMap.find(barB);

    if(itBarA == barMap.end() || itBarB == barMap.end())
        return;

//    TimeOfFlightMap::iterator itTofA =
//  (*itBarA).second.timeOfFlight.find(startLoc);
//     TimeOfFlightMap::iterator itTofB =
//  (*itBarB).second.timeOfFlight.find(startLoc);

//     if(itTofA == (*itBarA).second.timeOfFlight.end() ||
//        itTofB == (*itBarB).second.timeOfFlight.end())
//  return;

//     double tofA = (*itTofA).second;
//     double tofB = (*itTofB).second;
    double tdiffA = (*itBarA).second.walkCorTimeDiff;
    double tdiffB = (*itBarB).second.walkCorTimeDiff;
    double qdcA = (*itBarA).second.qdc;
    double qdcB = (*itBarB).second.qdc;

    //bool onBar = (tdiffA + tdiffB <= 0.75 && tdiffA + tdiffB >= 0.25);
    bool muon = (qdcA > 7500 && qdcB > 7500);

    double muonTOF =
        (*itBarA).second.timeAve - (*itBarB).second.timeAve;

    plot(3950, tdiffA*resMult+100, tdiffB*resMult+100);

    if(muon) {
            plot(3951, tdiffA*resMult+100, tdiffB*resMult+100);
            plot(3952, muonTOF*resMult*10 + resOffset);
    }

//     plot(DD_TOFBARBVSBARA, tofA*resMult+resOffset,
//       tofB*resMult+resOffset);

//     if((tofB > tofA) && (tofB < (tofA+150))) {
//      plot(DD_GATEDTQDCAVEVSTOF, tofA*resMult+resOffset,
//           (*itBarA).second.qdc);
//     }
}
*/
