/** \file MtcProcessor.cpp
 *
 * handling of mtc events
 * derived from timeclass.cpp
 * doesn't handle old style NSCL correlations
 *
 * Start subtype corresponds to leading edge of tape move signal
 * Stop subtype corresponds to trailing edge of tape move signal
 */

#include <iostream>

#include <cmath>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "MtcProcessor.hpp"

using namespace std;
using namespace dammIds::mtc;

namespace dammIds {
    namespace mtc {
        const int D_TDIFF0        = 0;
        const int D_TDIFF1        = 1;
        const int D_TDIFFSUM      = 2;
        const int D_MOVETIME      = 3;
        const int D_COUNTER       = 10;
        const int D_COUNTER_MOVE0 = 11;
        const int D_COUNTER_MOVE1 = 12;
    }
} // mtc namespace


MtcProcessor::MtcProcessor(void) : EventProcessor(OFFSET, RANGE), 
				   lastStartTime(NAN), lastStopTime(NAN)
{
    name = "mtc";

    associatedTypes.insert("timeclass"); // old detector type
    associatedTypes.insert("mtc");
}

void MtcProcessor::DeclarePlots(void)
{
    using namespace dammIds::mtc;
    
    const int counterBins = S4;
    const int timeBins = SA;

    DeclareHistogram1D(D_TDIFF0, timeBins, "tdiff btwn MTC starts, 10 ms/bin");
    DeclareHistogram1D(D_TDIFF1, timeBins, "tdiff btwn MTC stops, 10 ms/bin");
    DeclareHistogram1D(D_TDIFFSUM, timeBins, "sum tdiff btwn moves, 10 ms/bin");
    DeclareHistogram1D(D_MOVETIME, timeBins, "move time, 10 ms/bin");
    DeclareHistogram1D(D_COUNTER, counterBins, "MTC counter");
    DeclareHistogram1D(D_COUNTER_MOVE0, counterBins, "MTC1 counter");
    DeclareHistogram1D(D_COUNTER_MOVE1, counterBins, "MTC2 counter");
}

bool MtcProcessor::Process(RawEvent &event)
{
    // plot with 10 ms bins
    const double mtcPlotResolution = 10e-3 / pixie::clockInSeconds;
    static Correlator &corr = event.GetCorrelator();
 
    if (!EventProcessor::Process(event))
	return false;

    using namespace dammIds::mtc;
 
    const static DetectorSummary *mtcSummary = NULL;

    if (mtcSummary == NULL) {
	if ( sumMap.count("mtc") )
	    mtcSummary = sumMap["mtc"];
	else if ( sumMap.count("timeclass") ) 
	    mtcSummary = sumMap["timeclass"];
    }

    static const vector<ChanEvent*> &mtcEvents = mtcSummary->GetList();

    plot(D_COUNTER, dammIds::GENERIC_CHANNEL);

    for (vector<ChanEvent*>::const_iterator it = mtcEvents.begin();
	 it != mtcEvents.end(); it++) {
	ChanEvent *chan = *it;
	string subtype = chan->GetChanID().GetSubtype();

	double time = chan->GetTime();
	if(subtype == "start") {
	    if (!isnan(lastStartTime)) {
		double timediff = time - lastStartTime;
		plot(D_TDIFF0, timediff / mtcPlotResolution);
		plot(D_TDIFFSUM, timediff / mtcPlotResolution);
	    }
	    lastStartTime = time;
	    plot(D_COUNTER_MOVE0,dammIds::GENERIC_CHANNEL); //counter
	} else if (subtype == "stop") {
	    if (!isnan(lastStopTime) != 0) {
		double timeDiff1 = time - lastStopTime;
		plot(D_TDIFF1, timeDiff1 / mtcPlotResolution);
		plot(D_TDIFFSUM, timeDiff1 / mtcPlotResolution);
		if (!isnan(lastStartTime)) {
		    double moveTime = time - lastStartTime;    
		    plot(D_MOVETIME, moveTime / mtcPlotResolution);
		}
	    }
	    lastStopTime = time;
	    plot(D_COUNTER_MOVE1,dammIds::GENERIC_CHANNEL); //counter	  
	    // correlate the end of tape movement with the implantation time
	    // if mtc down, correlate with beam_start

	    EventInfo corEvent;
	    corEvent.time = time;
	    corEvent.type = EventInfo::IMPLANT_EVENT;

	    corr.Correlate(corEvent, 1, 1);

	} else if (subtype == "beam_start") {
	    /*
	    corr.Correlate(event, Correlator::IMPLANT_EVENT,
			   1, 1, time);
	    */
	}
    }



    EndProcess(); // update processing time
    return true;
}
