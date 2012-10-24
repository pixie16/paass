/************************************
Processes information from the VANDLE Bars 

Updated: S.V. Paulauskas 26 July 2010  
Original M. Madurga
*************************************/
#include <fstream>
#include <iostream>

#include <cmath>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "RawEvent.hpp"
#include "VandleProcessor.hpp"

namespace dammIds {
    const unsigned int BIG_OFFSET     = 30;
    const unsigned int MISC_OFFSET    = 60;
    const unsigned int TVANDLE_OFFSET = 70;
	
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
	const int DD_TDIFFVSTOF       = 8;
	const int DD_MAXRVSTOF        = 9;
	const int DD_MAXLVSTOF        = 10;
	const int DD_TQDCAVEVSTOF     = 11;
	
	//Plots related to the CorTOF
	const int DD_TDIFFVSCORTOF     = 12;
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
    }//namespace vandle

    namespace tvandle {
	const int D_TIMEDIFF     = 3  + TVANDLE_OFFSET;
	const int DD_PVSP        = 4  + TVANDLE_OFFSET; 
	const int DD_MAXVSTDIFF  = 5  + TVANDLE_OFFSET; 
	const int DD_QDCVSMAX    = 6  + TVANDLE_OFFSET; 
	const int DD_AMPMAPSTART = 7  + TVANDLE_OFFSET; 
	const int DD_AMPMAPSTOP  = 8  + TVANDLE_OFFSET; 
	const int D_SNRSTART     = 9  + TVANDLE_OFFSET; 
	const int D_SNRSTOP      = 10 + TVANDLE_OFFSET; 
	const int D_SDEVBASESTART= 11 + TVANDLE_OFFSET; 
	const int D_SDEVBASESTOP = 12 + TVANDLE_OFFSET; 
	const int DD_MAXSVSTDIFF = 14 + TVANDLE_OFFSET; 
    }// namespace tvandle
}//namespace dammIds

using namespace std;
using namespace dammIds::vandle;


//*********** VandleProcessor **********
VandleProcessor::VandleProcessor(): EventProcessor(dammIds::vandle::OFFSET, dammIds::vandle::RANGE)
{
    name = "vandle";
    associatedTypes.insert("liquid_scint"); 
    associatedTypes.insert("vandleSmall"); 
    associatedTypes.insert("vandleBig");
    associatedTypes.insert("tvandle");
}


//********** Init **********
bool VandleProcessor::Init(DetectorDriver &driver)
{
    if(EventProcessor::Init(driver)){
	return(true);
    }  else {
	return(false);
    }
}


//********** Declare Plots **********
void VandleProcessor::DeclarePlots(void)
{
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
       DeclareHistogram2D(DD_TDIFFVSTOF, SC, S9, 
			  "TDiff vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXRVSTOF, SD, SC, 
       // 			  "MaxR vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXLVSTOF, SD, SC,
       // 			  "MaxL vs. TOF(0.5ns/bin)");
       DeclareHistogram2D(DD_TQDCAVEVSTOF, SC, SD,
			  "<E> vs. TOF(0.5ns/bin)");
       //Plots related to the corTOF
       DeclareHistogram2D(DD_TDIFFVSCORTOF, SC, S9,
			  "TDiff vs. CorTOF(0.5ns/bin)");
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
       // DeclareHistogram2D(DD_TDIFFVSTOF+dammIds::BIG_OFFSET, SC, S9, 
       // 			  "TDiff vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXRVSTOF+dammIds::BIG_OFFSET, SD, SC, 
       // 			  "MaxR vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXLVSTOF+dammIds::BIG_OFFSET, SD, SC,
       // 			  "MaxL vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_TQDCAVEVSTOF+dammIds::BIG_OFFSET, SC, SD,
       // 			  "<E> vs. TOF(0.5ns/bin)");
       //Plots related to the corTOF
       // DeclareHistogram2D(DD_TDIFFVSCORTOF+dammIds::BIG_OFFSET, SC, S9,
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
	DeclareHistogram1D(D_TIMEDIFF, SA, "Time Difference");
	DeclareHistogram2D(DD_PVSP, SE, SE,"Phase vs. Phase");
	DeclareHistogram2D(DD_MAXVSTDIFF, SA, SC,"Max Right vs. Time Diff");
	DeclareHistogram2D(DD_QDCVSMAX, SC, SD,"QDC vs Max - Right");
	//DeclareHistogram2D(DD_AMPMAPSTART, S7, SC,"Amp Map Start");
	//DeclareHistogram2D(DD_AMPMAPSTOP, S7, SC,"Amp Map Stop");
	DeclareHistogram1D(D_SNRSTART, SE, "SNR - Right");
	DeclareHistogram1D(D_SNRSTOP, SE, "SNR - Left");
	DeclareHistogram1D(D_SDEVBASESTART, S8, "Sdev Base - Right");
	DeclareHistogram1D(D_SDEVBASESTOP, S8, "Sdev Base - Left");
	DeclareHistogram2D(DD_PROBLEMS, SB, S5, "Problems - 2D");
	DeclareHistogram2D(DD_MAXSVSTDIFF, SD, SC, "Max Left vs. Time Diff");
    }//if(hasTvandle);
	
    //Histograms for the CrossTalk Subroutine
    //DeclareHistogram1D(D_CROSSTALK, SC, "CrossTalk Between Two Bars");
    //DeclareHistogram2D(DD_GATEDTQDCAVEVSTOF, SC, SD, 
    //"<E> vs. TOF0 (0.5ns/bin) - Gated");
    //DeclareHistogram2D(DD_TOFBARBVSBARA, SC, SC, "TOF Bar1 vs. Bar2");

    //DeclareHistogram2D(, S8, S8, "tdiffA vs. tdiffB");
    //DeclareHistogram1D(, SD, "Muons");
    //DeclareHistogram2D(, S8, S8, "tdiffA vs. tdiffB");
}// Declare Plots


