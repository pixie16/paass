/*****************************************
Source File for the PulserProcessor
   S.V.P. 10 July 2009
********************************************/
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <unistd.h> //not standard c/c++, makes compliant with UNIX standards
#include <vector>

#include "damm_plotids.h"
#include "DetectorDriver.h"
#include "PulserProcessor.h"
#include "RawEvent.h"

extern "C" void count1cc_(const int &, const int &, const int &);
extern "C" void set2cc_(const int &, const int &, const int &, const int &);

using namespace std;
using namespace dammIds::pulserprocessor;

PulserProcessor::PulserProcessor(): EventProcessor()
{
    name = "Pulser";
    associatedTypes.insert("pulser"); //associate with pulser
}

void PulserProcessor::DeclarePlots(void) const
{
    DeclareHistogram1D(D_TIMEDIFF, SA, "Time Difference");
    DeclareHistogram1D(D_PROBLEMSTUFF, S5, "Problem Stuff");

    DeclareHistogram2D(DD_QDC, SE, S1,"QDC");
    DeclareHistogram2D(DD_MAX, SC, S1, "Max");
    DeclareHistogram2D(DD_PVSP, S9, S9,"Phase vs. Phase");
    DeclareHistogram2D(DD_MAXVSTDIFF, SA, SC, "Max vs. Time Diff");
    DeclareHistogram2D(DD_QDCVSMAX, SC, SE,"QDC vs Max");
    DeclareHistogram2D(DD_AMPMAPSTART, S7, SC,"Amp Map Start");
    DeclareHistogram2D(DD_AMPMAPSTOP, S7, SC,"Amp Map Stop");
}

bool PulserProcessor::Process(RawEvent &event) 
{
    if (!EventProcessor::Process(event)) //ensure that there is an event and starts the process timing
	return false;

    plot(D_PROBLEMSTUFF, 30);

    if(!RetrieveData(event))
    {
	EndProcess();
	return(false);
    }
    else
    {
	AnalyzeData();
	EndProcess();
	return(true);
    }
}

bool PulserProcessor::RetrieveData(RawEvent &event)
{   
    pulserMap.clear();

    static const DetectorSummary* pulserSummary = event.GetSummary("pulser");

    if(pulserSummary)
	for(vector<ChanEvent*>::const_iterator itPulser = pulserSummary->GetList().begin();
	    itPulser != pulserSummary->GetList().end(); itPulser++)
	{
	    ChanEvent *chan = *itPulser;
	    
	    string detSubtype = chan->GetChanID().GetSubtype();
	    pulserMap.insert(make_pair(detSubtype, PulserData(chan)));
	}
    
    if(pulserMap.empty() || pulserMap.size()%2 != 0)
    {
	plot(D_PROBLEMSTUFF, 27);
	return(false);
    }
    else
	return(true);
}//bool PulserProcessor::RetrieveData

void PulserProcessor::AnalyzeData(void)
{
    map<string, PulserData>::iterator itStart = pulserMap.find("start");
    map<string, PulserData>::iterator itStop  = pulserMap.find("stop");

    int maxPosStart = (*itStart).second.maxPos;

    if((*itStart).second.maxPos ==41)
	//if(((*itStart).second.trace.at(maxPosStart+1) > 1466) && ((*itStart).second.trace.at(maxPosStart+1) < 1506))
	    for(vector<int>::iterator i = (*itStart).second.trace.begin(); i != (*itStart).second.trace.end(); i++)
		plot(DD_AMPMAPSTART, int(i-(*itStart).second.trace.begin()), *i);
    
    if((*itStart).second.maxPos ==42)
	//if(((*itStart).second.trace.at(maxPosStart+1) > 1466) && ((*itStart).second.trace.at(maxPosStart+1) < 1506))
	    for(vector<int>::iterator i = (*itStart).second.trace.begin(); i != (*itStart).second.trace.end(); i++)
		plot(DD_AMPMAPSTOP, int(i-(*itStart).second.trace.begin()), *i);
    
    double timeDiff = (*itStop).second.highResTime - (*itStart).second.highResTime;
        
    //Create the histograms
    if(GoodDataCheck((*itStart).second) && (GoodDataCheck((*itStop).second)))
    {
	plot(D_TIMEDIFF, timeDiff*500+500);
	
	plot(DD_QDC, (*itStart).second.trcQDC, (*itStart).second.location);
	plot(DD_MAX, (*itStart).second.maxValue, (*itStart).second.location);
	
	plot(DD_QDC, (*itStop).second.trcQDC, (*itStop).second.location);
	plot(DD_MAX, (*itStop).second.maxValue, (*itStop).second.location);
	
	plot(DD_PVSP, (*itStart).second.phase*100-4000, (*itStop).second.phase*100-4000);
	plot(DD_MAXVSTDIFF, timeDiff*500+500, (*itStart).second.maxValue);
	plot(DD_QDCVSMAX, (*itStart).second.maxValue, (*itStart).second.trcQDC);
    }
} // void PulserProcessor::AnalyzeData

bool PulserProcessor::GoodDataCheck(const PulserData& DataCheck)
{
    if((DataCheck.maxValue != -9999) && (DataCheck.phase !=-9999) && (DataCheck.trcQDC !=-9999) && (DataCheck.highResTime != -9999))
	return(true);
    else
	return(false);
}

PulserProcessor::PulserData::PulserData(string type)
{
    location       = -9999;
    maxValue       = -9999;
    phase          = -9999;
    trcQDC         = -9999;
    stdDevBaseline = -9999;
    aveBaseline    = -9999;
    highResTime    = -9999;
    maxPos         = -9999;
    trace.clear();
}

PulserProcessor::PulserData::PulserData(ChanEvent* chan)
{
    location       = chan->GetChanID().GetLocation();
    trcQDC         = chan->GetTrcQDC();
    maxValue       = chan->GetMaxValue();	     
    phase          = chan->GetPhase();
    stdDevBaseline = chan->GetStdDevBaseline();
    aveBaseline    = chan->GetAveBaseline();
    highResTime    = chan->GetPhase() + chan->GetTime();
    maxPos         = chan->GetMaxPos();
    trace          = chan->GetTraceRef();
}
