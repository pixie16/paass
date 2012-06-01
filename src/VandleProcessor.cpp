/************************************
Processes information from the VANDLE Bars 

Updated: S. Paulauskas 26 July 2010  

*************************************/
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <fstream>
#include <iostream>
#include <numeric>
#include <string>

#include <unistd.h> //not standard c/c++, makes compliant with UNIX standards

#include "damm_plotids.h"
#include "DetectorDriver.h"
#include "RawEvent.h"
#include "VandleProcessor.h"

#ifdef useroot
#include <TTree.h>
#endif

extern "C" void count1cc_(const int &, const int &, const int &);
extern "C" void set2cc_(const int &, const int &, const int &, const int &);

const double VandleProcessor::LENGTH_SMALL_NS = 0.1*LENGTH_SMALL_CM/C_BAR_S;  // length of small bar in 10ns units
const double VandleProcessor::LENGTH_BIG_NS   = 0.1*LENGTH_BIG_CM/C_BAR_B;    // lenfth of big bar in 10ns units

using namespace dammIds::vandle;
using namespace std;

namespace dammIds { 
    namespace vandle{ 
        //HISTOGRAMS FOR INDIVIDUAL ENDS OF BARS
        const int DD_TQDCBARS         = 0;
        const int DD_MAXIMUMBARS      = 1;
        const int DD_TIMEDIFFBARS     = 2;
        const int DD_TOFBARS          = 3;
        const int DD_CORTOFBARS       = 4;
        const int DD_TQDCBETA         = 5;
        const int DD_MAXBETA          = 6;

        //HISTOGRAMS FOR EACH BAR
        const int DD_TQDCAVEVSTDIFF   = 10;

        //HISTOGRAMS FOR EACH BAR W.R.T. EACH TRIGGER
        const int DD_TDIFFVSTOF       = 30;
        const int DD_MAXRVSTOF        = 70;
        const int DD_MAXLVSTOF        = 110;
        const int DD_MAXBETA1VSTOF    = 150;
        const int DD_MAXBETA2VSTOF    = 190;
        const int DD_TQDCAVEVSTOF     = 230;

        const int DD_TDIFFVSCORTOF    = 270;
        const int DD_MAXRVSCORTOF     = 310;
        const int DD_MAXLVSCORTOF     = 350;
        const int DD_MAXBETA1VSCORTOF = 390;
        const int DD_MAXBETA2VSCORTOF = 430;
        const int DD_TDIFFVSCORCORTOF = 470;
        const int DD_TQDCAVEVSCORTOF  = 510;

        const int D_PROBLEMS  = 600;
        const int DD_PROBLEMSQDC = 601;
    }
}//vandle namespace

VandleProcessor::VandleProcessor(): EventProcessor(OFFSET, RANGE)
{
    name = "vandle";
    associatedTypes.insert("scint:beta"); //associate with scints
    associatedTypes.insert("vandle"); //associate with vandles
}

bool VandleProcessor::Init(DetectorDriver &driver)
{
    if(EventProcessor::Init(driver)){
	ReadVandleCal();
	return(true);
    }
    else
	return(false);
}

void VandleProcessor::ReadVandleCal(void) 
{
    unsigned int location;

    VandleCal vandleCal;
    
    ifstream vandleCalFile("vandleCal.txt");
    
    if (!vandleCalFile) 
        cout << "Cannot open file 'vandleCal.txt'" << endl;
    
    while(vandleCalFile) {
	if (isdigit(vandleCalFile.peek())) {
	    vandleCalFile >> location >> vandleCal.r0;
	    vandleCalFile >> vandleCal.xOffset >> vandleCal.zOffset;
	    vandleCalFile >> vandleCal.tofOffset0 >> vandleCal.tofOffset1;

	    vandleCal.z0 = 
		sqrt(vandleCal.r0*vandleCal.r0-vandleCal.xOffset*vandleCal.xOffset)-vandleCal.zOffset; //minimum distance to bar
	    
	    vandleCalMap.insert(make_pair(location, vandleCal));
	}
	else{
	    vandleCalFile.ignore(1000, '\n');
	}
    } // end while (!vandleCalFile) loop 

    
   vandleCalFile.close();
}

