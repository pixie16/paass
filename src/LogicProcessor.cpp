/** \file MtcProcessor.cpp
 *
 * handling of logic events
 * derived from MtcProcessor.cpp
 *
 * Start subtype corresponds to leading edge
 * Stop subtype corresponds to trailing edge
 */

#include <iostream>
#include <string>
#include <vector>

#include "damm_plotids.h"
#include "param.h"
#include "RawEvent.h"
#include "LogicProcessor.h"

using namespace std;

LogicProcessor::LogicProcessor(void) : 
  EventProcessor(), lastStartTime(dammIds::logic::MAX_LOGIC, NAN), lastStopTime(dammIds::logic::MAX_LOGIC, NAN),
  logicStatus(dammIds::logic::MAX_LOGIC)
{
    name = "logic";

    associatedTypes.insert("logic");
}

void LogicProcessor::DeclarePlots(void) const
{
    using namespace dammIds::logic;
    
    const int counterBins = S4;
    const int timeBins = SC;

    DeclareHistogram1D(D_COUNTER_START, counterBins, "logic start counter");
    DeclareHistogram1D(D_COUNTER_STOP, counterBins, "logic stop counter");
    for (int i=0; i < MAX_LOGIC; i++) {
      DeclareHistogram1D(D_TDIFF_STARTX + i, timeBins, "tdiff btwn logic starts, 10 us/bin");
      DeclareHistogram1D(D_TDIFF_STOPX + i, timeBins, "tdiff btwn logic stops, 10 us/bin");
      DeclareHistogram1D(D_TDIFF_SUMX + i, timeBins, "tdiff btwn both logic, 10 us/bin");
      DeclareHistogram1D(D_TDIFF_LENGTHX + i, timeBins, "logic high time, 10 us/bin");
    }
}

bool LogicProcessor::Process(RawEvent &event)
{
    const double logicPlotResolution = 10e-6 / pixie::clockInSeconds;
 
    if (!EventProcessor::Process(event))
	return false;

    using namespace dammIds::logic;

    static const vector<ChanEvent*> &events = sumMap["logic"]->GetList();

    for (vector<ChanEvent*>::const_iterator it = events.begin();
	 it != events.end(); it++) {
	ChanEvent *chan = *it;

	string subtype   = chan->GetChanID().GetSubtype();
	unsigned int loc = chan->GetChanID().GetLocation();
	double time = chan->GetTime();

	if(subtype == "start") {
	    if (!isnan(lastStartTime.at(loc))) {
	        double timediff = time - lastStartTime.at(loc);
	      
		plot(D_TDIFF_STARTX + loc, timediff / logicPlotResolution);
		plot(D_TDIFF_SUMX + loc,   timediff / logicPlotResolution);
	    }

	    //? bounds checking
	    lastStartTime.at(loc) = time;
	    logicStatus.at(loc) = true;

	    plot(D_COUNTER_START, loc);
	} else if (subtype == "stop") {
  	    if (!isnan(lastStopTime.at(loc))) {
		double timediff = time - lastStopTime.at(loc);
		plot(D_TDIFF_STOPX + loc, timediff / logicPlotResolution);
		plot(D_TDIFF_SUMX + loc,  timediff / logicPlotResolution);
		if (!isnan(lastStartTime.at(loc))) {
  		    double moveTime = time - lastStartTime.at(loc);    
		    plot(D_TDIFF_LENGTHX + loc, moveTime / logicPlotResolution);
		}
	    }
	    //? bounds checking
	    lastStopTime.at(loc) = time;
	    logicStatus.at(loc) = false;
	    plot(D_COUNTER_STOP, loc);	  
	}
    }

    EndProcess(); // update processing time
    return true;
}
