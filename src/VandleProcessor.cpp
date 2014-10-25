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

#include "DammPlotIds.hpp"
#include "GetArguments.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "VandleProcessor.hpp"

namespace dammIds {
    const unsigned int BIG_OFFSET      = 30;
    const unsigned int MISC_OFFSET     = 60;
    const unsigned int TVANDLE_OFFSET  = 70;
    const unsigned int DEBUGGING_OFFSET = 100;

    namespace vandle {
	//Plots for the general information about VANDLE
	const int DD_TQDCBARS         = 0;
	const int DD_MAXIMUMBARS      = 1;
	const int DD_TIMEDIFFBARS     = 2;
	const int DD_TOFBARS          = 3;
	const int DD_CORTOFBARS       = 5;
	const int D_TOF               = 6;
	const int DD_TQDCAVEVSTDIFF   = 7;

	//Plots related to the TOF

	const int DD_TOFVSTDIFF       = 8;
	const int DD_MAXRVSTOF        = 9;
	const int DD_MAXLVSTOF        = 10;
	const int DD_TQDCAVEVSTOF     = 11;

	//Plots related to the CorTOF
	const int DD_CORTOFVSTDIFF     = 12;
	const int DD_MAXRVSCORTOF      = 13;
	const int DD_MAXLVSCORTOF      = 14;
	const int DD_TQDCAVEVSCORTOF   = 15;
	const int DD_TQDCAVEVSENERGY   = 16;

	//Plots related to correlated times
	const int DD_CORRELATED_TOF      = 17;

	//Plots related to the Start detectors
	const int DD_MAXSTART0VSTOF   = 18;
	const int DD_MAXSTART1VSTOF   = 19;
	const int DD_MAXSTART0VSCORTOF = 20;
	const int DD_MAXSTART1VSCORTOF = 21;
	const int DD_TQDCAVEVSSTARTQDCSUM= 22;
	const int DD_TOFVSSTARTQDCSUM    = 23;

	//Plots related to the Ge detectors
	const int DD_GAMMAENERGYVSTOF = 24;
	const int DD_TQDCAVEVSTOF_VETO= 25;
	const int DD_TOFBARS_VETO  = 26;

	//Plots used for debugging
	const int D_PROBLEMS     = 0+MISC_OFFSET;
	const int DD_PROBLEMS    = 1+MISC_OFFSET;
	//CrossTalk Subroutine
	const int DD_TOFBARBVSBARA      = 2+MISC_OFFSET;
	const int DD_GATEDTQDCAVEVSTOF  = 3+MISC_OFFSET;
	const int D_CROSSTALK           = 4+MISC_OFFSET;

	const int DD_DEBUGGING0  = 0+DEBUGGING_OFFSET;
	const int DD_DEBUGGING1  = 1+DEBUGGING_OFFSET;
	const int DD_DEBUGGING2  = 2+DEBUGGING_OFFSET;
	const int DD_DEBUGGING3  = 3+DEBUGGING_OFFSET;

	const int DD_DEBUGGING4  = 4+DEBUGGING_OFFSET;
	const int DD_DEBUGGING5  = 5+DEBUGGING_OFFSET;
	const int DD_DEBUGGING6  = 6+DEBUGGING_OFFSET;
	const int DD_DEBUGGING7  = 7+DEBUGGING_OFFSET;
	const int DD_DEBUGGING8  = 8+DEBUGGING_OFFSET;
    }//namespace vandle

    namespace tvandle {
	const int D_TIMEDIFF         = 2  + TVANDLE_OFFSET;
	const int DD_PVSP            = 3  + TVANDLE_OFFSET;
	const int DD_MAXRIGHTVSTDIFF = 4  + TVANDLE_OFFSET;
	const int DD_MAXLEFTVSTDIFF  = 5  + TVANDLE_OFFSET;
	const int DD_MAXLVSTDIFFGATE = 6  + TVANDLE_OFFSET;
	const int DD_MAXLVSTDIFFAMP  = 7  + TVANDLE_OFFSET;
	const int DD_MAXLCORGATE     = 8  + TVANDLE_OFFSET;
	const int DD_QDCVSMAX        = 9  + TVANDLE_OFFSET;
	const int DD_SNRANDSDEV      = 10 + TVANDLE_OFFSET;
    }// namespace tvandle
}//namespace dammIds

