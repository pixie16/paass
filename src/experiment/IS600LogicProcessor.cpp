/** \file IS600LogicProcessor.cpp
 * \brief Handling of the Logic for the ISOLDE Experiment 599/600
 *
 * Start subtype corresponds to leading edge of tape move signal
 * Stop subtype corresponds to trailing edge of tape move signal
 */

#include <iostream>
#include <sstream>
#include <cmath>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "IS600LogicProcessor.hpp"

using namespace std;
using namespace dammIds::mtc;

namespace dammIds {
    namespace mtc {
        const int D_TDIFF_BEAM_START   = 0;//!< Tdiff between beam starts
        const int D_TDIFF_BEAM_STOP    = 1;//!< Tdiff between beam stops
        const int D_TDIFF_MOVE_START   = 2;//!< Tdiff between start move signals
        const int D_TDIFF_MOVE_STOP    = 3;//!< Tdiff between stop move signals
        const int D_TDIFF_SUPERCYCLE   = 7;//!< Tdiff between supercycles
        const int D_TDIFF_T1           = 8;//!< Tdiff between T1 signals
        const int D_MOVETIME           = 4;//!< Amount of time of the move
        const int D_BEAMTIME           = 5;//!< Amount of time the beam was on
        const int D_COUNTER            = 6;//!< A counter for cycles
        const int DD_TIME__DET_MTCEVENTS = 10;//!< Time vs. MTC Events

        const int MOVE_START_BIN = 1;//!< Start move bin
        const int MOVE_STOP_BIN = 3;//!< Stop move bin
        const int BEAM_START_BIN = 5;//!< Beam Start bin
        const int BEAM_STOP_BIN = 7;//!< Beam Stop bin
    }
}

IS600LogicProcessor::IS600LogicProcessor(bool double_stop, bool double_start)
  : MtcProcessor(double_stop, double_start) {
    associatedTypes.insert("mtc");
    double_stop_ = double_stop;
    double_start_ = double_start;
}

void IS600LogicProcessor::DeclarePlots(void) {
    using namespace dammIds::mtc;
    MtcProcessor::DeclarePlots();

    DeclareHistogram1D(D_TDIFF_SUPERCYCLE, SA, "Tdiff btwn Supercycles");
    DeclareHistogram1D(D_TDIFF_T1, SA, "Tdiff btwn T1");
}

bool IS600LogicProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    double clockInSeconds = Globals::get()->clockInSeconds();
    // plot with 10 ms bins
    const double mtcPlotResolution = 10e-3 / clockInSeconds;

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
            if (double_start_) {
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
            if (double_stop_) {
                if (abs(dt_stop * clockInSeconds) < doubleTimeLimit_ ||
                    abs(dt_beam * clockInSeconds) < doubleTimeLimit_)
                    continue;
            }
            TreeCorrelator::get()->place("Beam")->deactivate(time);

            plot(D_TDIFF_BEAM_STOP, dt_stop / mtcPlotResolution);
            plot(D_BEAMTIME, dt_beam / mtcPlotResolution);
            plot(D_COUNTER, BEAM_STOP_BIN);
            plot(DD_TIME__DET_MTCEVENTS, time_x, BEAM_STOP);
        } else if (place == "mtc_t1_0") {
	  //TreeCorrelator::get()->place("Protons")->activate(time);
	    double dt_t1 = time -
                 TreeCorrelator::get()->place("mtc_t1_0")->last().time;
	    plot(D_TDIFF_T1, dt_t1 / mtcPlotResolution);
	} else if (place == "mtc_supercycle_0") {
	    TreeCorrelator::get()->place("Supercycle")->activate(time);
	    double dt_supercycle = time -
                 TreeCorrelator::get()->place("mtc_supercycle_0")->last().time;
	    plot(D_TDIFF_SUPERCYCLE, dt_supercycle / mtcPlotResolution);
	}
    }
    return true;
}

bool IS600LogicProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
        return false;
    EndProcess(); // update processing time
    return true;
}
