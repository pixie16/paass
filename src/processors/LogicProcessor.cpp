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

/** Upper limit in seconds for bad (double) start/stop event */
static const double doubleTimeLimit_ = 10e-6;

namespace dammIds {
    namespace logic {
	static double clockInSeconds = Globals::get()->clockInSeconds(); //!< clock in seconds
	static const unsigned int MAX_LOGIC = 10; //!<Maximum Number of Logic Signals
	const double logicPlotResolution =
	    10e-6 / Globals::get()->clockInSeconds(); //!<Resolution for Logic Plots
	const double mtcPlotResolution = 10e-3 / clockInSeconds; //!<Res. for MTC Plots

	///Original Logic Processor
        const int D_COUNTER_START  = 0;//!< Counter for the starts
        const int D_COUNTER_STOP   = 1;//!< Counter for the stops
        const int DD_TDIFF_START   = 2;//!< Tdiff between starts
        const int DD_TDIFF_STOP    = 3;//!< Tdiff between stops
        const int DD_TDIFF_SUM     = 4;//!< Sum of tdiffs
        const int DD_TDIFF_LENGTH  = 5;//!< Length between tdiff

	///MTC Processor
	const int D_TDIFF_BEAM_START = 6;//!< Tdiff between beam starts
        const int D_TDIFF_BEAM_STOP  = 7;//!< Tdiff between beam stops
        const int D_TDIFF_MOVE_START = 8;//!< Tdiff between start move signals
        const int D_TDIFF_MOVE_STOP  = 9;//!< Tdiff between stop move signals
        const int D_MOVETIME         = 10;//!< Amount of time of the move
        const int D_BEAMTIME         = 11;//!< Amount of time the beam was on
        const int D_COUNTER          = 12;//!< A counter for cycles
	const int D_TDIFF_SUPERCYCLE = 13;//!< Tdiff between supercycles
        const int D_TDIFF_T1         = 14;//!< Tdiff between T1 signals
        const int DD_TIME_DET_MTCEVENTS = 15;//!< Time vs. MTC Events

        const int MOVE_START_BIN = 16;//!< Start move bin
        const int MOVE_STOP_BIN = 17;//!< Stop move bin
        const int BEAM_START_BIN = 18;//!< Beam Start bin
        const int BEAM_STOP_BIN = 19;//!< Beam Stop bin

	///From Beam Logic Processor
	const int D_COUNTER_BEAM = 21; //!< Beam cycle counter
        const int D_TIME_STOP_LENGTH = 22;//!< Time between stop events

	///Trigger Logic Processor
	const int DD_RUNTIME_LOGIC = 23;//!< Plot of run time logic
    }
} // logic namespace

LogicProcessor::LogicProcessor(void) :
    EventProcessor(dammIds::logic::OFFSET, dammIds::logic::RANGE, "LogicProcessor"),
    lastStartTime(MAX_LOGIC, NAN), lastStopTime(MAX_LOGIC, NAN),
    logicStatus(MAX_LOGIC), stopCount(MAX_LOGIC), startCount(MAX_LOGIC) {
    associatedTypes.insert("logic");
    associatedTypes.insert("timeclass"); // old detector type
    associatedTypes.insert("mtc");
}

