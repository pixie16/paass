/*****************************************
Source File for the TeenyVandleProcessor
   S.V. Paulauskas 02 November 2011
********************************************/
#include <fstream>
#include <iostream>

#include <cmath>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "TeenyVandleProcessor.hpp"
#include "RawEvent.hpp"

namespace dammIds {
   namespace teenyvandle {
      const int D_TIMEDIFF     = 0; 
      const int D_PROBLEMSTUFF = 1; 
      
      const int DD_QDC         = 2; 
      const int DD_MAX         = 3; 
      const int DD_PVSP        = 4; 
      const int DD_MAXVSTDIFF  = 5; 
      const int DD_QDCVSMAX    = 6; 
      const int DD_AMPMAPSTART = 7; 
      const int DD_AMPMAPSTOP  = 8; 
      const int D_SNRSTART     = 9; 
      const int D_SNRSTOP      = 10; 
      const int D_SDEVBASESTART= 11; 
      const int D_SDEVBASESTOP = 12; 
      const int DD_PROBLEMS    = 13; 
      const int DD_MAXSVSTDIFF = 14; 
   }
}

using namespace std;
using namespace dammIds::teenyvandle;

TeenyVandleProcessor::TeenyVandleProcessor(): EventProcessor(OFFSET, RANGE)
{
    name = "TeenyVandle";
    associatedTypes.insert("tvandle"); //associate with tvandle

    counter = 0;
}

void TeenyVandleProcessor::DeclarePlots(void)
{
    DeclareHistogram1D(D_TIMEDIFF, SE, "Time Difference");
    DeclareHistogram1D(D_PROBLEMSTUFF, S5, "Problem Stuff");

    DeclareHistogram2D(DD_QDC, SE, S1,"QDC");
    DeclareHistogram2D(DD_MAX, SC, S1, "Max");
    DeclareHistogram2D(DD_PVSP, SE, SE,"Phase vs. Phase");
    DeclareHistogram2D(DD_MAXVSTDIFF, SA, SC,"Max Right vs. Time Diff");
    DeclareHistogram2D(DD_QDCVSMAX, SC, SE,"QDC vs Max - Right");
    //DeclareHistogram2D(DD_AMPMAPSTART, S7, SC,"Amp Map Start");
    //DeclareHistogram2D(DD_AMPMAPSTOP, S7, SC,"Amp Map Stop");
    DeclareHistogram1D(D_SNRSTART, SE, "SNR - Right");
    DeclareHistogram1D(D_SNRSTOP, SE, "SNR - Left");
    DeclareHistogram1D(D_SDEVBASESTART, S8, "Sdev Base - Right");
    DeclareHistogram1D(D_SDEVBASESTOP, S8, "Sdev Base - Left");
    DeclareHistogram2D(DD_PROBLEMS, SB, S5, "Problems - 2D");
    DeclareHistogram2D(DD_MAXSVSTDIFF, SD, SC, "Max Left vs. Time Diff");
}

bool TeenyVandleProcessor::Process(RawEvent &event) 
{
    if (!EventProcessor::Process(event))
	return false;

    plot(D_PROBLEMSTUFF, 30);

    if(!RetrieveData(event)) {
	EndProcess();
	return (didProcess = false);
    } else {
	AnalyzeData();
	EndProcess();
	return true;
    }
}

bool TeenyVandleProcessor::RetrieveData(RawEvent &event)
{   
    tVandleMap.clear();

    static const vector<ChanEvent*> & tVandleEvents = 
	event.GetSummary("tvandle")->GetList();

    for(vector<ChanEvent*>::const_iterator itTvandle = tVandleEvents.begin();
	itTvandle != tVandleEvents.end(); itTvandle++) {
	
	unsigned int location = (*itTvandle)->GetChanID().GetLocation();
	string subType = (*itTvandle)->GetChanID().GetSubtype();

	IdentKey tVandleKey(location, subType);
	tVandleMap.insert(make_pair(tVandleKey, TimingData(*itTvandle)));
    }
    
    if(tVandleMap.empty() || tVandleMap.size()%2 != 0) {
	plot(D_PROBLEMSTUFF, 27);
	return(false);
    } else {
	return(true);
    }
}//bool TeenyVandleProcessor::RetrieveData

void TeenyVandleProcessor::AnalyzeData(void)
{
    IdentKey rightKey(0,"right");
    IdentKey leftKey (1,"left");

    TimingDataMap::iterator itRight = tVandleMap.find(rightKey);
    TimingDataMap::iterator itLeft  = tVandleMap.find(leftKey);
    
    unsigned int maxPosRight = (unsigned int)(*itRight).second.maxpos;
    unsigned int maxPosLeft  = (unsigned int)(*itLeft).second.maxpos;
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
    plot(DD_QDC, (*itRight).second.tqdc, 
	 (*itRight).first.first);
    plot(DD_MAX, (*itRight).second.maxval, 
	 (*itRight).first.first);
    plot(DD_QDC, (*itLeft).second.tqdc, 
	 (*itLeft).first.first);
    plot(DD_MAX, (*itLeft).second.maxval, 
	 (*itLeft).first.first);
    plot(DD_QDCVSMAX, (*itRight).second.maxval, 
	 (*itRight).second.tqdc);
   
    if((*itRight).second.dataValid && 
       (*itLeft).second.dataValid){
	double timeRes = 50; //100 ps/bin
	double timeOff = 2000; 

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
} // void TeenyVandleProcessor::AnalyzeData