using namespace std;
using namespace dammIds::vandle;

VandleProcessor::VandleProcessor():
    EventProcessor(dammIds::vandle::OFFSET, dammIds::vandle::RANGE, "vandle") {
    associatedTypes.insert("vandleSmall");
    associatedTypes.insert("vandleBig");
    associatedTypes.insert("tvandle");
}

void VandleProcessor::DeclarePlots(void) {
    bool hasSmall   = true;
    bool hasBig     = false;
    bool hasTvandle = false;
    const unsigned int numSmallEnds = S7;
    const unsigned int numBigEnds   = S4;

    //Plots used for debugging
    DeclareHistogram1D(D_PROBLEMS, S5, "1D Debugging");
    DeclareHistogram2D(DD_PROBLEMS, S7, S7, "2D Debugging");

    if(hasSmall) {
       //Plots used for the general information about VANDLE
	DeclareHistogram2D(DD_TQDCBARS, SD, numSmallEnds,
			   "Det Loc vs Trace QDC");
       // DeclareHistogram2D(DD_MAXIMUMBARS, SC, S5,
       // 			  "Det Loc vs. Maximum");
       DeclareHistogram2D(DD_TIMEDIFFBARS, S9, numSmallEnds,
			  "Bars vs. Time Differences");
       DeclareHistogram2D(DD_TOFBARS, SC, numSmallEnds,
			  "Bar vs. Time of Flight");
       DeclareHistogram2D(DD_CORTOFBARS, SC, numSmallEnds,
			  "Bar vs  Cor Time of Flight");
       //DeclareHistogram2D(DD_TQDCAVEVSTDIFF, SC, SD,
       //		  "<E> vs. Time Diff(0.5ns/bin)");
       //Plots related to the TOF
       DeclareHistogram2D(DD_TOFVSTDIFF, S9, SC,
			  "TOF vs. TDiff(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXRVSTOF, SD, SC,
       // 			  "MaxR vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXLVSTOF, SD, SC,
       // 			  "MaxL vs. TOF(0.5ns/bin)");
       DeclareHistogram2D(DD_TQDCAVEVSTOF, SC, SD,
			  "<E> vs. TOF(0.5ns/bin)");
       //Plots related to the corTOF
       DeclareHistogram2D(DD_CORTOFVSTDIFF, S9, SC,
			  "corTOF vs. Tdiff(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXRVSCORTOF, SD, SC,
       // 			  "MaxR vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXLVSCORTOF, SD, SC,
       // 			  "MaxL vs. CorTOF(0.5ns/bin)");
       DeclareHistogram2D(DD_TQDCAVEVSCORTOF, SC, SD,
			  "<E> vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_TQDCAVEVSENERGY, SC, SD,
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
       DeclareHistogram2D(DD_TQDCAVEVSSTARTQDCSUM, SC, SD,
			  "<E> VANDLE vs. <E> BETA - SUMMED");
       DeclareHistogram2D(DD_TOFVSSTARTQDCSUM, SC, SD,
			  "TOF VANDLE vs. <E> BETA - SUMMED");

       //Plots related to the Ge detectors
       DeclareHistogram2D(DD_GAMMAENERGYVSTOF, SC, S9,
			  "GAMMA ENERGY vs. CorTOF VANDLE");
       DeclareHistogram2D(DD_TQDCAVEVSTOF_VETO, SC, SD,
			  "<E> VANDLE vs. CorTOF VANDLE - Gamma Veto");
       DeclareHistogram2D(DD_TOFBARS_VETO, SC, S9,
			  "Bar vs CorTOF - Gamma Veto");
    }//if (hasSmall)

    if(hasBig) {
       //Plots used for the general information about VANDLE
       DeclareHistogram2D(DD_TQDCBARS+dammIds::BIG_OFFSET, SD, numBigEnds,
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

    if(hasTvandle) {
	using namespace dammIds::tvandle;
	DeclareHistogram2D(DD_TQDCBARS+dammIds::TVANDLE_OFFSET, SD, S1,"QDC");
	DeclareHistogram2D(DD_MAXIMUMBARS+dammIds::TVANDLE_OFFSET, SC, S1, "Max");
	DeclareHistogram1D(D_TIMEDIFF, SE, "Time Difference");
	DeclareHistogram2D(DD_PVSP, SE, SE,"Phase vs. Phase");
	DeclareHistogram2D(DD_MAXRIGHTVSTDIFF, SA, SD,"Max Right vs. Time Diff");
	DeclareHistogram2D(DD_MAXLEFTVSTDIFF, SA, SD, "Max Left vs. Time Diff");
	DeclareHistogram2D(DD_MAXLVSTDIFFGATE, SA, SD,
			   "Max Left vs. Time Diff - gated on max right");
	DeclareHistogram2D(DD_MAXLVSTDIFFAMP, SA, SD,
			   "Max Left vs. Time Diff - amp diff");
	DeclareHistogram2D(DD_MAXLCORGATE, SA, SD,
			   "Max Left vs. Cor Time Diff");
	DeclareHistogram2D(DD_QDCVSMAX, SC, SD,"QDC vs Max - Right");
	DeclareHistogram2D(DD_SNRANDSDEV, S8, S2, "SNR and SDEV R01/L23");
    }//if(hasTvandle);

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

bool VandleProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    hasDecay =
        (event.GetCorrelator().GetCondition() == Correlator::VALID_DECAY);
    if(hasDecay)
        decayTime = event.GetCorrelator().GetDecayTime() *
                Globals::get()->clockInSeconds();
    plot(D_PROBLEMS, 30);

    if(RetrieveData(event)) {
        AnalyzeData(event);
        //CrossTalk();
        EndProcess();
        return true;
    } else {
        EndProcess();
        return (didProcess = false);
    }
}

bool VandleProcessor::RetrieveData(RawEvent &event) {
    ClearMaps();

    static const vector<ChanEvent*> &smallEvents =
        event.GetSummary("vandleSmall")->GetList();
    static const vector<ChanEvent*> &bigEvents =
        event.GetSummary("vandleBig")->GetList();
    static const vector<ChanEvent*> &betaStarts =
        event.GetSummary("scint:beta:start")->GetList();
    static const vector<ChanEvent*> &liquidStarts =
        event.GetSummary("scint:liquid:start")->GetList();
    static const vector<ChanEvent*> &tvandleEvents =
        event.GetSummary("tvandle")->GetList();

    vector<ChanEvent*> startEvents;
    startEvents.insert(startEvents.end(),
		       betaStarts.begin(), betaStarts.end());
    startEvents.insert(startEvents.end(),
		       liquidStarts.begin(), liquidStarts.end());

    if(smallEvents.empty() && bigEvents.empty() && tvandleEvents.empty()) {
        plot(D_PROBLEMS, 27);
        return(false);
    }

    FillMap(smallEvents, "small", smallMap);
    FillMap(bigEvents, "big", bigMap);
    FillMap(tvandleEvents, "tvandle", tvandleMap);
    FillMap(startEvents, "start", startMap);

    //Make the VandleBars - small/big will be in the same map
    //BuildBars(bigMap, "big", barMap);
    BuildBars(smallMap, "small", barMap);

    if(barMap.empty() && tvandleMap.empty()) {
        plot(D_PROBLEMS, 25); //DEBUGGING
        return(false);
    }

    return(true);
}

void VandleProcessor::AnalyzeData(RawEvent& rawev) {
    if(!tvandleMap.empty() && tvandleMap.size()%2 == 0)
        Tvandle();

    for (BarMap::iterator itBar = barMap.begin();
        itBar !=  barMap.end(); itBar++) {
        if(!(*itBar).second.event)
            continue;

        BarData bar = (*itBar).second;

        const int resMult = 2; //set resolution of histograms
        const int resOffset = 200; // offset of histograms
        unsigned int barLoc = (*itBar).first.first;
        unsigned int idOffset = -1;
        if((*itBar).first.second == "small")
            idOffset = 0;
        else
            idOffset = dammIds::BIG_OFFSET;

        TimingCalibration cal =
            TimingCalibrator::get()->GetCalibration((*itBar).first);

        double timeDiff = bar.timeDiff;

        plot(DD_DEBUGGING0, bar.qdcPos*resMult+resOffset,
            timeDiff*resMult+resOffset);

        plot(DD_TIMEDIFFBARS+idOffset,
            timeDiff*resMult+resOffset, barLoc);
        plot(DD_TQDCAVEVSTDIFF+idOffset,
            timeDiff*resMult+resOffset, bar.qdc);

        for(TimingDataMap::iterator itStart = startMap.begin();
            itStart != startMap.end(); itStart++) {
            if(!(*itStart).second.dataValid)
            continue;

            unsigned int startLoc = (*itStart).first.first;
            unsigned int barPlusStartLoc = barLoc*2 + startLoc;

            double tofOffset;
            if(startLoc == 0)
                tofOffset = cal.GetTofOffset0();
            else
                tofOffset = cal.GetTofOffset1();

            double TOF = bar.walkCorTimeAve -
                (*itStart).second.walkCorTime + tofOffset;
            double corTOF =
                CorrectTOF(TOF, bar.flightPath, cal.GetZ0());
            double energy =
                CalcEnergy(corTOF, cal.GetZ0());

            bar.timeOfFlight.insert(make_pair(startLoc, TOF));
            bar.corTimeOfFlight.insert(make_pair(startLoc, corTOF));
            bar.energy.insert(make_pair(startLoc, energy));

            if(corTOF >= 5)
                plot(DD_TQDCAVEVSENERGY+idOffset, energy, bar.qdc);
            plot(DD_TOFBARS+idOffset,
                TOF*resMult+resOffset, barPlusStartLoc);
            plot(DD_TOFVSTDIFF+idOffset, timeDiff*resMult+resOffset,
                 TOF*resMult+resOffset);
            plot(DD_MAXRVSTOF+idOffset,
                TOF*resMult+resOffset, bar.rMaxVal);
            plot(DD_MAXLVSTOF+idOffset,
                TOF*resMult+resOffset, bar.lMaxVal);
            plot(DD_TQDCAVEVSTOF+idOffset,
                TOF*resMult+resOffset, bar.qdc);

            plot(DD_CORTOFBARS,
                corTOF*resMult+resOffset, barPlusStartLoc);
            plot(DD_CORTOFVSTDIFF+idOffset,
                timeDiff*resMult + resOffset, corTOF*resMult+resOffset);
            plot(DD_MAXRVSCORTOF+idOffset, corTOF*resMult+resOffset,
                 bar.rMaxVal);
            plot(DD_MAXLVSCORTOF+idOffset,
                corTOF*resMult+resOffset, bar.lMaxVal);
            plot(DD_TQDCAVEVSCORTOF+idOffset,
                corTOF*resMult+resOffset, bar.qdc);

            if(startLoc == 0) {
                plot(DD_MAXSTART0VSTOF+idOffset,
                    TOF*resMult+resOffset, (*itStart).second.maxval);
                plot(DD_MAXSTART0VSCORTOF+idOffset,
                    corTOF*resMult+resOffset, (*itStart).second.maxval);
            } else if (startLoc == 1) {
                plot(DD_MAXSTART1VSCORTOF+idOffset,
                    corTOF*resMult+resOffset, (*itStart).second.maxval);
                plot(DD_MAXSTART1VSCORTOF+idOffset,
                    corTOF*resMult+resOffset, (*itStart).second.maxval);
            }

            static const DetectorSummary *geSummary = rawev.GetSummary("ge");

            if (geSummary) {
                if (geSummary->GetMult() > 0) {
                    const vector<ChanEvent *> &geList = geSummary->GetList();
                    for (vector<ChanEvent *>::const_iterator itGe = geList.begin();
                        itGe != geList.end(); itGe++) {
                        double calEnergy = (*itGe)->GetCalEnergy();
                        plot(DD_GAMMAENERGYVSTOF+idOffset, TOF, calEnergy);
                    }
                } else {
                    plot(DD_TQDCAVEVSTOF_VETO+idOffset, TOF, bar.qdc);
                    plot(DD_TOFBARS_VETO+idOffset, TOF, barPlusStartLoc);
                }
            }
        } // for(TimingDataMap::iterator itStart
    } //(BarMap::iterator itBar
} //void VandleProcessor::AnalyzeData

void VandleProcessor::BuildBars(const TimingDataMap &endMap, const string &type,
                                BarMap &barMap) {
    for(TimingDataMap::const_iterator itEndA = endMap.begin();
        itEndA != endMap.end();) {

        TimingDataMap::const_iterator itEndB = itEndA;
        itEndB++;

        if(itEndB == endMap.end()) //Handle some shit
            break;
        if((*itEndA).first.first != (*itEndB).first.first) {
            itEndA = itEndB;
            continue;
        }
        if(!(*itEndA).second.dataValid || !(*itEndB).second.dataValid){
            itEndA = itEndB;
            continue;
        }

        Vandle::BarIdentifier barKey((*itEndA).first.first, type);

        TimingCalibration cal = TimingCalibrator::get()->GetCalibration(barKey);

        if((*itEndA).second.dataValid && (*itEndB).second.dataValid)
            barMap.insert(make_pair(barKey, BarData((*itEndB).second,
                                    (*itEndA).second, cal, type)));
        else {
            itEndA = itEndB;
            continue;
        }
        itEndA = itEndB;
    }
}

void VandleProcessor::ClearMaps(void) {
    barMap.clear();
    bigMap.clear();
    smallMap.clear();
    startMap.clear();
    tvandleMap.clear();
}

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

    //Information for the bar of interest.
    string barType = "small";
    Vandle::BarIdentifier barA(0, barType);
    Vandle::BarIdentifier barB(1, barType);

    CrossTalkKey barsOfInterest(barA.first, barB.first);

    CrossTalkMap::iterator itBars =
	crossTalk.find(barsOfInterest);

    const int resMult = 2; //set resolution of histograms
    const int resOffset = 200; // set offset of histograms

    if(itBars != crossTalk.end())
	plot(D_CROSSTALK, (*itBars).second * resMult + resOffset);

    //Carbon Recoil Stuff
    BarMap::iterator itBarA = barMap.find(barA);
    BarMap::iterator itBarB = barMap.find(barB);

    if(itBarA == barMap.end() || itBarB == barMap.end())
	return;

//    TimeOfFlightMap::iterator itTofA =
// 	(*itBarA).second.timeOfFlight.find(startLoc);
//     TimeOfFlightMap::iterator itTofB =
// 	(*itBarB).second.timeOfFlight.find(startLoc);

//     if(itTofA == (*itBarA).second.timeOfFlight.end() ||
//        itTofB == (*itBarB).second.timeOfFlight.end())
// 	return;

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

    if(muon){
	plot(3951, tdiffA*resMult+100, tdiffB*resMult+100);
	plot(3952, muonTOF*resMult*10 + resOffset);
    }

//     plot(DD_TOFBARBVSBARA, tofA*resMult+resOffset,
//  	 tofB*resMult+resOffset);

//     if((tofB > tofA) && (tofB < (tofA+150))) {
//  	plot(DD_GATEDTQDCAVEVSTOF, tofA*resMult+resOffset,
//  	     (*itBarA).second.qdc);
//     }
}

void VandleProcessor::FillMap(const vector<ChanEvent*> &eventList,
			      const string type, TimingDataMap &eventMap) {
    unsigned int OFFSET = 0;
    if(type == "big")
	OFFSET = dammIds::BIG_OFFSET;
    else if(type == "tvandle")
	OFFSET = dammIds::TVANDLE_OFFSET;

    for(vector<ChanEvent*>::const_iterator it = eventList.begin();
	it != eventList.end(); it++) {
	unsigned int location = (*it)->GetChanID().GetLocation();
	string subType = (*it)->GetChanID().GetSubtype();

	Vandle::BarIdentifier key(location, subType);

	TimingDataMap::iterator itTemp =
	    eventMap.insert(make_pair(key, TimingData(*it))).first;

	if(type == "start")
	    continue;

	if((*itTemp).second.dataValid && (*itTemp).first.second == "right") {
	    plot(DD_TQDCBARS + OFFSET, (*itTemp).second.tqdc, location*2);
	    plot(DD_MAXIMUMBARS + OFFSET, (*itTemp).second.maxval, location*2);
	}
	else if((*itTemp).second.dataValid && (*itTemp).first.second == "left") {
	    plot(DD_TQDCBARS + OFFSET, (*itTemp).second.tqdc, location*2+1);
	    plot(DD_MAXIMUMBARS + OFFSET, (*itTemp).second.maxval, location*2+1);
	}
    }
}

void VandleProcessor::Tvandle(void) {
    //Needs cleaned heavily!!
    using namespace dammIds::tvandle;
    TimingData right = (*tvandleMap.find(make_pair(0,"right"))).second;
    TimingData left  = (*tvandleMap.find(make_pair(0,"left"))).second;

    char hisFileName[32];
    GetArgument(2, hisFileName, 32);
    string temp = hisFileName;
    temp = temp.substr(0, temp.find_first_of(" "));
    double offset = atof(temp.c_str());

    double timeDiff = left.highResTime - right.highResTime - offset;
    double corTimeDiff = left.walkCorTime - right.walkCorTime - offset;

    vector<int> trc = right.trace;
    vector<int> trc1 = left.trace;
    if(timeDiff < -5.418 && left.maxval < 90) {
	for(vector<int>::iterator it = trc.begin(); it != trc.end(); it++)
    	    plot(dammIds::vandle::DD_PROBLEMS, it-trc.begin(), counter, *it);
    	for(vector<int>::iterator it = trc1.begin(); it != trc1.end(); it++)
    	    plot(dammIds::vandle::DD_PROBLEMS, it-trc1.begin(), counter+1, *it);
    	counter+=2;
    }

    plot(DD_QDCVSMAX, right.maxval, right.tqdc);

    if(right.dataValid && left.dataValid){
	double timeRes = 50; //20 ps/bin
	double timeOff = 500;

	plot(D_TIMEDIFF, timeDiff*timeRes + timeOff);
	plot(DD_PVSP, right.tqdc, left.tqdc);
	plot(DD_MAXRIGHTVSTDIFF, timeDiff*timeRes+timeOff, right.maxval);
	plot(DD_MAXLEFTVSTDIFF, timeDiff*timeRes+timeOff, left.maxval);

	//Plot information about the SNR
	plot(DD_SNRANDSDEV, right.snr+50, 0);
	plot(DD_SNRANDSDEV, right.stdDevBaseline*timeRes+timeOff, 1);
	plot(DD_SNRANDSDEV, left.snr+50, 2);
	plot(DD_SNRANDSDEV, left.stdDevBaseline*timeRes+timeOff, 3);

	//Plot information used to determine the impact of walk.
	double tempVal = fabs(right.maxval-left.maxval);
	if(tempVal <= 50)
	    plot(DD_MAXLVSTDIFFAMP, timeDiff*timeRes+timeOff, left.maxval);

	plot(DD_MAXLCORGATE,
	     corTimeDiff*timeRes+timeOff, left.maxval);

	if(right.maxval > 3000) {
	    plot(DD_MAXLVSTDIFFGATE,
		 timeDiff*timeRes+timeOff, left.maxval);
	}
    } else // if(right.dataValid
	plot(D_PROBLEMS, 2);
}