void VandleProcessor::DeclarePlots(void)
{
    DeclareHistogram1D(D_PROBLEMS, S5, "Problem Detectors");
    DeclareHistogram2D(DD_PROBLEMSQDC, S7, S7, "PROBLEM TRACES");
    
    //2-D Histograms
    //histograms to contain all of the detectors 
    DeclareHistogram2D(DD_TQDCBARS, SE, S5, "Det Loc vs Trace QDC");
    DeclareHistogram2D(DD_MAXIMUMBARS, SC, S5, "Det Loc vs. Maximum");
    DeclareHistogram2D(DD_TIMEDIFFBARS, S9, S5, "Bars vs. Time Differences");
    DeclareHistogram2D(DD_TOFBARS, SC, S5, "Bar vs. Time of Flight");
    DeclareHistogram2D(DD_CORTOFBARS, SC, S5, "Bar vs  Cor Time of Flight");
    DeclareHistogram2D(DD_TQDCBETA, SE, S5, "Start vs. Trace QDC");
    DeclareHistogram2D(DD_MAXBETA, SC, S5, "Start vs. Maximum");

     for(unsigned int i = 0; i < NUMBARS/2; i++) //Histograms for each end of the bars
//         DeclareHistogram2D(DD_TQDCAVEVSTDIFF+i, SC, SE,"<E> vs. Time Diff(0.5ns/bin) - Small");
    
    for(unsigned int i = 0; i <NUMBARS; i++) //Histograms that have both starts
    {
// 	DeclareHistogram2D(DD_TDIFFVSTOF+i, SC, S9,"TDiff vs. TOF(0.5ns/bin)");
// 	DeclareHistogram2D(DD_MAXRVSTOF+i, SD, SC,"MaxR vs. TOF(0.5ns/bin)");
// 	DeclareHistogram2D(DD_MAXLVSTOF+i, SD, SC,"MaxL vs. TOF(0.5ns/bin)");
// 	DeclareHistogram2D(DD_MAXBETA1VSTOF+i, SD, SC,"Max Beta Top vs. TOF(0.5ns/bin)");
// 	DeclareHistogram2D(DD_MAXBETA2VSTOF+i, SD, SC,"Max Beta Bot vs. TOF(0.5ns/bin)");
// 	DeclareHistogram2D(DD_TQDCAVEVSTOF+i, SC, SE,"<E> vs. TOF(0.5ns/bin)");

// 	DeclareHistogram2D(DD_TDIFFVSCORTOF+i, SC, S9,"TDiff vs. CorTOF(0.5ns/bin)");
// 	DeclareHistogram2D(DD_MAXRVSCORTOF+i, SD, SC,"MaxR vs. CorTOF(0.5ns/bin)");
// 	DeclareHistogram2D(DD_MAXLVSCORTOF+i, SD, SC,"MaxL vs. CorTOF(0.5ns/bin)");
//  	DeclareHistogram2D(DD_MAXBETA1VSCORTOF+i, SD, SC,"Max Beta Top vs. CorTOF(0.5ns/bin)");
//  	DeclareHistogram2D(DD_MAXBETA2VSCORTOF+i, SD, SC,"Max Beta Bot vs. CorTOF(0.5ns/bin)");
// 	DeclareHistogram2D(DD_TQDCAVEVSCORTOF+i, SC, SE,"<E> vs. CorTOF(0.5ns/bin)");
    }
}// Declare Plots

