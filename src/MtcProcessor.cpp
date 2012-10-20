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
#include <sstream>
#include <cmath>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "MtcProcessor.hpp"

using namespace std;
using namespace dammIds::mtc;

namespace dammIds {
    namespace mtc {
        const int D_TDIFF_BEAM_START   = 0;
        const int D_TDIFF_BEAM_STOP    = 1;
        const int D_TDIFF_MOVE_START   = 2;
        const int D_TDIFF_MOVE_STOP    = 3;
        const int D_MOVETIME           = 4;
        const int D_BEAMTIME           = 5;
        const int D_COUNTER            = 10;

        const int MOVE_START_BIN = 1;
        const int MOVE_STOP_BIN = 3;
        const int BEAM_START_BIN = 5;
        const int BEAM_STOP_BIN = 7;
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
    
    const int counterBins = S3;
    const int timeBins = SA;

    DeclareHistogram1D(D_TDIFF_BEAM_START, timeBins, "Time diff btwn beam starts, 10 ms/bin");
    DeclareHistogram1D(D_TDIFF_BEAM_STOP, timeBins, "Time diff btwn beam stops, 10 ms/bin");
    DeclareHistogram1D(D_TDIFF_MOVE_START, timeBins, "Time diff btwn move starts, 10 ms/bin");
    DeclareHistogram1D(D_TDIFF_MOVE_STOP, timeBins, "Time diff btwn move stops, 10 ms/bin");
    DeclareHistogram1D(D_MOVETIME, timeBins, "Move time, 10 ms/bin");
    DeclareHistogram1D(D_BEAMTIME, timeBins, "Beam on time, 10 ms/bin");
    // Counter of events; see dammIds::mtc namespace for bin definition
    DeclareHistogram1D(D_COUNTER, counterBins, "MTC and beam counter");
}

bool MtcProcessor::PreProcess(RawEvent &event)
{
    if (!EventProcessor::PreProcess(event))
        return false;

    const static DetectorSummary *mtcSummary = NULL;

    // plot with 10 ms bins
    const double mtcPlotResolution = 10e-3 / pixie::clockInSeconds;

    if (mtcSummary == NULL) {
        if ( sumMap.count("mtc") )
            mtcSummary = sumMap["mtc"];
        else if ( sumMap.count("timeclass") ) 
            mtcSummary = sumMap["timeclass"];
    }

    static const vector<ChanEvent*> &mtcEvents = mtcSummary->GetList();

    for (vector<ChanEvent*>::const_iterator it = mtcEvents.begin();
	 it != mtcEvents.end(); it++) {
        string subtype = (*it)->GetChanID().GetSubtype();
        double time   = (*it)->GetTime();	
        string place = (*it)->GetChanID().GetPlaceName();

        if (TreeCorrelator::get().places.count(place) == 1) {
            double time   = (*it)->GetTime();
            TreeCorrelator::get().places[place]->activate(time);
        } else {
            cerr << "In MtcProcessor: place " << place
                    << " does not exist." << endl;
            return false;
        }

        if(subtype == "start") {

            double dt_start = time - 
                     TreeCorrelator::get().places["TapeMove"]->secondlast().time;
            TreeCorrelator::get().places["TapeMove"]->activate(time);
            TreeCorrelator::get().places["Cycle"]->deactivate(time);

            plot(D_TDIFF_MOVE_START, dt_start / mtcPlotResolution);
            plot(D_COUNTER, MOVE_START_BIN);

        } else if (subtype == "stop") {

            double dt_stop = time - 
                     TreeCorrelator::get().places["TapeMove"]->secondlast().time;
            double dt_move = time - 
                     TreeCorrelator::get().places["TapeMove"]->last().time;
            TreeCorrelator::get().places["TapeMove"]->deactivate(time);

            plot(D_TDIFF_MOVE_STOP, dt_stop / mtcPlotResolution);
            plot(D_MOVETIME, dt_move / mtcPlotResolution);
            plot(D_COUNTER, MOVE_STOP_BIN);

        } else if (subtype == "beam_start") {

            double dt_start = time -
                      TreeCorrelator::get().places["Beam"]->secondlast().time;
            TreeCorrelator::get().places["Beam"]->activate(time);
            TreeCorrelator::get().places["Cycle"]->activate(time);

            plot(D_TDIFF_BEAM_START, dt_start / mtcPlotResolution);
            plot(D_COUNTER, BEAM_START_BIN);

        } else if (subtype == "beam_stop") {

            double dt_stop = time - 
                      TreeCorrelator::get().places["Beam"]->secondlast().time;
            double dt_beam = time - 
                      TreeCorrelator::get().places["Beam"]->last().time;
            TreeCorrelator::get().places["Beam"]->deactivate(time);

            plot(D_TDIFF_BEAM_STOP, dt_stop / mtcPlotResolution);
            plot(D_BEAMTIME, dt_beam / mtcPlotResolution);
            plot(D_COUNTER, BEAM_STOP_BIN);

        }
    }
    return true;
}

bool MtcProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
        return false;
    using namespace dammIds::mtc;
    EndProcess(); // update processing time
    return true;
}
