/*****************************************
Source File for the PulserProcessor
   S.V. Paulauskas 10 July 2009
********************************************/
#include <fstream>
#include <iostream>

#include <cmath>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "PulserProcessor.hpp"
#include "RawEvent.hpp"

extern "C" void count1cc_(const int &, const int &, const int &);
extern "C" void set2cc_(const int &, const int &, const int &, 
			const int &);

namespace dammIds {
   const int D_TIMEDIFF     = 4600; 
   const int D_PROBLEMSTUFF = 4601; 
   
   const int DD_QDC         = 4602; 
   const int DD_MAX         = 4603; 
   const int DD_PVSP        = 4604; 
   const int DD_MAXVSTDIFF  = 4605; 
   const int DD_QDCVSMAX    = 4606; 
   const int DD_AMPMAPSTART = 4607; 
   const int DD_AMPMAPSTOP  = 4608; 
   const int D_SNRSTART     = 4609; 
   const int D_SNRSTOP      = 4610; 
   const int D_SDEVBASESTART= 4611; 
   const int D_SDEVBASESTOP = 4612; 
   const int DD_PROBLEMS    = 4613; 
   const int DD_MAXSVSTDIFF = 4614; 
}


using namespace std;
using namespace dammIds;

PulserProcessor::PulserProcessor(): EventProcessor()
{
    name = "Pulser";
    associatedTypes.insert("pulser"); //associate with pulser
}

void PulserProcessor::DeclarePlots(void)
{
    DeclareHistogram1D(D_TIMEDIFF, SA, "Time Difference");
    DeclareHistogram1D(D_PROBLEMSTUFF, S5, "Problem Stuff");

    DeclareHistogram2D(DD_QDC, SE, S1,"QDC");
    DeclareHistogram2D(DD_MAX, SC, S1, "Max");
    DeclareHistogram2D(DD_PVSP, S9, S9,"Phase vs. Phase");
    DeclareHistogram2D(DD_MAXVSTDIFF, SA, SC, 
		       "Max vs. Time Diff");
    DeclareHistogram2D(DD_QDCVSMAX, SC, SE,"QDC vs Max");
    //DeclareHistogram2D(DD_AMPMAPSTART, S7, SC,"Amp Map Start");
    //DeclareHistogram2D(DD_AMPMAPSTOP, S7, SC,"Amp Map Stop");

    DeclareHistogram1D(D_SNRSTART, SE, "SNR - Start");
    DeclareHistogram1D(D_SNRSTOP, SE, "SNR - Stop");

    DeclareHistogram1D(D_SDEVBASESTART, S8, "Sdev Base - Start");
    DeclareHistogram1D(D_SDEVBASESTOP, S8, "Sdev Base - Stop");

    DeclareHistogram2D(DD_PROBLEMS, SB, S5, "Problems - 2D");
}

bool PulserProcessor::Process(RawEvent &event) 
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

bool PulserProcessor::RetrieveData(RawEvent &event)
{   
    pulserMap.clear();

    static const vector<ChanEvent*> & pulserEvents = 
	event.GetSummary("pulser")->GetList();

    for(vector<ChanEvent*>::const_iterator itPulser = pulserEvents.begin();
	itPulser != pulserEvents.end(); itPulser++) {
	
	unsigned int location = (*itPulser)->GetChanID().GetLocation();
	string subType = (*itPulser)->GetChanID().GetSubtype();

	IdentKey pulserKey(location, subType);
	pulserMap.insert(make_pair(pulserKey, TimingData(*itPulser)));
    }
    
    if(pulserMap.empty() || pulserMap.size()%2 != 0) {
	plot(D_PROBLEMSTUFF, 27);
	return(false);
    } else {
	return(true);
    }
}//bool PulserProcessor::RetrieveData

void PulserProcessor::AnalyzeData(void)
{
    IdentKey startKey(0,"start");
    IdentKey stopKey (1,"stop");

    TimingDataMap::iterator itStart = pulserMap.find(startKey);
    TimingDataMap::iterator itStop  = pulserMap.find(stopKey);
    
    unsigned int maxPosStart = (unsigned int)(*itStart).second.maxpos;
    unsigned int maxPosStop = (unsigned int)(*itStop).second.maxpos;

    unsigned int cutVal = 15;
    if((*itStart).second.maxpos == 41)
    if((*itStart).second.maxval < 2384-cutVal)
	for(Trace::const_iterator it = (*itStart).second.trace.begin(); 
	    it != (*itStart).second.trace.end(); it++)
	    plot(DD_AMPMAPSTART, int(it-(*itStart).second.trace.begin()), *it);
    
    if((*itStop).second.maxval < 2555-cutVal)
	for(Trace::const_iterator it = (*itStart).second.trace.begin(); 
	    it != (*itStart).second.trace.end(); it++)
	    plot(DD_AMPMAPSTOP, int(it-(*itStart).second.trace.begin()), *it);
    
    double timeDiff = 
	(*itStop).second.highResTime - (*itStart).second.highResTime;

    //Fill histograms
    if((*itStart).second.dataValid && 
       (*itStop).second.dataValid){
	
	double timeRes = 10; //100 ps/bin
	double timeOff = 100; 

	plot(D_TIMEDIFF, timeDiff*timeRes + timeOff);
	plot(DD_QDC, (*itStart).second.tqdc, 
	     (*itStart).first.first);
	plot(DD_MAX, (*itStart).second.maxval, 
	     (*itStart).first.first);
	plot(DD_QDC, (*itStop).second.tqdc, 
	     (*itStop).first.first);
	plot(DD_MAX, (*itStop).second.maxval, 
	     (*itStop).first.first);
	plot(DD_PVSP, (*itStart).second.phase*timeRes-4000, 
	     (*itStop).second.phase*timeRes-4000);
	plot(DD_MAXVSTDIFF, timeDiff*timeRes+timeOff, 
	     (*itStart).second.maxval);
	plot(DD_QDCVSMAX, (*itStart).second.maxval, 
	     (*itStart).second.tqdc);

	plot(D_SNRSTART, (*itStart).second.signalToNoise*0.25);
	plot(D_SNRSTOP, (*itStop).second.signalToNoise*0.25);
	plot(D_SDEVBASESTART, (*itStart).second.stdDevBaseline*timeRes+timeOff);
	plot(D_SDEVBASESTOP, (*itStop).second.stdDevBaseline*timeRes+timeOff);
    }
} // void PulserProcessor::AnalyzeData
