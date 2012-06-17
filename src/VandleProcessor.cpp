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
   const unsigned int BIG_OFFSET  = 400;
   const unsigned int MISC_OFFSET = 800;
   
   //Plots used for debugging
   const int D_PROBLEMS     = 3900;
   const int DD_PROBLEMS    = 3901;
   
   //Plots for the general information about VANDLE
   const int DD_TQDCBARS         = 3100;
   const int DD_MAXIMUMBARS      = 3101;
   const int DD_TIMEDIFFBARS     = 3102;
   const int DD_TOFBARS          = 3103;
   const int DD_CORTOFBARS       = 3105;
   const int D_TOF               = 3106;
   const int DD_TQDCAVEVSTDIFF   = 3107;
   
   //Plots related to the TOF
   const int DD_TDIFFVSTOF       = 3108;
   const int DD_MAXRVSTOF        = 3109;
   const int DD_MAXLVSTOF        = 3110;
   const int DD_TQDCAVEVSTOF     = 3111;
   
   //Plots related to the CorTOF
   const int DD_TDIFFVSCORTOF     = 3112;
   const int DD_MAXRVSCORTOF      = 3113;
   const int DD_MAXLVSCORTOF      = 3114;
   const int DD_TQDCAVEVSCORTOF   = 3115;
   const int DD_TQDCAVEVSENERGY   = 3116;
   
   //Plots related to correlated times
   const int DD_CORRELATED_TOF      = 3117;
   
   //Plots related to the Start detectors
   const int DD_MAXSTART0VSTOF   = 3118;
   const int DD_MAXSTART1VSTOF   = 3119;
   const int DD_MAXSTART0VSCORTOF = 3120;
   const int DD_MAXSTART1VSCORTOF = 3121;
   const int DD_TQDCAVEVSSTARTQDCSUM= 3122;
   const int DD_TOFVSSTARTQDCSUM    = 3123;
   
   //Plots related to the Ge detectors
   const int DD_GAMMAENERGYVSTOF = 3124;
   const int DD_TQDCAVEVSTOF_VETO= 3125;
   const int DD_TOFBARS_VETO  = 3126;
   
   //CrossTalk Subroutine
   const int DD_TOFBARBVSBARA      = 3902;
   const int DD_GATEDTQDCAVEVSTOF  = 3903;
   const int D_CROSSTALK           = 3904;
}//namespace dammIds

using namespace std;
using namespace dammIds;