//********** Process **********
bool VandleProcessor::Process(RawEvent &event) 
{
    if (!EventProcessor::Process(event)) //start event processing
	return false;

    hasDecay = 
	(event.GetCorrelator().GetCondition() == Correlator::VALID_DECAY);
    if(hasDecay)
	decayTime = event.GetCorrelator().GetDecayTime() * pixie::clockInSeconds;

    plot(D_PROBLEMS, 30); //DEBUGGING

    if(RetrieveData(event)) {
	AnalyzeData();
	//CrossTalk();
	EndProcess();
	return true;
    } else {
	EndProcess();
	return (didProcess = false);
    }
}


//********** RetrieveData **********
bool VandleProcessor::RetrieveData(RawEvent &event) 
{    
    ClearMaps();
   
    static const vector<ChanEvent*> &smallEvents = 
	event.GetSummary("vandleSmall")->GetList();
    static const vector<ChanEvent*> &bigEvents = 
	event.GetSummary("vandleBig")->GetList();
    static const vector<ChanEvent*> &betaStarts = 
	event.GetSummary("liquid_scint:beta:start")->GetList();
    static const vector<ChanEvent*> &liquidStarts = 
	event.GetSummary("liquid_scint:liquid:start")->GetList();
    static const vector<ChanEvent*> &tvandleEvents = 
	event.GetSummary("tvandle")->GetList();

    //Construct and fill the vector for the startEvents
    vector<ChanEvent*> startEvents;
    startEvents.insert(startEvents.end(), 
		       betaStarts.begin(), betaStarts.end());
    startEvents.insert(startEvents.end(), 
		       liquidStarts.begin(), liquidStarts.end());

    if(smallEvents.empty() && bigEvents.empty() ) {
        plot(D_PROBLEMS, 27); //DEBUGGING
	return(false);
    }
     
    FillMap(smallEvents, "small", smallMap);
    FillMap(bigEvents, "big", bigMap);
    FillMap(tvandleEvents, "tvandle", tvandleMap);
    FillMap(startEvents, "start", startMap);
    
    //Make the VandleBars - small/big will be in the same map
    BuildBars(bigMap, "big", barMap);
    BuildBars(smallMap, "small", barMap);
    
    if(barMap.empty()) {
	plot(D_PROBLEMS, 25); //DEBUGGING
	return(false);
    }

    return(true);
} // bool VandleProcessor::RetrieveData


