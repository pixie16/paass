/** \file ScintProcessor.cpp
 *
 * implementation for scintillator processor
 */
#include <iostream>
#include <cmath>
#include <limits>

#include "DammPlotIds.hpp"
#include "RawEvent.hpp"
#include "ChanEvent.hpp"
#include "Hen3Processor.hpp"
#include "ScintProcessor.hpp"
#include "Trace.hpp"

using namespace std;
using namespace dammIds::hen3;

namespace dammIds {
    namespace hen3 {
        const int D_MULT_HEN3 = 0;//!< Multiplicity
        const int D_MULT_NEUTRON = 1;//!< Multiplicity of Neutrons
        const int D_ENERGY_HEN3 = 2;//!< Energy
        const int D_ENERGY_NEUTRON = 3;//!< Neutron Energy
        const int D_ENERGY_HEN3_TAPE = 4;//!< Energy
        const int D_TIME_NEUTRON = 7;//!< Neutron Time

        namespace beta {
            const int D_MULT_NEUTRON = 11;//!< Beta Gated Neutron Multiplicity
            const int D_ENERGY_NEUTRON = 13;//!< Beta Gated Neutron Energy
            const int D_TDIFF_HEN3_BETA = 15;//!< Beta Gated Tdiff btwn Hen3 & Beta
            const int D_TDIFF_NEUTRON_BETA = 16;//!< Beta Gated Tdiff btwn Neutron & Beta
        }

        const int DD_DISTR_HEN3 = 20;//!< Event Distribution
        const int DD_DISTR_NEUTRON = 21;//!< Neutron Distribution
    }
}

Hen3Processor::Hen3Processor() : EventProcessor(OFFSET, RANGE, "Hen3Processor") {
    associatedTypes.insert("3hen");
}

EventData Hen3Processor::BestBetaForNeutron(double nTime) {
    PlaceOR* betas = dynamic_cast<PlaceOR*>(
                        TreeCorrelator::get()->place("Beta"));
    unsigned sz = betas->info_.size();

    if (sz == 0)
        return EventData(-1);

    double bestTime = numeric_limits<double>::max();
    unsigned bestIndex = -1;
    for (unsigned index = 0; index < sz; ++index) {
        double dtime = (nTime - betas->info_.at(index).time);
        // We assume that neutron must be slower than beta
        if (dtime < 0)
            continue;
        if (dtime < bestTime) {
            bestTime = dtime;
            bestIndex = index;
        }
    }
    if (bestIndex > 0)
        return betas->info_.at(bestIndex);
    else
        return EventData(-1);
}

void Hen3Processor::DeclarePlots(void) {
    DeclareHistogram1D(D_MULT_HEN3, S4, "3Hen event multiplicity");
    DeclareHistogram1D(D_MULT_NEUTRON, S4, "3Hen real neutron multiplicity");
    DeclareHistogram1D(beta::D_MULT_NEUTRON, S4,
            "Beta gated 3Hen real neutron multiplicity");

    DeclareHistogram1D(D_ENERGY_HEN3, SE, "3Hen raw energy");
    DeclareHistogram1D(D_ENERGY_NEUTRON, SE, "Neutron raw energy");

    DeclareHistogram1D(beta::D_ENERGY_NEUTRON, SE,
                        "Beta gated neutron raw energy");

    DeclareHistogram1D(beta::D_TDIFF_HEN3_BETA, S8,
            "time diff hen3 - beta + 100 (1 us/ch)");
    DeclareHistogram1D(beta::D_TDIFF_NEUTRON_BETA, S8,
            "time diff neutron - beta + 100 (1 us/ch)");
    DeclareHistogram1D(D_ENERGY_HEN3_TAPE, SE,
            "3Hen raw energy, tape move period");

    DeclareHistogram1D(D_TIME_NEUTRON, SD,
            "Neutron events vs cycle time (1 ms / bin");

    DeclareHistogram2D(DD_DISTR_HEN3, S5, S5, "3Hen event distribution");
    DeclareHistogram2D(DD_DISTR_NEUTRON, S5, S5,
            "3Hen neutron distribution");
}

bool Hen3Processor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    /** Notice that places Hen3 and Neutron (counters) are activated as parents
     * of detectors activated here. The threshold for the latter is set
     * in the xml file.
     */
    static const DetectorSummary *hen3Summary = event.GetSummary("3hen", true);
    for (vector<ChanEvent*>::const_iterator it =
            hen3Summary->GetList().begin();
        it != hen3Summary->GetList().end(); it++) {
            double time = (*it)->GetTime();
            double energy = (*it)->GetEnergy();
            int location = (*it)->GetChanID().GetLocation();

            EventData data(time, energy, location, true);
            stringstream neutron;
            neutron << "Neutron_" << location;
            TreeCorrelator::get()->place(neutron.str())->activate(data);
    }

    return true;
}

