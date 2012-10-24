/** \file ScintProcessor.cpp
 *
 * implementation for scintillator processor
 */
#include <iostream>
#include <cmath>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "RawEvent.hpp"
#include "Hen3Processor.hpp"
#include "ScintProcessor.hpp"
#include "Trace.hpp"

using namespace std;
using namespace dammIds::hen3;

namespace dammIds {
    namespace hen3 {
        const int D_MULT_HEN3 = 0;
        const int D_MULT_NEUTRON = 1;
        const int D_ENERGY_HEN3 = 2;
        const int D_ENERGY_NEUTRON = 3;
        const int D_TDIFF_HEN3_BETA = 4;
        const int D_TDIFF_NEUTRON_BETA = 5;

        const int DD_DISTR_HEN3 = 10;
        const int DD_DISTR_NEUTRON = 11;
    }
}

Hen3Processor::Hen3Processor() : EventProcessor(OFFSET, RANGE)
{
    name = "3hen";
    associatedTypes.insert("3hen"); // associate with the scint type
}

void Hen3Processor::DeclarePlots(void)
{
    DeclareHistogram1D(D_MULT_HEN3, S6, "3Hen event multiplicity");
    DeclareHistogram1D(D_MULT_NEUTRON, S6, "3Hen real neutron multiplicity");

    DeclareHistogram1D(D_ENERGY_HEN3, SE, "3Hen raw energy");
    DeclareHistogram1D(D_ENERGY_NEUTRON, SE, "Neutron raw energy");

    DeclareHistogram1D(D_TDIFF_HEN3_BETA, SE, "time diff hen3 - beta + 1000 (10 ns/ch)");
    DeclareHistogram1D(D_TDIFF_NEUTRON_BETA, SE, "time diff neutron - beta + 1000 (10 ns/ch)");

    DeclareHistogram2D(DD_DISTR_HEN3, 20, 20, "3Hen event distribution");
    DeclareHistogram2D(DD_DISTR_NEUTRON, 20, 20, "3Hen real neutron distribution");

}

bool Hen3Processor::PreProcess(RawEvent &event)
{
    if (!EventProcessor::PreProcess(event))
        return false;

    /** Notice that places Hen3 and Neutron (counters) are activated as parents
     * of detectors activated here. The thresholds are set in DetectorDriver.cpp
     * in constructors.
     */
    static const DetectorSummary *hen3Summary = event.GetSummary("3hen");
    for (vector<ChanEvent*>::const_iterator it = hen3Summary->GetList().begin(); 
        it != hen3Summary->GetList().end(); it++) {
            double time = (*it)->GetTime();
            double energy = (*it)->GetEnergy();
            int location = (*it)->GetChanID().GetLocation();
            string place = (*it)->GetChanID().GetPlaceName();
            CorrEventData data(time, true, energy);
            TreeCorrelator::get().places[place]->activate(data);
            stringstream neutron;
            neutron << "Neutron" << location;
            TreeCorrelator::get().places[neutron.str()]->activate(data);
    }

    return true;
}

bool Hen3Processor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
        return false;
  
    static const DetectorSummary *hen3Summary = event.GetSummary("3hen");
    
    int hen3_count = dynamic_cast<PlaceCounter*>(TreeCorrelator::get().places["Hen3"])->getCounter();
    int neutron_count = dynamic_cast<PlaceCounter*>(TreeCorrelator::get().places["Neutrons"])->getCounter();

    plot(D_MULT_HEN3, hen3_count);
    plot(D_MULT_NEUTRON, neutron_count);

    for (vector<ChanEvent*>::const_iterator it = hen3Summary->GetList().begin(); 
        it != hen3Summary->GetList().end(); it++) {
            ChanEvent *chan = *it;
            int location = chan->GetChanID().GetLocation();

            double energy = chan->GetEnergy();
            plot(D_ENERGY_HEN3, energy);

            stringstream neutron_name;
            neutron_name << "Neutron" << location;
            if (TreeCorrelator::get().places[neutron_name.str()]->status())
                plot(D_ENERGY_NEUTRON, energy);

            string place = chan->GetChanID().GetPlaceName();
            if (TreeCorrelator::get().places["Beta"]->status()) {
                double hen3_time = chan->GetTime();
                double beta_time = TreeCorrelator::get().places["Beta"]->last().time;
                const double timeResolution = 10e-9 / pixie::clockInSeconds;
                double dt = int(hen3_time - beta_time + 1000);
                if (TreeCorrelator::get().places[neutron_name.str()]->status())
                    plot(D_TDIFF_NEUTRON_BETA, dt / timeResolution);
                else
                    plot(D_TDIFF_HEN3_BETA, dt / timeResolution);

            }

            int Hen3Loc = chan->GetChanID().GetLocation();
            int xpos = 0;
            int ypos = 0;

            if( (0 <= Hen3Loc) && (Hen3Loc <= 7) ) {
                xpos = 6;
                ypos = Hen3Loc + 1;
            } else if( (8 <= Hen3Loc) && (Hen3Loc <= 14) ) {
                xpos = 7;
                ypos = Hen3Loc - 6;
            } else if( (15 <= Hen3Loc) && (Hen3Loc <= 20) ) {
                xpos = 8;
                ypos = Hen3Loc - 12;
            } else if((21 <= Hen3Loc) && (Hen3Loc <= 23)) {
                xpos = 9;
                ypos = Hen3Loc - 17;
            } else if( (24 <= Hen3Loc) && (Hen3Loc <= 31) ) {
                xpos = 4;
                ypos = Hen3Loc - 23;
            } else if( (32 <= Hen3Loc) && (Hen3Loc <= 38) ) {
                xpos = 3;
                ypos = Hen3Loc - 30;
            } else if( (39 <= Hen3Loc) && (Hen3Loc <= 44) ) {
                xpos = 2;
                ypos = Hen3Loc - 36;
            } else if( (45 <= Hen3Loc) && (Hen3Loc <= 47) ) {
                xpos = 1;
                ypos = Hen3Loc - 41;
            }

            plot(DD_DISTR_HEN3, xpos, ypos);
            if (TreeCorrelator::get().places[neutron_name.str()]->status())
                plot(DD_DISTR_NEUTRON, xpos, ypos);
    }

    EndProcess();
    return true;
}
