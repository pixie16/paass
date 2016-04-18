/** \file LogicProcessor.cpp
 * \brief handling of logic events, derived from MtcProcessor.cpp
 *
 * Start subtype corresponds to leading edge
 * Stop subtype corresponds to trailing edge
 */

#include <iostream>
#include <string>
#include <vector>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "LogicProcessor.hpp"

using namespace std;
using namespace dammIds::logic;

namespace dammIds {
    namespace logic {
	static double clockInSeconds = Globals::get()->clockInSeconds();
	static const unsigned int MAX_LOGIC = 10; //!<Maximum Number of Logic Signals
	const double logicPlotResolution = 
	    10e-6 / Globals::get()->clockInSeconds(); //!<Resolution for Logic Plots
	const double mtcPlotResolution = 10e-3 / clockInSeconds; //!<Res. for MTC Plots

        const int D_COUNTER_START = 0;//!< Counter for the starts
        const int D_COUNTER_STOP  = 5;//!< Counter for the stops
        const int D_TDIFF_STARTX  = 10;//!< Tdiff between starts
        const int D_TDIFF_STOPX   = 20;//!< Tdiff between stops
        const int D_TDIFF_SUMX    = 30;//!< Sum of tdiffs
        const int D_TDIFF_LENGTHX = 50;//!< Length between tdiff

	const int D_TDIFF_BEAM_START   = 0;//!< Tdiff between beam starts
        const int D_TDIFF_BEAM_STOP    = 1;//!< Tdiff between beam stops
        const int D_TDIFF_MOVE_START   = 2;//!< Tdiff between start move signals
        const int D_TDIFF_MOVE_STOP    = 3;//!< Tdiff between stop move signals
        const int D_MOVETIME           = 4;//!< Amount of time of the move
        const int D_BEAMTIME           = 5;//!< Amount of time the beam was on
        const int D_COUNTER            = 6;//!< A counter for cycles
        const int DD_TIME__DET_MTCEVENTS = 10;//!< Time vs. MTC Events
	
        const int MOVE_START_BIN = 1;//!< Start move bin
        const int MOVE_STOP_BIN = 3;//!< Stop move bin
        const int BEAM_START_BIN = 5;//!< Beam Start bin
        const int BEAM_STOP_BIN = 7;//!< Beam Stop bin

	const int DD_RUNTIME_LOGIC = 80;//!< Plot of run time logic

	namespace mtc{

	}
    }
} // logic namespace

LogicProcessor::LogicProcessor(void) : 
    EventProcessor(dammIds::logic::OFFSET, dammIds::logic::RANGE, "Logic"),
    lastStartTime(MAX_LOGIC, NAN), lastStopTime(MAX_LOGIC, NAN),
    logicStatus(MAX_LOGIC), stopCount(MAX_LOGIC), startCount(MAX_LOGIC) {
    associatedTypes.insert("logic");
    associatedTypes.insert("timeclass"); // old detector type
    associatedTypes.insert("mtc");
}

LogicProcessor::LogicProcessor(int offset, int range, bool doubleStop/*=false*/,
			       bool doubleStart/*=false*/) : 
    EventProcessor(offset, range, "Logic"),
    lastStartTime(MAX_LOGIC, NAN), lastStopTime(MAX_LOGIC, NAN),
    logicStatus(MAX_LOGIC), stopCount(MAX_LOGIC), startCount(MAX_LOGIC) {
    associatedTypes.insert("logic");
    associatedTypes.insert("timeclass"); // old detector type
    associatedTypes.insert("mtc");

    doubleStop_ = doubleStop;
    doubleStart_ = doubleStart;
}