//********** AnalyzeData **********
void VandleProcessor::AnalyzeData(void)
{
    //Analyze the Teeny VANDLE data if there is any
    if(!tvandleMap.empty() && tvandleMap.size()%2 == 0)
	Tvandle();

    //Analyze the VANDLE bars if any are present.
    for (BarMap::iterator itBar = barMap.begin(); 
	 itBar !=  barMap.end(); itBar++) {
	if(!(*itBar).second.event)
	    continue;
	
	//Set some useful values.
	const int resMult = 2; //set resolution of histograms
	const int resOffset = 200; // offset of histograms
	unsigned int barLoc = (*itBar).first.first;
	unsigned int idOffset = -1;
	if((*itBar).first.second == "small")
	    idOffset = 0;
	else
	   idOffset = dammIds::BIG_OFFSET;

	TimingCal calibration =
	    GetTimingCal((*itBar).first);
	
	plot(DD_TIMEDIFFBARS+idOffset, 
	     (*itBar).second.timeDiff*resMult+resOffset, barLoc); 
	plot(DD_TQDCAVEVSTDIFF+idOffset, 
	     (*itBar).second.timeDiff*resMult+resOffset, (*itBar).second.qdc);
	
	//Loop over the starts in the event	
	for(TimingDataMap::iterator itStart = startMap.begin(); 
	    itStart != startMap.end(); itStart++) {
	    if(!(*itStart).second.dataValid)
		continue;
	    
	    unsigned int startLoc = (*itStart).first.first;
	    unsigned int barPlusStartLoc = barLoc*2 + startLoc;

	    double tofOffset;
	    if(startLoc == 0)
		tofOffset = calibration.tofOffset0;
	    else 
		tofOffset = calibration.tofOffset1;
	    
	    //times are calculated in ns, energy in keV
	    double TOF = 
		(*itBar).second.timeAve - (*itStart).second.highResTime + tofOffset; 
	    double corTOF = 
		CorrectTOF(TOF, (*itBar).second.flightPath, calibration.z0); 
	    double energy = 
		CalcEnergy(corTOF, calibration.z0);
	    
	    (*itBar).second.timeOfFlight.insert(make_pair(startLoc, TOF));
	    (*itBar).second.corTimeOfFlight.insert(make_pair(startLoc, corTOF));
	    (*itBar).second.energy.insert(make_pair(startLoc, energy));
	    
	    if(corTOF >= 5) // cut out the gamma prompt
		plot(DD_TQDCAVEVSENERGY+idOffset, energy, (*itBar).second.qdc);
	    plot(DD_TOFBARS+idOffset, 
		 TOF*resMult+resOffset, barPlusStartLoc);
	    plot(DD_TDIFFVSTOF+idOffset, TOF*resMult+resOffset, 
		 (*itBar).second.timeDiff*resMult+resOffset);
	    plot(DD_MAXRVSTOF+idOffset, 
		 TOF*resMult+resOffset, (*itBar).second.rMaxVal);
	    plot(DD_MAXLVSTOF+idOffset, 
		 TOF*resMult+resOffset, (*itBar).second.lMaxVal);
	    plot(DD_TQDCAVEVSTOF+idOffset, 
		 TOF*resMult+resOffset, (*itBar).second.qdc);

	    plot(DD_CORTOFBARS, 
		 corTOF*resMult+resOffset, barPlusStartLoc); 
	    plot(DD_TDIFFVSCORTOF+idOffset, corTOF*resMult+resOffset, 
		 (*itBar).second.timeDiff*resMult + resOffset);
	    plot(DD_MAXRVSCORTOF+idOffset, 
		 corTOF*resMult+resOffset, (*itBar).second.rMaxVal);
	    plot(DD_MAXLVSCORTOF+idOffset, 
		 corTOF*resMult+resOffset, (*itBar).second.lMaxVal);
	    plot(DD_TQDCAVEVSCORTOF+idOffset, 
		 corTOF*resMult+resOffset, (*itBar).second.qdc);
	    
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

	    //Now we will do some Ge related stuff
	    extern RawEvent rawev;
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
		    // vetoed stuff
		    plot(DD_TQDCAVEVSTOF_VETO+idOffset, TOF, 
			 (*itBar).second.qdc);
		    plot(DD_TOFBARS_VETO+idOffset, TOF, barPlusStartLoc);
		}
	    } 
	} // for(TimingDataMap::iterator itStart
    } //(BarMap::iterator itBar
} //void VandleProcessor::AnalyzeData


//********** BuildBars **********
void VandleProcessor::BuildBars(const TimingDataMap &endMap, const string &type, BarMap &barMap) 
{
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
	
	IdentKey barKey((*itEndA).first.first, type); 
	
	TimingCal calibrations =
	    GetTimingCal(barKey);
	
	if((*itEndA).second.dataValid
	   && (*itEndB).second.dataValid) 
	    barMap.insert(
		make_pair(barKey, BarData((*itEndB).second, 
					  (*itEndA).second, calibrations, type)));
        else {
	    itEndA = itEndB;
	    continue;
	}
	itEndA = itEndB;
    } // for(itEndA
} //void VandleProcessor::BuildBars


