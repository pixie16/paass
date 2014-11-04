/** \file BeamLogicProcessor.cpp
 *
 * Handling of beam logic events
 *
 */

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "BeamLogicProcessor.hpp"
#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "Messenger.hpp"
#include "RawEvent.hpp"

using namespace dammIds::logic;


namespace dammIds {
    namespace logic {
        const int D_COUNTER_BEAM = 0; //!< Beam cycle counter
        const int D_TIME_STOP_LENGTH = 1;//!< Time between stop events
    }
}

BeamLogicProcessor::BeamLogicProcessor(void) :
  EventProcessor(OFFSET, RANGE, "beamlogic")
{
    associatedTypes.insert("logic");
}

void BeamLogicProcessor::DeclarePlots(void)
{
    DeclareHistogram1D(D_COUNTER_BEAM, S2, "Beam counter toggle/analog/none");
    DeclareHistogram1D(D_TIME_STOP_LENGTH, SA, "Beam stop length (1 s / bin)");
}

bool BeamLogicProcessor::PreProcess(RawEvent &event)
{
    using namespace std;

    if (!EventProcessor::Process(event))
        return false;

    static const vector<ChanEvent*> &logicEvents =
        event.GetSummary("logic", true)->GetList();

    for (vector<ChanEvent*>::const_iterator it = logicEvents.begin();
        it != logicEvents.end(); it++)
    {
        double time = (*it)->GetTime();
        string place = (*it)->GetChanID().GetPlaceName();

        // for 2d plot of events 1s / bin
        // Bins in plot
        const unsigned BEAM_TOGGLE = 0;
        const unsigned BEAM_ANALOG = 1;
        const unsigned BEAM_NONE = 2;

        if (place == "logic_beam_0") {
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
    }

    return true;
}


bool BeamLogicProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
        return false;
    EndProcess(); // update processing time
    return true;
}
