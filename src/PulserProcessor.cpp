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
        const int D_TIMEDIFF     = 0;//!< Time Difference
        const int D_PROBLEMSTUFF = 1;//!< Histogram for problems

        const int DD_QDC         = 2;//!< QDCs
        const int DD_MAX         = 3;//!< Max Values
        const int DD_PVSP        = 4;//!< Phase vs. Phase
        const int DD_MAXVSTDIFF  = 5;//!< Maximum Start vs. TDiff
        const int DD_QDCVSMAX    = 6;//!< QDC Start vs. Max Start
        const int DD_AMPMAPSTART = 7;//!< Amplitude Map Start
        const int DD_AMPMAPSTOP  = 8;//!< Amplitude Map Stop
        const int DD_SNRANDSDEV  = 9;//!< SNR and Standard Deviation
        const int DD_PROBLEMS    = 13;//!< 2D problems
        const int DD_MAXSVSTDIFF = 14;//!< Maximum Stop vs. Tdiff
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
    pulserMap.clear();

    static const vector<ChanEvent*> & pulserEvents =
        event.GetSummary("pulser")->GetList();

    for(vector<ChanEvent*>::const_iterator itPulser = pulserEvents.begin();
	itPulser != pulserEvents.end(); itPulser++) {
        unsigned int location = (*itPulser)->GetChanID().GetLocation();
        string subType = (*itPulser)->GetChanID().GetSubtype();

        TimingDefs::TimingIdentifier key(location, subType);
        pulserMap.insert(make_pair(key, HighResTimingData(*itPulser)));
    }

    if(pulserMap.empty() || pulserMap.size()%2 != 0) {
        plot(D_PROBLEMSTUFF, 27);
        return(false);
    } else
        return(true);
}

void PulserProcessor::AnalyzeData(void) {
    HighResTimingData start =
        (*pulserMap.find(make_pair(0,"start"))).second;
    HighResTimingData stop  =
        (*pulserMap.find(make_pair(0,"stop"))).second;

    static int counter = 0;
    for(Trace::const_iterator it = start.GetTrace()->begin();
        it!= start.GetTrace()->end(); it++)
        plot(DD_PROBLEMS, int(it-start.GetTrace()->begin()), counter, *it);
    counter ++;

    // unsigned int cutVal = 15;
    // if(start.maxpos == 41)
    // if(start.GetMaximumValue() < 2384-cutVal)
    // 	for(Trace::const_iterator it = start.GetTrace()->begin();
    // 	    it != start.GetTrace()->end(); it++)
    // 	    plot(DD_AMPMAPSTART, int(it-start.GetTrace()->begin()), *it);

    // if(stop.GetMaximumValue() < 2555-cutVal)
    // 	for(Trace::const_iterator it = start.GetTrace()->begin();
    // 	    it != start.GetTrace()->end(); it++)
    // 	    plot(DD_AMPMAPSTOP, int(it-start.GetTrace()->begin()), *it);

    if(start.GetIsValidData() && stop.GetIsValidData()) {
        double timeDiff = stop.GetHighResTime() - start.GetHighResTime();
        double timeRes  = 50; //20 ps/bin
        double timeOff  = 31000.;
        double phaseX   = 7000.;

//        cout << timeDiff * timeRes + timeOff << " "
//             << start.GetPhase()*timeRes-phaseX << endl;

        plot(D_TIMEDIFF, timeDiff*timeRes + timeOff);
        plot(DD_PVSP, start.GetPhase()*timeRes-phaseX,
            stop.GetPhase()*timeRes-phaseX);

        plot(DD_QDC, start.GetTraceQdc(), 0);
        plot(DD_MAX, start.GetMaximumValue(), 0);
        plot(DD_MAXVSTDIFF, timeDiff*timeRes+timeOff, start.GetMaximumValue());
        plot(DD_QDCVSMAX, start.GetMaximumValue(), start.GetTraceQdc());
        plot(DD_QDC, stop.GetTraceQdc(), 1);
        plot(DD_MAX, stop.GetMaximumValue(), 1);
        plot(DD_SNRANDSDEV, start.GetSignalToNoiseRatio()+50, 0);
        plot(DD_SNRANDSDEV, start.GetStdDevBaseline()*timeRes+timeOff, 1);
        plot(DD_SNRANDSDEV, stop.GetSignalToNoiseRatio()+50, 2);
        plot(DD_SNRANDSDEV, stop.GetStdDevBaseline()*timeRes+timeOff, 3);
    }
} // void PulserProcessor::AnalyzeData