//********** ClearMaps *********
void VandleProcessor::ClearMaps(void)
{
    barMap.clear();
    bigMap.clear();
    smallMap.clear();
    startMap.clear();
    tvandleMap.clear();
}


//********** CrossTalk **********
void VandleProcessor::CrossTalk(void)
{
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
    IdentKey barA(0, barType);
    IdentKey barB(1, barType);

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
} //void VandleProcessor::CrossTalk


//********** FillMap **********
void VandleProcessor::FillMap(const vector<ChanEvent*> &eventList, 
			      const string type, TimingDataMap &eventMap) 
{
    unsigned int OFFSET = 0;
    if(type == "big")
	OFFSET = dammIds::BIG_OFFSET;
    else if(type == "tvandle")
	OFFSET = dammIds::TVANDLE_OFFSET;

    for(vector<ChanEvent*>::const_iterator it = eventList.begin();
	it != eventList.end(); it++) {
	unsigned int location = (*it)->GetChanID().GetLocation();
	string subType = (*it)->GetChanID().GetSubtype();
	
	IdentKey key(location, subType);
	
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


//********* Tvandle **********
void VandleProcessor::Tvandle(void) 
{
    //Needs cleaned heavily!!
    using namespace dammIds::tvandle;
    IdentKey rightKey(0,"right");
    IdentKey leftKey (1,"left");
    
    TimingDataMap::iterator itRight = tvandleMap.find(rightKey);
    TimingDataMap::iterator itLeft  = tvandleMap.find(leftKey);
    
    // unsigned int maxPosRight = (unsigned int)(*itRight).second.maxpos;
    // unsigned int maxPosLeft  = (unsigned int)(*itLeft).second.maxpos;
    unsigned int maxValRight = (unsigned int)(*itRight).second.maxval;
    unsigned int maxValLeft  = (unsigned int)(*itLeft).second.maxval;
    double timeDiff = 
	(*itLeft).second.highResTime - (*itRight).second.highResTime;
    double walkCorTimeDiff = 
	(*itLeft).second.walkCorTime - (*itRight).second.walkCorTime;

    double snrRight = pow((*itRight).second.maxval/(*itRight).second.stdDevBaseline, 2.);
    double snrLeft = pow((*itLeft).second.maxval/(*itLeft).second.stdDevBaseline, 2.);

    vector<int> trc = (*itRight).second.trace;
    vector<int> trc1 = (*itLeft).second.trace;
    if(timeDiff < (1600.-2000.)/50.) {
	for(vector<int>::iterator it = trc.begin(); it != trc.end(); it++)
	    plot(DD_PROBLEMS, it-trc.begin(), counter, *it);
	for(vector<int>::iterator it = trc1.begin(); it != trc1.end(); it++)
	    plot(DD_PROBLEMS, it-trc1.begin(), counter+1, *it);
	counter+=2;
    }
    
    //Fill histograms
    plot(DD_QDCVSMAX, (*itRight).second.maxval, 
	 (*itRight).second.tqdc);
   
    if((*itRight).second.dataValid && 
       (*itLeft).second.dataValid){
	double timeRes = 50; //100 ps/bin
	double timeOff = 200; 

	plot(D_TIMEDIFF, timeDiff*timeRes + timeOff);
	plot(DD_PVSP, (*itRight).second.phase*timeRes-12000, 
	     (*itLeft).second.phase*timeRes-12000);
	//plot(DD_MAXVSTDIFF, timeDiff*timeRes+timeOff, maxValRight);
	
	//Plot information Pertaining to the SNR
	plot(D_SNRSTART, snrRight*0.25);
	plot(D_SNRSTOP, snrLeft*0.25);
	plot(D_SDEVBASESTART, (*itRight).second.stdDevBaseline*timeRes+timeOff);
	plot(D_SDEVBASESTOP, (*itLeft).second.stdDevBaseline*timeRes+timeOff);

	//Plot information used to determine the impact of walk.
	double tempVal = fabs(maxValRight-maxValLeft);
	if(tempVal <= 50)
	    plot(DD_MAXVSTDIFF, timeDiff*timeRes+timeOff, 
		 maxValRight);
	
	if(maxValRight < 4096 && maxValRight > 3700)
//	    plot(DD_MAXSVSTDIFF, 
//		 timeDiff*timeRes+timeOff, maxValLeft);
	    plot(DD_MAXSVSTDIFF, 
		 walkCorTimeDiff*timeRes+timeOff, maxValLeft);

    }// if((*itRight).second.dataValid
}
