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
        const int D_HEN3_MULT         = 0;
        const int D_NEUTRON_MULT      = 1;
        const int D_BETA_NEUTR_TIME   = 2;
        const int DD_HEN3_DISTR       = 3;
        const int DD_NEUTRON_DISTR    = 4;
    }
}

Hen3Processor::Hen3Processor() : EventProcessor(OFFSET, RANGE)
{
    name = "3hen";
    associatedTypes.insert("3hen"); // associate with the scint type
}

void Hen3Processor::DeclarePlots(void)
{
    DeclareHistogram1D(D_HEN3_MULT, S6, "3Hen event multiplicity");
    DeclareHistogram1D(D_NEUTRON_MULT, S6, "3Hen real neutron multiplicity");
    DeclareHistogram2D(DD_HEN3_DISTR, 20, 20, "3Hen event distribution");
    DeclareHistogram2D(DD_NEUTRON_DISTR, 20, 20, "3Hen real neutron distribution");//s.w.p. for 3Hen event distribution
    DeclareHistogram1D(D_BETA_NEUTR_TIME, SE, "time diff beta neutron");//time difference between beta and neutron
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
  
    int gammaMult = event.GetSummary("ge:clover_high")->GetMult();
    static const DetectorSummary *hen3Summary = event.GetSummary("3hen");
    
    bool hen3 = TreeCorrelator::get().places["Hen3"]->status();
    bool neutron = TreeCorrelator::get().places["Neutrons"]->status();
    int hen3_count = dynamic_cast<PlaceCounter*>(TreeCorrelator::get().places["Hen3"])->getCounter();
    int neutron_count = dynamic_cast<PlaceCounter*>(TreeCorrelator::get().places["Neutrons"])->getCounter();

    plot(D_HEN3_MULT, hen3_count);
    if (neutron_count > 0)
        plot(D_NEUTRON_MULT, neutron_count);

    for (vector<ChanEvent*>::const_iterator it = hen3Summary->GetList().begin(); 
        it != hen3Summary->GetList().end(); it++) {
            ChanEvent *chan = *it;

            string place = chan->GetChanID().GetPlaceName();
            if (TreeCorrelator::get().places["Beta"]->status()) {
                double hen3_time = chan->GetTime();
                double beta_time = TreeCorrelator::get().places["Beta"]->last().time;

                double dt = int(hen3_time - beta_time);
                plot(D_BETA_NEUTR_TIME, dt);
            }

            int Hen3Loc = chan->GetChanID().GetLocation();
            double Hen3Energy = chan->GetEnergy();
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

            plot(DD_HEN3_DISTR, xpos, ypos);

            if (neutron_count > 0){
                plot(DD_NEUTRON_DISTR, xpos, ypos);
            }
        

    }

    EndProcess();
    return true;
}