bool VandleProcessor::Process(RawEvent &event) 
{
    if (!EventProcessor::Process(event)) //ensure that there is an event and starts the process timing
	return false;

    plot(D_PROBLEMS, 30); //DEBUGGING

    if(!RetrieveData(event)) {
	EndProcess();
	return (didProcess = false);
    } else {
	AnalyzeData();
	EndProcess();
	return true;
    }
}

bool VandleProcessor::RetrieveData(RawEvent &event) 
{    
    vandleEndsMap.clear();
    vandleBarMap.clear();
    scintMap.clear();
        
    static const DetectorSummary* vandleEvents = event.GetSummary("vandle", true);
    static const DetectorSummary* scintEvents = event.GetSummary("scint:beta", true);
    
    if(vandleEvents->GetList().empty() || scintEvents->GetList().empty()) {
	plot(D_PROBLEMS, 27); //DEBUGGING
	return(false);
    }
    
    vector<ChanEvent*> allEvents;
    allEvents.insert(allEvents.end(), vandleEvents->GetList().begin(), vandleEvents->GetList().end());
    allEvents.insert(allEvents.end(), scintEvents->GetList().begin(), scintEvents->GetList().end());
    
    for(vector<ChanEvent*>::const_iterator allEventsIt = allEvents.begin();
	allEventsIt != allEvents.end(); allEventsIt++) {

	ChanEvent *chan = *allEventsIt;
	string detectorSubtype = chan->GetChanID().GetSubtype();
	
	if(detectorSubtype == "beta") {// BETA DETECTORS (START)
	    unsigned int location = chan->GetChanID().GetLocation();
	    map<int, struct VandleData>::iterator itTemp = scintMap.insert(make_pair(location, VandleData(chan))).first;
	    
	    if (GoodDataCheck((*itTemp).second))
		plot(D_PROBLEMS, 10);
	}
	else if(detectorSubtype == "small_right") { // SMALL VANDLE RIGHT
	    unsigned int location = chan->GetChanID().GetLocation();
	    map<int, struct VandleData>::iterator itTemp = vandleEndsMap.insert(make_pair(location, VandleData(chan))).first;
	    
	    if(location%2 != 0) //check that the bars are ordered properly in the map
		ImproperDetOrder();
	    
	    if (GoodDataCheck((*itTemp).second)) {
		plot(DD_TQDCBARS, (*itTemp).second.tqdc, location);
		plot(DD_MAXIMUMBARS, (*itTemp).second.maxval, location);
		
		plot(D_PROBLEMS, 12);
	    }
	}
	else if(detectorSubtype == "small_left") { //SMALL VANDLE LEFT
	    unsigned int location = chan->GetChanID().GetLocation();
	    map<int, struct VandleData>::iterator itTemp = vandleEndsMap.insert(make_pair(location, VandleData(chan))).first;
	    
	    if(location%2 == 0) //check that the bars are ordered properly in the map
		ImproperDetOrder();
	    	    
	    if (GoodDataCheck((*itTemp).second)) { 
		plot(DD_TQDCBARS, (*itTemp).second.tqdc, location);
		plot(DD_MAXIMUMBARS, (*itTemp).second.maxval, location);
		
		plot(D_PROBLEMS, 14);
	    }
	}
	else if(detectorSubtype == "big_right") {//BIG VANDLE RIGHT
	    unsigned int location = chan->GetChanID().GetLocation();
	    map<int, struct VandleData>::iterator itTemp = vandleEndsMap.insert(make_pair(location, VandleData(chan))).first;
	    
	    if(location%2 != 0) //check that the bars are ordered properly in the map
		ImproperDetOrder();
	    	    
	    if (GoodDataCheck((*itTemp).second)) { 
		plot(DD_TQDCBARS, (*itTemp).second.tqdc, location);
		plot(DD_MAXIMUMBARS, (*itTemp).second.maxval, location);
	    }
	}
	else if(detectorSubtype == "big_left") {//BIG VANDLE LEFT
		    unsigned int location = chan->GetChanID().GetLocation();
	    map<int, struct VandleData>::iterator itTemp = vandleEndsMap.insert(make_pair(location, VandleData(chan))).first;

	    if(location%2 == 0) //check that the bars are ordered properly in the map
		ImproperDetOrder();
	    
	    if (GoodDataCheck((*itTemp).second)) { 
		plot(DD_TQDCBARS, (*itTemp).second.tqdc, location);
		plot(DD_MAXIMUMBARS, (*itTemp).second.maxval, location);
	    }
	}
    } //for(vector<ChanEvent*>::const_iterator allEventsIt

    if(vandleEndsMap.size()%2 != 0) //DEBUGGING
	plot(D_PROBLEMS, 24);

    /**********MAKE SURE THAT THE VANDLE BARS HAVE MATES*********/
    for (map<int,  struct VandleData>::iterator itDetCheck = vandleEndsMap.begin(); 
	 itDetCheck != vandleEndsMap.end();)
    {
	int pairNumber = ((*itDetCheck).first%2 == 0) ? ((*itDetCheck).first+1) : ((*itDetCheck).first-1);
	
	if(vandleEndsMap.find(pairNumber) == vandleEndsMap.end())
	{
	    vandleEndsMap.erase(itDetCheck);
	    itDetCheck = vandleEndsMap.lower_bound((*itDetCheck).first+1);
	}
	else
	    itDetCheck++;
    }
    
    if(vandleEndsMap.empty())
	return(false);

    return(true);
} // bool VandleProcessor::RetrieveData

