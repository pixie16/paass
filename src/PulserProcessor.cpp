/** \file PulserProcessor.cpp
 * \brief Analyzes pulser signals
 *
 * Analyzes pulser signals for electronics and high resolution
 * timing applications.
 *
 * \author S. V. Paulauskas
 * \date 10 July 2009
 */
#include <fstream>
#include <iostream>

#include <cmath>

#include "DammPlotIds.hpp"
#include "PulserProcessor.hpp"
#include "RawEvent.hpp"
#include "TimingCalibrator.hpp"

namespace dammIds {
    namespace pulser {
	const int D_TIMEDIFF     = 0;
	const int D_PROBLEMSTUFF = 1;

	const int DD_QDC         = 2;
	const int DD_MAX         = 3;
	const int DD_PVSP        = 4;
	const int DD_MAXVSTDIFF  = 5;
	const int DD_QDCVSMAX    = 6;
	const int DD_AMPMAPSTART = 7;
	const int DD_AMPMAPSTOP  = 8;
	const int DD_SNRANDSDEV  = 9;
	const int DD_PROBLEMS    = 13;
	const int DD_MAXSVSTDIFF = 14;
    }
}

using namespace std;
using namespace dammIds::pulser;

PulserProcessor::PulserProcessor(): EventProcessor(OFFSET, RANGE, "pulser") {
    associatedTypes.insert("pulser");
}

void PulserProcessor::DeclarePlots(void) {
    DeclareHistogram1D(D_TIMEDIFF, SC, "Time Difference");
    DeclareHistogram1D(D_PROBLEMSTUFF, S5, "Problem Stuff");

    DeclareHistogram2D(DD_QDC, SD, S1,"QDC");
    DeclareHistogram2D(DD_MAX, SC, S1, "Max");
    DeclareHistogram2D(DD_PVSP, SC, SC,"Phase vs. Phase");
    DeclareHistogram2D(DD_MAXVSTDIFF, SC, SC, "Max vs. Time Diff");
    DeclareHistogram2D(DD_QDCVSMAX, SC, SD,"QDC vs Max");
    //DeclareHistogram2D(DD_AMPMAPSTART, S7, SC,"Amp Map Start");
    //DeclareHistogram2D(DD_AMPMAPSTOP, S7, SC,"Amp Map Stop");
    DeclareHistogram2D(DD_SNRANDSDEV, S8, S2, "SNR and SDEV R01/L23");
    DeclareHistogram2D(DD_PROBLEMS, SB, S5, "Problems - 2D");
}

bool PulserProcessor::Process(RawEvent &event) {
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

bool PulserProcessor::RetrieveData(RawEvent &event) {
    TimingCalibrator *tcal = TimingCalibrator::get();
    pulserMap.clear();

    static const vector<ChanEvent*> & pulserEvents =
	event.GetSummary("pulser")->GetList();

    for(vector<ChanEvent*>::const_iterator itPulser = pulserEvents.begin();
	itPulser != pulserEvents.end(); itPulser++) {
        unsigned int location = (*itPulser)->GetChanID().GetLocation();
        string subType = (*itPulser)->GetChanID().GetSubtype();

        TimingInformation::IdentKey pulserKey(location, subType);
        pulserMap.insert(make_pair(pulserKey,
                                   TimingInformation::TimingData(*itPulser)));
    }

    if(pulserMap.empty() || pulserMap.size()%2 != 0) {
        plot(D_PROBLEMSTUFF, 27);
        return(false);
    } else {
        return(true);
    }
}//bool PulserProcessor::RetrieveData

void PulserProcessor::AnalyzeData(void) {
    TimingInformation::TimingData start =
        (*pulserMap.find(make_pair(0,"start"))).second;
    TimingInformation::TimingData stop  =
        (*pulserMap.find(make_pair(0,"stop"))).second;

    static int counter = 0;
    for(Trace::const_iterator it = start.trace.begin();
        it!= start.trace.end(); it++)
        plot(DD_PROBLEMS, int(it-start.trace.begin()), counter, *it);
    counter ++;

    // unsigned int cutVal = 15;
    // if(start.maxpos == 41)
    // if(start.maxval < 2384-cutVal)
    // 	for(Trace::const_iterator it = start.trace.begin();
    // 	    it != start.trace.end(); it++)
    // 	    plot(DD_AMPMAPSTART, int(it-start.trace.begin()), *it);

    // if(stop.maxval < 2555-cutVal)
    // 	for(Trace::const_iterator it = start.trace.begin();
    // 	    it != start.trace.end(); it++)
    // 	    plot(DD_AMPMAPSTOP, int(it-start.trace.begin()), *it);

    //Fill histograms
    if(start.dataValid && stop.dataValid) {
        double timeDiff = stop.highResTime - start.highResTime;
        double timeRes  = 50; //20 ps/bin
        double timeOff  = 30000.;
        double phaseX   = 7000.;

//        cout << timeDiff * timeRes + timeOff << " "
//             << start.phase*timeRes-phaseX << endl;

        plot(D_TIMEDIFF, timeDiff*timeRes + timeOff);
        plot(DD_PVSP, start.phase*timeRes-phaseX,
            stop.phase*timeRes-phaseX);

        plot(DD_QDC, start.tqdc, 0);
        plot(DD_MAX, start.maxval, 0);
        plot(DD_MAXVSTDIFF, timeDiff*timeRes+timeOff, start.maxval);
        plot(DD_QDCVSMAX, start.maxval, start.tqdc);
        plot(DD_QDC, stop.tqdc, 1);
        plot(DD_MAX, stop.maxval, 1);
        plot(DD_SNRANDSDEV, start.snr+50, 0);
        plot(DD_SNRANDSDEV, start.stdDevBaseline*timeRes+timeOff, 1);
        plot(DD_SNRANDSDEV, stop.snr+50, 2);
        plot(DD_SNRANDSDEV, stop.stdDevBaseline*timeRes+timeOff, 3);
    }
} // void PulserProcessor::AnalyzeData