void LogicProcessor::DeclarePlots(void) {
    const int counterBins = S4;
    const int timeBins = SC;
    
    DeclareHistogram1D(D_COUNTER_START, counterBins, "logic start counter");
    DeclareHistogram1D(D_COUNTER_STOP, counterBins, "logic stop counter");
    for (unsigned int i=0; i < MAX_LOGIC; i++) {
	DeclareHistogram1D(D_TDIFF_STARTX + i, timeBins, "tdiff btwn logic starts, 10 us/bin");
	DeclareHistogram1D(D_TDIFF_STOPX + i, timeBins, "tdiff btwn logic stops, 10 us/bin");
	DeclareHistogram1D(D_TDIFF_SUMX + i, timeBins, "tdiff btwn both logic, 10 us/bin");
	DeclareHistogram1D(D_TDIFF_LENGTHX + i, timeBins, "logic high time, 10 us/bin");
    }
    
    DeclareHistogram1D(D_TDIFF_BEAM_START, timeBins, "Time diff btwn beam starts, 10 ms/bin");
    DeclareHistogram1D(D_TDIFF_BEAM_STOP, timeBins, "Time diff btwn beam stops, 10 ms/bin");
    DeclareHistogram1D(D_TDIFF_MOVE_START, timeBins, "Time diff btwn move starts, 10 ms/bin");
    DeclareHistogram1D(D_TDIFF_MOVE_STOP, timeBins, "Time diff btwn move stops, 10 ms/bin");
    DeclareHistogram1D(D_MOVETIME, timeBins, "Move time, 10 ms/bin");
    DeclareHistogram1D(D_BEAMTIME, timeBins, "Beam on time, 10 ms/bin");
    DeclareHistogram1D(D_COUNTER, counterBins, "MTC and beam counter");
    
    DeclareHistogram2D(DD_TIME__DET_MTCEVENTS, SF, S2, "MTC and beam events");
    
    DeclareHistogram2D(DD_RUNTIME_LOGIC, plotSize, plotSize,
                       "runtime logic [1ms]");
    for(unsigned int i=1; i < MAX_LOGIC; i++)
        DeclareHistogram2D(DD_RUNTIME_LOGIC+i, plotSize,
                           plotSize, "runtime logic [1ms]");
}

bool LogicProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

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
	    
	    lastStartTime.at(loc) = time;
	    logicStatus.at(loc) = true;
	    
	    startCount.at(loc)++;
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
	    
            lastStopTime.at(loc) = time;
            logicStatus.at(loc) = false;
	    
            stopCount.at(loc)++;
            plot(D_COUNTER_STOP, loc);
        }
    }
    
    static const vector<ChanEvent*> &mtcEvents =
        event.GetSummary("mtc", true)->GetList();
    for (vector<ChanEvent*>::const_iterator it = mtcEvents.begin();
	 it != mtcEvents.end(); it++) {
        string subtype = (*it)->GetChanID().GetSubtype();
        double time   = (*it)->GetTime();
        // Time of the first event
        static double t0 = time;
        string place = (*it)->GetChanID().GetPlaceName();

        // for 2d plot of events 100ms / bin
        const double eventsResolution = 100e-3 / clockInSeconds;
        const unsigned MTC_START = 0;
        const unsigned MTC_STOP = 1;
        const unsigned BEAM_START = 2;
        const unsigned BEAM_STOP = 3;
        double time_x = int((time - t0) / eventsResolution);

        if(place == "mtc_start_0") {
            double dt_start = time -
                     TreeCorrelator::get()->place(place)->secondlast().time;
            TreeCorrelator::get()->place("TapeMove")->activate(time);
            TreeCorrelator::get()->place("Cycle")->deactivate(time);

            plot(D_TDIFF_MOVE_START, dt_start / mtcPlotResolution);
            plot(D_COUNTER, MOVE_START_BIN);
            plot(DD_TIME__DET_MTCEVENTS, time_x, MTC_START);
        } else if (place == "mtc_stop_0") {
            double dt_stop = time -
                     TreeCorrelator::get()->place(place)->secondlast().time;
            double dt_move = time -
                     TreeCorrelator::get()->place("mtc_start_0")->last().time;
            TreeCorrelator::get()->place("TapeMove")->deactivate(time);

            plot(D_TDIFF_MOVE_STOP, dt_stop / mtcPlotResolution);
            plot(D_MOVETIME, dt_move / mtcPlotResolution);
            plot(D_COUNTER, MOVE_STOP_BIN);
            plot(DD_TIME__DET_MTCEVENTS, time_x, MTC_STOP);
        } else if (place == "mtc_beam_start_0") {
	    double dt_start = time -
                      TreeCorrelator::get()->place(place)->secondlast().time;
            //Remove double starts
            if (doubleStart_) {
                double dt_stop = abs(time -
                  TreeCorrelator::get()->place("mtc_beam_stop_0")->last().time);
                if (abs(dt_start * clockInSeconds) < doubleTimeLimit_ ||
                    abs(dt_stop * clockInSeconds) < doubleTimeLimit_)
                    continue;
            }
            TreeCorrelator::get()->place("Beam")->activate(time);
            TreeCorrelator::get()->place("Cycle")->activate(time);

            plot(D_TDIFF_BEAM_START, dt_start / mtcPlotResolution);
            plot(D_COUNTER, BEAM_START_BIN);
            plot(DD_TIME__DET_MTCEVENTS, time_x, BEAM_START);
        } else if (place == "mtc_beam_stop_0") {
            double dt_stop = time -
                TreeCorrelator::get()->place(place)->secondlast().time;
            double dt_beam = time -
                 TreeCorrelator::get()->place("mtc_beam_start_0")->last().time;
            //Remove double stops
            if (doubleStop_) {
                if (abs(dt_stop * clockInSeconds) < doubleTimeLimit_ ||
                    abs(dt_beam * clockInSeconds) < doubleTimeLimit_)
                    continue;
            }
            TreeCorrelator::get()->place("Beam")->deactivate(time);

            plot(D_TDIFF_BEAM_STOP, dt_stop / mtcPlotResolution);
            plot(D_BEAMTIME, dt_beam / mtcPlotResolution);
            plot(D_COUNTER, BEAM_STOP_BIN);
            plot(DD_TIME__DET_MTCEVENTS, time_x, BEAM_STOP);
        }
    }
    return(true);
}