LogicProcessor::LogicProcessor(int offset, int range, bool doubleStop/*=false*/,
			       bool doubleStart/*=false*/) :
    EventProcessor(offset, range, "LogicProcessor"),
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

    ///From Original Logic Processor
    DeclareHistogram1D(D_COUNTER_START, counterBins, "logic start counter");
    DeclareHistogram1D(D_COUNTER_STOP, counterBins, "logic stop counter");
    DeclareHistogram2D(DD_TDIFF_START, S4,timeBins, "TDiff btwn starts, 10 us/bin");
    DeclareHistogram2D(DD_TDIFF_STOP, S4,timeBins, "TDiff btwn starts, 10 us/bin");
    DeclareHistogram2D(DD_TDIFF_SUM, S4,timeBins, "TDiff btwn starts, 10 us/bin");
    DeclareHistogram2D(DD_TDIFF_LENGTH, S4,timeBins, "TDiff btwn starts, 10 us/bin");

    ///From MTC Processor
    DeclareHistogram1D(D_TDIFF_BEAM_START, timeBins, "Time diff btwn beam starts, 10 ms/bin");
    DeclareHistogram1D(D_TDIFF_BEAM_STOP, timeBins, "Time diff btwn beam stops, 10 ms/bin");
    DeclareHistogram1D(D_TDIFF_MOVE_START, timeBins, "Time diff btwn move starts, 10 ms/bin");
    DeclareHistogram1D(D_TDIFF_MOVE_STOP, timeBins, "Time diff btwn move stops, 10 ms/bin");
    DeclareHistogram1D(D_MOVETIME, timeBins, "Move time, 10 ms/bin");
    DeclareHistogram1D(D_BEAMTIME, timeBins, "Beam on time, 10 ms/bin");
    DeclareHistogram1D(D_COUNTER, counterBins, "MTC and beam counter");
    DeclareHistogram2D(DD_TIME_DET_MTCEVENTS, SF, S2, "MTC and beam events");

    ///From BeamLogicProcessor
    DeclareHistogram1D(D_COUNTER_BEAM, S2, "Beam counter toggle/analog/none");
    DeclareHistogram1D(D_TIME_STOP_LENGTH, SA, "Beam stop length (1 s / bin)");

    ///From TriggerLogicProcessor
    // DeclareHistogram2D(DD_RUNTIME_LOGIC, plotSize, plotSize,
    //                    "runtime logic [1ms]");
    // for(unsigned int i=1; i < MAX_LOGIC; i++)
    //     DeclareHistogram2D(DD_RUNTIME_LOGIC+i, plotSize,
    //                        plotSize, "runtime logic [1ms]");
}