//*********** VandleProcessor **********
VandleProcessor::VandleProcessor(): EventProcessor()
{
    name = "vandle";
    associatedTypes.insert("scint"); 
    associatedTypes.insert("vandleSmall"); 
    associatedTypes.insert("vandleBig"); 
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
    //Plots used for debugging
    DeclareHistogram1D(D_PROBLEMS, S5, "1D Debugging");
    DeclareHistogram2D(DD_PROBLEMS, S7, S7, "2D Debugging");
    
    bool hasSmall = true;
    bool hasBig   = false;

    if(hasSmall) {
       //Plots used for the general information about VANDLE
       DeclareHistogram2D(DD_TQDCBARS, SE, S5, 
			  "Det Loc vs Trace QDC");
       // DeclareHistogram2D(DD_MAXIMUMBARS, SC, S5, 
       // 			  "Det Loc vs. Maximum");
       DeclareHistogram2D(DD_TIMEDIFFBARS, S9, S5, 
			  "Bars vs. Time Differences");
       DeclareHistogram2D(DD_TOFBARS, SC, S5, 
			  "Bar vs. Time of Flight");
       DeclareHistogram2D(DD_CORTOFBARS, SC, S5, 
			  "Bar vs  Cor Time of Flight");
       //DeclareHistogram2D(DD_TQDCAVEVSTDIFF, SC, SE, 
       //		  "<E> vs. Time Diff(0.5ns/bin)");
       //Plots related to the TOF
       DeclareHistogram2D(DD_TDIFFVSTOF, SC, S9, 
			  "TDiff vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXRVSTOF, SD, SC, 
       // 			  "MaxR vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXLVSTOF, SD, SC,
       // 			  "MaxL vs. TOF(0.5ns/bin)");
       DeclareHistogram2D(DD_TQDCAVEVSTOF, SC, SE,
			  "<E> vs. TOF(0.5ns/bin)");
       //Plots related to the corTOF
       DeclareHistogram2D(DD_TDIFFVSCORTOF, SC, S9,
			  "TDiff vs. CorTOF(0.5ns/bin)");
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
       DeclareHistogram2D(DD_TQDCBARS+dammIds::BIG_OFFSET, SE, S5, 
			  "Det Loc vs Trace QDC");
       // DeclareHistogram2D(DD_MAXIMUMBARS+dammIds::BIG_OFFSET, SC, S5, 
       // 			  "Det Loc vs. Maximum");
       DeclareHistogram2D(DD_TIMEDIFFBARS+dammIds::BIG_OFFSET, S9, S5, 
			  "Bars vs. Time Differences");
       DeclareHistogram2D(DD_TOFBARS+dammIds::BIG_OFFSET, SC, S5, 
			  "Bar vs. Time of Flight");
       DeclareHistogram2D(DD_CORTOFBARS+dammIds::BIG_OFFSET, SC, S5, 
			  "Bar vs  Cor Time of Flight");
       DeclareHistogram2D(DD_TQDCAVEVSTDIFF+dammIds::BIG_OFFSET, SC, SE, 
			  "<E> vs. Time Diff(0.5ns/bin)");
       //Plots related to the TOF
       DeclareHistogram2D(DD_TDIFFVSTOF+dammIds::BIG_OFFSET, SC, S9, 
			  "TDiff vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXRVSTOF+dammIds::BIG_OFFSET, SD, SC, 
       // 			  "MaxR vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXLVSTOF+dammIds::BIG_OFFSET, SD, SC,
       // 			  "MaxL vs. TOF(0.5ns/bin)");
       DeclareHistogram2D(DD_TQDCAVEVSTOF+dammIds::BIG_OFFSET, SC, SE,
			  "<E> vs. TOF(0.5ns/bin)");
       //Plots related to the corTOF
       DeclareHistogram2D(DD_TDIFFVSCORTOF+dammIds::BIG_OFFSET, SC, S9,
			  "TDiff vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXRVSCORTOF+dammIds::BIG_OFFSET, SD, SC,
       // 			  "MaxR vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXLVSCORTOF+dammIds::BIG_OFFSET, SD, SC,
       // 			  "MaxL vs. CorTOF(0.5ns/bin)");    
       // DeclareHistogram2D(DD_TQDCAVEVSCORTOF+dammIds::BIG_OFFSET, SC, SE, 
       // 			  "<E> vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_TQDCAVEVSENERGY+dammIds::BIG_OFFSET, SC, SE, 
       // 			  "TQDC vs Energy (kev/bin)");
       //Plots related to Correlated times
       DeclareHistogram2D(DD_CORRELATED_TOF+dammIds::BIG_OFFSET, SC, SC, 
			  "Correlated TOF");
       //Plots related to the Starts
       // DeclareHistogram2D(DD_MAXSTART0VSTOF+dammIds::BIG_OFFSET, SD, SC,
       // 			  "Max Start0 vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXSTART1VSTOF+dammIds::BIG_OFFSET, SD, SC,
       // 			  "Max Start1 vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXSTART0VSCORTOF+dammIds::BIG_OFFSET, SD, SC,
       // 			  "Max Start0 vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXSTART1VSCORTOF+dammIds::BIG_OFFSET, SD, SC, 
       // 			  "Max Start1 vs. CorTOF(0.5ns/bin)");
       DeclareHistogram2D(DD_TQDCAVEVSSTARTQDCSUM+dammIds::BIG_OFFSET,
			  SC, SE, "<E> VANDLE vs. <E> BETA - SUMMED");
       DeclareHistogram2D(DD_TOFVSSTARTQDCSUM+dammIds::BIG_OFFSET, SC, SE, 
			  "TOF VANDLE vs. <E> BETA - SUMMED");
       //Plots related to the Ge detectors
       DeclareHistogram2D(DD_GAMMAENERGYVSTOF+dammIds::BIG_OFFSET, 
			  SC, S9, "GAMMA ENERGY vs. CorTOF VANDLE");
       DeclareHistogram2D(DD_TQDCAVEVSTOF_VETO+dammIds::BIG_OFFSET, SC, SE, 
			  "<E> VANDLE vs. CorTOF VANDLE - Gamma Veto");
       DeclareHistogram2D(DD_TOFBARS_VETO+dammIds::BIG_OFFSET, SC, S9, 
			  "Bar vs CorTOF - Gamma Veto"); 
    }//if (hasBig)

    //Histograms for the CrossTalk Subroutine
//    DeclareHistogram1D(D_CROSSTALK, SC, "CrossTalk Between Two Bars");
//    DeclareHistogram2D(DD_GATEDTQDCAVEVSTOF, SC, SE, "<E> vs. TOF0 (0.5ns/bin) - Gated");
//    DeclareHistogram2D(DD_TOFBARBVSBARA, SC, SC, "TOF Bar1 vs. Bar2");

    DeclareHistogram2D(3950, S8, S8, "tdiffA vs. tdiffB");
    DeclareHistogram1D(3951, SE, "Muons");
    DeclareHistogram2D(3952, S8, S8, "tdiffA vs. tdiffB");
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
	event.GetSummary("scint:beta:start")->GetList();
    static const vector<ChanEvent*> &liquidStarts = 
	event.GetSummary("scint:liquid:start")->GetList();

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
    
    for(vector<ChanEvent*>::const_iterator itStart = startEvents.begin(); 
	itStart != startEvents.end(); itStart++) {
	unsigned int location = 
	    (*itStart)->GetChanID().GetLocation();
	string subType = (*itStart)->GetChanID().GetSubtype();
	
	IdentKey startKey(location, subType);
	
	TimingDataMap::iterator itTemp = 
	    startMap.insert(make_pair(startKey, TimingData(*itStart))).first;
    } 
    
    //Make the maps for the BIG ends and graph.
    for(vector<ChanEvent*>::const_iterator itBig = bigEvents.begin();
	itBig != bigEvents.end(); itBig++) {
	unsigned int location = (*itBig)->GetChanID().GetLocation();
	string subType = (*itBig)->GetChanID().GetSubtype();
	
	IdentKey bigKey(location, subType);
	TimingDataMap::iterator itTemp = 
	    bigMap.insert(make_pair(bigKey, TimingData(*itBig))).first;

	if((*itTemp).second.dataValid && (*itTemp).first.second == "right") {
	    plot(DD_TQDCBARS, (*itTemp).second.tqdc, location*2);
	    plot(DD_MAXIMUMBARS, (*itTemp).second.maxval, location*2);
	}
	else if((*itTemp).second.dataValid && (*itTemp).first.second == "left") {
	    plot(DD_TQDCBARS, (*itTemp).second.tqdc, location*2+1);
	    plot(DD_MAXIMUMBARS, (*itTemp).second.maxval, location*2+1);
	}
    }

    //Make the maps for the SMALL ends and graph. 
    for(vector<ChanEvent*>::const_iterator itSmall = smallEvents.begin();
	itSmall != smallEvents.end(); itSmall++) {
	unsigned int location = (*itSmall)->GetChanID().GetLocation();
	string subType = (*itSmall)->GetChanID().GetSubtype();
	
	IdentKey smallKey(location, subType);

	TimingDataMap::iterator itTemp = 
	    smallMap.insert(make_pair(smallKey, TimingData(*itSmall))).first;
	
	if((*itTemp).second.dataValid && (*itTemp).first.second == "right") {
	    plot(DD_TQDCBARS, (*itTemp).second.tqdc, location*2);
	    plot(DD_MAXIMUMBARS, (*itTemp).second.maxval, location*2);
	} 
	else if((*itTemp).second.dataValid && (*itTemp).first.second == "left") {
	    plot(DD_TQDCBARS, (*itTemp).second.tqdc, location*2+1);
	    plot(DD_MAXIMUMBARS, (*itTemp).second.maxval, location*2+1);
	}
    } 
    
    //Make the VandleBars - small/big will be in the same map
    BuildBars(bigMap, "big", barMap);
    BuildBars(smallMap, "small", barMap);
    
    if(barMap.empty()) {
	plot(D_PROBLEMS, 25); //DEBUGGING
	return(false);
    }

    return(true);
} // bool VandleProcessor::RetrieveData


//********** CorrectTOF **********
double VandleProcessor::CorrectTOF(const double &TOF, const double &corRadius, const double &z0)
{
    return((z0/corRadius)*TOF); // in ns
}


//********** AnalyzeData **********
void VandleProcessor::AnalyzeData(void)
{ 
    for (BarMap::iterator itBar = barMap.begin(); 
	 itBar !=  barMap.end(); itBar++) {
	if(!(*itBar).second.barEvent)
	    continue;
	
	unsigned int barLoc = (*itBar).first.first;
	unsigned int idOffset = -1;
	if((*itBar).first.second == "small")
	    idOffset = 0;
	else
	   idOffset = dammIds::BIG_OFFSET;

	TimingCal calibration =
	    GetTimingCal((*itBar).first);
	
	plot(DD_TIMEDIFFBARS+idOffset, 
	     (*itBar).second.timeDiff*2+200, barLoc); 
	plot(DD_TQDCAVEVSTDIFF+idOffset, 
	     (*itBar).second.timeDiff*2+200, (*itBar).second.qdc);
	
	//Loop over the starts in the event	
	for(TimingDataMap::iterator itStart = startMap.begin(); 
	    itStart != startMap.end(); itStart++) {
	    if(!(*itStart).second.dataValid)
		continue;
	    
	    unsigned int startLoc = (*itStart).first.first;

	    unsigned int barPlusStartLoc = barLoc*2 + startLoc;

	    const int resMult = 2; //set resolution of histograms
	    const int resOffSet = 2000; // offset of histograms
	    
	    double tofOffset;
	    if(startLoc == 0)
		tofOffset = calibration.tofOffset0;
	    else 
		tofOffset = calibration.tofOffset1;
	    
	    //times are calculated in ns, energy in keV
	    double TOF = 
		(*itBar).second.timeAve - (*itStart).second.highResTime - tofOffset; 
	    double corTOF = 
		CorrectTOF(TOF, (*itBar).second.corRadius, calibration.z0); 
	    double energy = 
		CalcEnergy(corTOF, calibration.z0);
	    
	    (*itBar).second.timeOfFlight.insert(make_pair(startLoc, TOF));
	    (*itBar).second.corTimeOfFlight.insert(make_pair(startLoc, corTOF));
	    (*itBar).second.energy.insert(make_pair(startLoc, energy));
	    
	    if(corTOF >= 5) // cut out the gamma prompt
		plot(DD_TQDCAVEVSENERGY+idOffset, energy, (*itBar).second.qdc);
	    plot(DD_TOFBARS+idOffset, 
		 TOF*resMult+resOffSet, barPlusStartLoc);
	    plot(DD_TDIFFVSTOF+idOffset, 
		 TOF*resMult+resOffSet, (*itBar).second.timeDiff*resMult+200);
	    plot(DD_MAXRVSTOF+idOffset, 
		 TOF*resMult+resOffSet, (*itBar).second.rMaxVal);
	    plot(DD_MAXLVSTOF+idOffset, 
		 TOF*resMult+resOffSet, (*itBar).second.lMaxVal);
	    plot(DD_TQDCAVEVSTOF+idOffset, 
		 TOF*resMult+resOffSet, (*itBar).second.qdc);

	    plot(DD_CORTOFBARS, 
		 corTOF*resMult+resOffSet, barPlusStartLoc); 
	    plot(DD_TDIFFVSCORTOF+idOffset, 
		 corTOF*resMult+resOffSet, (*itBar).second.timeDiff);
	    plot(DD_MAXRVSCORTOF+idOffset, 
		 corTOF*resMult+resOffSet, (*itBar).second.rMaxVal);
	    plot(DD_MAXLVSCORTOF+idOffset, 
		 corTOF*resMult+resOffSet, (*itBar).second.lMaxVal);
	    plot(DD_TQDCAVEVSCORTOF+idOffset, 
		 corTOF*resMult+resOffSet, (*itBar).second.qdc);
	    
	    if(startLoc == 0) {
		plot(DD_MAXSTART0VSTOF+idOffset, 
		     TOF*resMult+resOffSet, (*itStart).second.maxval);
		plot(DD_MAXSTART0VSCORTOF+idOffset, 
		     corTOF*resMult+resOffSet, (*itStart).second.maxval);
	    } else if (startLoc == 1) {
 	        plot(DD_MAXSTART1VSCORTOF+idOffset, 
		     corTOF*resMult+resOffSet, (*itStart).second.maxval);
		plot(DD_MAXSTART1VSCORTOF+idOffset, 
		     corTOF*resMult+resOffSet, (*itStart).second.maxval);
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
}


//********** CrossTalk **********
void VandleProcessor::CrossTalk(void)
{
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
    unsigned int startLoc = 0;
        

    CrossTalkKey barsOfInterest(barA.first, barB.first);
    
    CrossTalkMap::iterator itBars = 
	crossTalk.find(barsOfInterest);
    
    const int resMult = 2; //set resolution of histograms
    const int resOffset = 2000; // set offset of histograms
    
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

    bool onBar = (tdiffA + tdiffB <= 0.75 && tdiffA + tdiffB >= 0.25);
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
