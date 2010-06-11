/** \file MtcProcessor.cpp
 *
 * handling of mtc events
 * derived from timeclass.cpp
 * doesn't handle old style NSCL correlations
 *
 * Location 0 refers to the discriminated leading edge of the tape signal
 * Location 1 refers to the discriminated trailing edge of the tape signal
 */

#include <iostream>

#include "damm_plotids.h"
#include "param.h"
#include "RawEvent.h"
#include "MtcProcessor.h"

using namespace std;

MtcProcessor::MtcProcessor(void) : EventProcessor(), 
				   prevTime0(0.), prevTime1(0.)
{
    name = "mtc";

    associatedTypes.insert("timeclass"); // old detector type
    associatedTypes.insert("mtc");
}

void MtcProcessor::DeclarePlots(void) const
{
    using namespace dammIds::mtc;
    
    const int timeBins = SA;

    DeclareHistogram1D(D_TDIFF0, timeBins, "tdiff btwn MTC0 sigs, 10 ms/bin");
    DeclareHistogram1D(D_TDIFF1, timeBins, "tdiff btwn MTC1 sigs, 10 ms/bin");
    DeclareHistogram1D(D_TDIFFSUM, timeBins, "tdiff btwn MTC01 sigs, 10 ms/bin");
    DeclareHistogram1D(D_MOVETIME, timeBins, "tdiff btwn MTC1/2 sigs, 10 ms/bin");
    DeclareHistogram1D(D_COUNTER, timeBins, "MTC1/2 counter");
    DeclareHistogram1D(D_COUNTER_MOVE0, timeBins, "MTC1 counter");
    DeclareHistogram1D(D_COUNTER_MOVE1, timeBins, "MTC2 counter");
}

bool MtcProcessor::Process(RawEvent &event)
{
    // plot with 10 ms bins
    const double mtcPlotResolution = 10e-3 / pixie::clockInSeconds;

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

    plot(D_COUNTER, GENERIC_CHANNEL);

    double time0, time1;

    for (vector<ChanEvent*>::const_iterator it = mtcEvents.begin();
	 it != mtcEvents.end(); it++) {
	ChanEvent *chan = *it;
	int detNum = chan->GetChanID().GetLocation();
      
	if(detNum == 0) {
	    time0 = chan->GetTime();
	    double timeDiff0 = time0 - prevTime0;
	    prevTime0 = time0;

	    plot(D_COUNTER_MOVE0,GENERIC_CHANNEL); //counter
	    plot(D_TDIFF0, timeDiff0 / mtcPlotResolution);
	    plot(D_TDIFFSUM, timeDiff0 / mtcPlotResolution);
	} else if (detNum == 1) {
	    time1 = chan->GetTime();
	    double timeDiff1 = time1 - prevTime1;
	    prevTime1 = time1;
	  
	    plot(D_COUNTER_MOVE1,GENERIC_CHANNEL); //counter
	    plot(D_TDIFF1, timeDiff1 / mtcPlotResolution);
	    plot(D_TDIFFSUM, timeDiff1 / mtcPlotResolution);

	    double moveTime = prevTime1 - prevTime0;    
	    plot(D_MOVETIME, moveTime / mtcPlotResolution);
	}
    }



    EndProcess(); // update processing time
    return true;
}