bool LogicProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    static const vector<ChanEvent*> &events = sumMap["logic"]->GetList();

    for (vector<ChanEvent*>::const_iterator it = events.begin();
	 it != events.end(); it++) {
	ChanEvent *chan = *it;

	string place = (*it)->GetChanID().GetPlaceName();
	string subtype   = chan->GetChanID().GetSubtype();
	unsigned int loc = chan->GetChanID().GetLocation();
	double time = chan->GetTime();

	static double t0 = time;

	// for 2d plot of events 100ms / bin
	const double eventsResolution = 100e-3 / clockInSeconds;
	const unsigned MTC_START = 0;
	const unsigned MTC_STOP = 1;
	const unsigned BEAM_START = 2;
	const unsigned BEAM_STOP = 3;
        // for 2d plot of events 1s / bin
        // Bins in plot
        const unsigned BEAM_TOGGLE = 0;
        const unsigned BEAM_ANALOG = 1;
        const unsigned BEAM_NONE = 2;
	double time_x = int((time - t0) / eventsResolution);

	if(subtype == "start") {
	    if (!isnan(lastStartTime.at(loc))) {
		double timediff = time - lastStartTime.at(loc);
		plot(DD_TDIFF_START, timediff / logicPlotResolution, loc);
		plot(DD_TDIFF_SUM, timediff / logicPlotResolution, loc);
	    }

	    lastStartTime.at(loc) = time;
	    logicStatus.at(loc) = true;

	    startCount.at(loc)++;
	    plot(D_COUNTER_START, loc);
	} else if (subtype == "stop") {
	    if (!isnan(lastStopTime.at(loc))) {
                double timediff = time - lastStopTime.at(loc);
                plot(DD_TDIFF_STOP, timediff / logicPlotResolution, loc);
                plot(DD_TDIFF_SUM, timediff / logicPlotResolution, loc);
                if (!isnan(lastStartTime.at(loc))) {
                    double moveTime = time - lastStartTime.at(loc);
                    plot(DD_TDIFF_LENGTH, moveTime / logicPlotResolution, loc);
                }
            }

            lastStopTime.at(loc) = time;
            logicStatus.at(loc) = false;

            stopCount.at(loc)++;
            plot(D_COUNTER_STOP, loc);
        } else if(place == "logic_mtc_start_0") {
	    double dt_start = time -
		TreeCorrelator::get()->place(place)->secondlast().time;
	    TreeCorrelator::get()->place("TapeMove")->activate(time);
	    TreeCorrelator::get()->place("Cycle")->deactivate(time);

	    plot(D_TDIFF_MOVE_START, dt_start / mtcPlotResolution);
	    plot(D_COUNTER, MOVE_START_BIN);
	    plot(DD_TIME_DET_MTCEVENTS, time_x, MTC_START);
	} else if (place == "logic_mtc_stop_0") {
	    double dt_stop = time -
		TreeCorrelator::get()->place(place)->secondlast().time;
	    double dt_move = time -
		TreeCorrelator::get()->place("logic_mtc_start_0")->last().time;
	    TreeCorrelator::get()->place("TapeMove")->deactivate(time);

	    plot(D_TDIFF_MOVE_STOP, dt_stop / mtcPlotResolution);
	    plot(D_MOVETIME, dt_move / mtcPlotResolution);
	    plot(D_COUNTER, MOVE_STOP_BIN);
	    plot(DD_TIME_DET_MTCEVENTS, time_x, MTC_STOP);
	} else if (place == "logic_beam_start_0") {
	    double dt_start = time -
		TreeCorrelator::get()->place(place)->secondlast().time;
	    //Remove double starts
	    if (doubleStart_) {
		double dt_stop =
		    abs(time -
			TreeCorrelator::get()->place("logic_beam_stop_0")->last().time);
		if (abs(dt_start * clockInSeconds) < doubleTimeLimit_ ||
		    abs(dt_stop * clockInSeconds) < doubleTimeLimit_)
		    continue;
	    }
	    TreeCorrelator::get()->place("Beam")->activate(time);
	    TreeCorrelator::get()->place("Cycle")->activate(time);

	    plot(D_TDIFF_BEAM_START, dt_start / mtcPlotResolution);
	    plot(D_COUNTER, BEAM_START_BIN);
	    plot(DD_TIME_DET_MTCEVENTS, time_x, BEAM_START);
	} else if (place == "logic_beam_stop_0") {
	    double dt_stop = time -
		TreeCorrelator::get()->place(place)->secondlast().time;
	    double dt_beam = time -
		TreeCorrelator::get()->place("logic_beam_start_0")->last().time;
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
	    plot(DD_TIME_DET_MTCEVENTS, time_x, BEAM_STOP);
	} else if (place == "logic_t1_0") {
	    //TreeCorrelator::get()->place("Protons")->activate(time);
	    double dt_t1 = time -
		TreeCorrelator::get()->place("logic_t1_0")->last().time;
	    plot(D_TDIFF_T1, dt_t1 / mtcPlotResolution);
	} else if (place == "logic_supercycle_0") {
	    TreeCorrelator::get()->place("Supercycle")->activate(time);
	    double dt_supercycle = time -
		TreeCorrelator::get()->place("logic_supercycle_0")->last().time;
	    plot(D_TDIFF_SUPERCYCLE, dt_supercycle / mtcPlotResolution);
	}else if (place == "logic_beam_0") {
            double last_time =
                TreeCorrelator::get()->place(place)->secondlast().time;
            double dt_beam_stop = abs(time - last_time);

            Messenger m;
            stringstream ss;
            // Check for double recorded same event (1 us limit)
            if (dt_beam_stop < doubleTimeLimit_) {
                ss << "Ignore fast beam stop" << endl;
                m.warning(ss.str());
                continue;
            }

            // If beam was stopped, activate place and plot stop length
            if (!TreeCorrelator::get()->place("Beam")->status()) {
                double clockInSeconds = Globals::get()->clockInSeconds();
                double resolution = 1.0 / clockInSeconds;

                plot(D_TIME_STOP_LENGTH, dt_beam_stop / resolution);

                TreeCorrelator::get()->place("Beam")->activate(time);
                ss << "Beam started after: " << dt_beam_stop / resolution
                   << " s ";
                m.run_message(ss.str());
            }
            else {
                TreeCorrelator::get()->place("Beam")->deactivate(time);
                ss << "Beam stopped";
                m.run_message(ss.str());
            }
            plot(D_COUNTER_BEAM, BEAM_TOGGLE);

        }
        else if (place == "logic_analog_0") {
            plot(D_COUNTER_BEAM, BEAM_ANALOG);
        }
        else if (place == "logic_none_0") {
            plot(D_COUNTER_BEAM, BEAM_NONE);
        }
    }//events loop
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