void VandleProcessor::AnalyzeData(void)
{ 
    for (map<int, VandleData>::iterator itRight = vandleEndsMap.begin(); itRight !=  vandleEndsMap.end(); itRight++) {
	if((*itRight).first%2 != 0)
	    continue;

	map<int, VandleData>::iterator itLeft = itRight;
	itLeft++;
	
	if(((*itRight).first-(*itLeft).first) == 1) //DEBUGGING
	{
	    plot(D_PROBLEMS, 21);

	    if(vandleEndsMap.size()%2 != 0)
		plot(D_PROBLEMS, 18);
	}
	
	int location = (*itRight).first/2;

	map<int, VandleCal>::iterator itCal = vandleCalMap.find(location);

	bool flagGoodVandle = (GoodDataCheck((*itLeft).second) && GoodDataCheck((*itRight).second));
	
	//MAKE THE BAR MAP
	map<int, VandleBarData>::iterator itBar = 
	    vandleBarMap.insert(make_pair(location, VandleBarData((*itRight).second, (*itLeft).second, (*itCal).second))).first;

	if(flagGoodVandle && ((*itLeft).second.maxval+(*itLeft).second.aveBaseline == 4095))	{
	    for(Trace::iterator i = (*itLeft).second.trace.begin(); i != (*itLeft).second.trace.end(); i++)
		plot(DD_PROBLEMSQDC, int(i-(*itLeft).second.trace.begin()), counter, *i);
	    
	    counter++;
	}


// 	if((*itBar).second.barQDC == 9999) //DEBUGGING
// 	{
// 	    for(vector<int>::iterator i = (*itLeft).second.trace.begin(); i != (*itLeft).second.trace.end(); i++)
// 		plot(DD_PROBLEMSQDC, int(i-(*itLeft).second.trace.begin()), counter, *i);
	    
// 	    counter++;
	    
// 	    for(vector<int>::iterator i = (*itRight).second.trace.begin(); i != (*itRight).second.trace.end(); i++)
// 		plot(DD_PROBLEMSQDC, int(i-(*itRight).second.trace.begin()), counter, *i);
	    
// 	    counter++;
// 	} 

	if(flagGoodVandle && BarEventCheck((*itBar).second.timeDiff, (*itBar).second.barType)) {
	    plot(DD_TIMEDIFFBARS, (*itBar).second.timeDiff*20+200, location); //*20 = 0.5ns/bin
	    plot(DD_TQDCAVEVSTDIFF+(*itBar).first, (*itBar).second.timeDiff*20+200, (*itBar).second.barQDC);
	    
	    for(map<int, VandleData>::iterator itStart = scintMap.begin(); itStart != scintMap.end(); itStart++) {// operations w.r.t triggers 
		int barPlusStartLoc = location*2 + (*itStart).first;
		const int resMult = 20;
		const int resOffSet = 2000;
		double tofOffset = 2000;

		if((*itStart).first == 0)
		    tofOffset = (*itCal).second.tofOffset0;
		else 
		    tofOffset = (*itCal).second.tofOffset1;
		
		double TOF = (*itBar).second.timeAve - (*itStart).second.highResTime - tofOffset; //in 10ns
		double corTOF = CorrectTOF(TOF, (*itBar).second.corRadius, (*itCal).second.z0, (*itBar).second.barType); // in ns

		if(GoodDataCheck((*itStart).second)) {
		    plot(DD_TOFBARS, TOF*resMult+resOffSet, barPlusStartLoc);
		    plot(DD_TDIFFVSTOF+barPlusStartLoc, TOF*resMult+resOffSet, (*itBar).second.timeDiff*resMult+200);
		    plot(DD_MAXRVSTOF+barPlusStartLoc, TOF*resMult+resOffSet, (*itRight).second.maxval);
		    plot(DD_MAXLVSTOF+barPlusStartLoc, TOF*resMult+resOffSet, (*itLeft).second.maxval);
		    plot(DD_TQDCAVEVSTOF+barPlusStartLoc, TOF*resMult+resOffSet, (*itBar).second.barQDC);

		    plot(DD_CORTOFBARS, corTOF*2+resOffSet, barPlusStartLoc); //*2 for 0.5ns/bin 
		    plot(DD_TDIFFVSCORTOF+barPlusStartLoc, corTOF*2+resOffSet, (*itBar).second.timeDiff);
		    plot(DD_MAXRVSCORTOF+barPlusStartLoc, corTOF*2+resOffSet, (*itRight).second.maxval);
		    plot(DD_MAXLVSCORTOF+barPlusStartLoc, corTOF*2+resOffSet, (*itLeft).second.maxval);
		    plot(DD_TQDCAVEVSCORTOF+barPlusStartLoc, corTOF*2+resOffSet, (*itBar).second.barQDC);

		    if((*itStart).first == 0) {
			plot(DD_MAXBETA1VSTOF, TOF*resMult+resOffSet, (*itStart).second.maxval);
			plot(DD_MAXBETA1VSCORTOF, corTOF*2+resOffSet, (*itStart).second.maxval);
		    }
		    else if ((*itStart).first == 1) {
			plot(DD_MAXBETA2VSCORTOF, corTOF*2+resOffSet, (*itStart).second.maxval);
			plot(DD_MAXBETA2VSCORTOF, corTOF*2+resOffSet, (*itStart).second.maxval);
		    }

		    vandledataroot.maxRight = (*itRight).second.maxval;
		    vandledataroot.maxLeft  = (*itLeft).second.maxval;
		    vandledataroot.qdcRight = (*itRight).second.tqdc;
		    vandledataroot.qdcLeft  = (*itLeft).second.tqdc;
		    vandledataroot.timeDiff = (*itBar).second.timeDiff;
		    vandledataroot.TOF      = TOF;
		}
	    }
	} //if(fabs(timeDiff) < LENGTH_SMALL_NS)
    } //(map<int,struct VandleData>::iterator itRight
} //void VandleProcessor::AnalyzeData