bool Hen3Processor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    static const DetectorSummary *hen3Summary = event.GetSummary("3hen", true);

    int hen3_count = dynamic_cast<PlaceCounter*>(
            TreeCorrelator::get()->place("Hen3"))->getCounter();
    int neutron_count = dynamic_cast<PlaceCounter*>(
            TreeCorrelator::get()->place("Neutrons"))->getCounter();

    double clockInSeconds = Globals::get()->clockInSeconds();
    /** Place Cycle is activated by BeamOn event and deactivated by TapeMove*/
    bool tapeMove = !(TreeCorrelator::get()->place("Cycle")->status());
    /** Cycle time is measured from the beginning of the last BeamON event */
    double cycleTime = TreeCorrelator::get()->place("Cycle")->last().time;

    if (tapeMove) {
        for (vector<ChanEvent*>::const_iterator it =
                 hen3Summary->GetList().begin();
             it != hen3Summary->GetList().end(); it++) {
                double energy = (*it)->GetEnergy();
                plot(D_ENERGY_HEN3_TAPE, energy);
        }
        return true;
    }

    plot(D_MULT_HEN3, hen3_count);
    plot(D_MULT_NEUTRON, neutron_count);

    int beta_gated_neutron_multi = 0;
    for (vector<ChanEvent*>::const_iterator it = hen3Summary->GetList().begin();
        it != hen3Summary->GetList().end(); it++) {
            ChanEvent *chan = *it;
            int location = chan->GetChanID().GetLocation();
            double energy = chan->GetEnergy();
            double time = chan->GetTime();

            plot(D_ENERGY_HEN3, energy);

            stringstream neutron_name;
            neutron_name << "Neutron_" << location;
            if (TreeCorrelator::get()->place(neutron_name.str())->status()) {
                plot(D_ENERGY_NEUTRON, energy);
                double decayTime = (time - cycleTime) * clockInSeconds;
                int decayTimeBin = int(decayTime / cycleTimePlotResolution_);
                plot(D_TIME_NEUTRON, decayTimeBin);
            }

            string place = chan->GetChanID().GetPlaceName();
            if (TreeCorrelator::get()->place("Beta")->status()) {
                EventData bestBeta = BestBetaForNeutron(time);
                double nb_dtime = (time - bestBeta.time) * clockInSeconds;
                             TreeCorrelator::get()->place("Beta")->last().time;
                double dt = 100 + nb_dtime / diffTimePlotResolution_;
                if (dt > S8)
                    dt = S8 - 1;
                if (dt < 0) {
                    dt = 0;
                }
                if (TreeCorrelator::get()->
                                    place(neutron_name.str())->status()) {
                    plot(beta::D_TDIFF_NEUTRON_BETA, dt);
                    plot(beta::D_ENERGY_NEUTRON, energy);
                    ++beta_gated_neutron_multi;
                }
                plot(beta::D_TDIFF_HEN3_BETA, dt);
            }

            /** These plots show He3 bar location hit
             * picture is as looking along the beam line
             * (having ORIC behind, 3Hen in front)
             */
            int xpos = 0;
            int ypos = 0;
            if( 0 <= location && location <= 7 ) {
                ypos = 11;
                xpos = 15 - 2 * location;
            } else if( 8 <= location && location <= 14 ) {
                ypos = 13;
                xpos = 30 - 2 * location;
            } else if( 15 <= location && location <= 20 ) {
                ypos = 15;
                xpos = 43 - 2 * location;
            } else if( 21 <= location && location <= 23 ) {
                ypos = 17;
                xpos = 52 - 2 * location;
            } else if( 24 <= location && location <= 31 ) {
                ypos = 7;
                xpos = 63 - 2 * location;
            } else if( 32 <= location && location <= 38 ) {
                ypos = 5;
                xpos = 78 - 2 * location;
            } else if( 39 <= location && location <= 44 ) {
                ypos = 3;
                xpos = 91 - 2 * location;
            } else if( 45 <= location && location <= 47 ) {
                ypos = 1;
                xpos = 100 - 2 * location;
            }

            plot(DD_DISTR_HEN3, xpos, ypos);
            if (TreeCorrelator::get()->place(neutron_name.str())->status())
                plot(DD_DISTR_NEUTRON, xpos, ypos);
    }
    plot(beta::D_MULT_NEUTRON, beta_gated_neutron_multi);

    EndProcess();
    return true;
}