bool LogicProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);
    EndProcess();
    return(true);
}

/* // Came from TriggerLogicProcessor.cpp
bool LogicProcessor::NiftyGraph(RawEvent &event) {
    const double logicPlotResolution = 1e-3 / Globals::get()->clockInSeconds();
    const long maxBin = plotSize * plotSize;
    static DetectorSummary *stopsSummary    = event.GetSummary("logic:stop");
    static DetectorSummary *triggersSummary = event.GetSummary("logic:trigger");
    
    static const vector<ChanEvent*> &stops    = stopsSummary->GetList();
    static const vector<ChanEvent*> &triggers = triggersSummary->GetList();
    static int firstTimeBin = -1;

    for(vector<ChanEvent*>::const_iterator it = stops.begin();
    it != stops.end(); it++) {
        ChanEvent *chan = *it;

        unsigned int loc = chan->GetChanID().GetLocation();

        int timeBin = int(chan->GetTime() / logicPlotResolution);
        int startTimeBin = 0;

        if(!isnan(lastStartTime.at(loc))) {
            startTimeBin = int(lastStartTime.at(loc) / logicPlotResolution);
            if(firstTimeBin == -1) {
                firstTimeBin = startTimeBin;
            }
        } else if(firstTimeBin == -1) {
            firstTimeBin = startTimeBin;
        }
        startTimeBin = max(0, startTimeBin - firstTimeBin);
        timeBin -= firstTimeBin;

        for(int bin=startTimeBin; bin < timeBin; bin++) {
            int row = bin / plotSize;
            int col = bin % plotSize;
            plot(DD_RUNTIME_LOGIC, col, row, loc + 1);
            plot(DD_RUNTIME_LOGIC + loc, col, row, 1);
        }
    }
    for(vector<ChanEvent*>::const_iterator it = triggers.begin();
        it != triggers.end(); it++) {
        int timeBin = int((*it)->GetTime() / logicPlotResolution);
        timeBin -= firstTimeBin;
        if(timeBin >= maxBin || timeBin < 0)
            continue;

        int row = timeBin / plotSize;
        int col = timeBin % plotSize;

        plot(DD_RUNTIME_LOGIC, col, row, 20);
        for(int i=1; i < MAX_LOGIC; i++)
            plot(DD_RUNTIME_LOGIC + i, col, row, 5);
    }
    return(true);
}
*/