bool VandleProcessor::BarEventCheck(const double &timeDiff, const string &barType)
{
    if((barType == "small") && (fabs(timeDiff) < LENGTH_SMALL_NS+20))
	return(true);
    else if((barType == "big") && (fabs(timeDiff) < LENGTH_BIG_NS+20))
	return(true);
    else 
	return(false);
}

double VandleProcessor::VandleBarData::BendBar(double &timeDiff, const VandleCal& Cal, const string &barType)
{
    double radius = 0;

   if(barType == "smalTrace &traceTrace &traceTrace &tracel")
	radius = sqrt(Cal.z0*Cal.z0+pow(C_BAR_S*0.5*10*timeDiff-Cal.xOffset,2)); 
    else if(barType == "big")
	radius = sqrt(Cal.z0*Cal.z0+pow(C_BAR_B*0.5*10*timeDiff-Cal.xOffset,2));
    
    return(radius);
}

double VandleProcessor::CorrectTOF(double &TOF, double &corRadius, const double &z0, const string &barType)
{
    double corTOF = 0;
    
    if(barType == "small")
	corTOF = (z0*(TOF*10)/corRadius); 
    else if(barType == "big")
	corTOF = (z0*(TOF*10)/corRadius); 

    return(corTOF);
}

bool VandleProcessor::GoodDataCheck(const VandleData& DataCheck)
{
    if((DataCheck.maxval != -9999) && (DataCheck.phase !=-9999) && (DataCheck.tqdc !=-9999) && (DataCheck.highResTime != -9999))
	return(true);
    else
	return(false);
}

void VandleProcessor::ImproperDetOrder(void)
{
    cout << endl << endl << "ERROR: For VANDLE Analysis you must order the detectors Right(even)/Left(odd)" << endl << endl; 
    exit(EXIT_FAILURE);
}

double VandleProcessor::InverseVelocity (const double &corTOF, const double &z0) 
{
    return(corTOF/z0);
}

VandleProcessor::VandleData::VandleData(string type)
{
    detSubtype     = type;
    maxval         = -9999;
    maxpos         = -9999;
    phase          = -9999;
    tqdc           = -9999;
    stdDevBaseline = -9999;
    aveBaseline    = -9999;
    highResTime    = -9999;
}

VandleProcessor::VandleData::VandleData(ChanEvent *chan)
{
    detSubtype     = chan->GetChanID().GetSubtype();
    tqdc           = trace.GetValue("tqdc");
    maxval         = trace.GetValue("maxval");
    maxpos         = trace.GetValue("maxpos");
    phase          = trace.GetValue("phase");
    stdDevBaseline = trace.GetValue("baseline");
    aveBaseline    = trace.GetValue("sigmaBaseline");
    highResTime    = chan->GetHighResTime();
}

VandleProcessor::VandleBarData::VandleBarData(const VandleData& Right, const VandleData& Left, const VandleCal& Cal) 
{
    barQDC = sqrt(Right.tqdc*Left.tqdc);

    if(Right.detSubtype == "small_right")
	barType = "small";
    else if(Right.detSubtype == "big_right")
	barType = "big";

    timeAve = (Right.highResTime + Left.highResTime)*0.5; 
    timeDiff = Left.highResTime-Right.highResTime;
    corRadius = BendBar(timeDiff, Cal, barType); //in cm
}

VandleProcessor::VandleDataRoot::VandleDataRoot()
{
    maxRight  = -9999;
    maxLeft   = -9999;
    qdcRight  = -9999;
    qdcLeft   = -9999;
    timeDiff  = -9999;
    TOF       = -9999;
}

#ifdef useroot
bool VandleProcessor::AddBranch(TTree *tree)
{
  if (tree) {
    TBranch *vandleBranch = 
	tree->Branch(name.c_str(), &vandledataroot, "maxRight/D:maxLeft:qdcRight:qdcLeft:timeDiff:TOF");

    return (vandleBranch != NULL);
  } 
  return false;
}

void VandleProcessor::FillBranch(void)
{
  if (!HasEvent())
      VandleDataRoot();  
}
#endif //ifdef useroot
