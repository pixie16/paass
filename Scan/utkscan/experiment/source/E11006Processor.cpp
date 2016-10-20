#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <cmath>

#include "RawEvent.hpp"
#include "E11006Processor.hpp"
#include "DammPlotIds.hpp"

using namespace std;
using namespace dammIds::e11006;

//declare the namespace for the histogram numbers
namespace dammIds {
    namespace experiment {
        const int D_DE = 1;
        const int D_GATE = 2;

        const int D_TAC1 = 3;
        const int D_TAC2 = 4;
        const int D_TAC3 = 5;

        const int D_BETA1 = 6;
        const int D_BETA2 = 7;

        const int DD_TOF_TAC1 = 10;
        const int DD_TOF_TAC2 = 11;
        const int DD_TOF_TAC3 = 12;
        const int DD_TOF_TAC1_GATED = 13;
        const int DD_TOF_TAC2_GATED = 14;
        const int DD_TOF_TAC3_GATED = 15;
    }
}

void E11006Processor::DeclarePlots(void) {
    //declare a single histogram
    DeclareHistogram1D(D_TAC1, SD, "TAC_1");
    DeclareHistogram1D(D_TAC2, SD, "TAC_2");
    DeclareHistogram1D(D_TAC3, SD, "TAC_3");
    DeclareHistogram1D(D_DE, SD, "DE");
    DeclareHistogram1D(D_BETA1, SD, "BETA_1");
    DeclareHistogram1D(D_BETA2, SD, "BETA_2");

    DeclareHistogram2D(DD_TOF_TAC1, SC, SB, "DE vs TOF TAC1");
    DeclareHistogram2D(DD_TOF_TAC2, SC, SB, "DE vs TOF TAC2");
    DeclareHistogram2D(DD_TOF_TAC3, SC, SB, "DE vs TOF TAC3");

    DeclareHistogram2D(DD_TOF_TAC1_GATED, SB, SB, "DE vs TOF TAC1");
    DeclareHistogram2D(DD_TOF_TAC2_GATED, SC, SB, "DE vs TOF TAC2");
    DeclareHistogram2D(DD_TOF_TAC3_GATED, SC, SB, "DE vs TOF TAC3");
}


E11006Processor::E11006Processor() : EventProcessor(OFFSET, RANGE,
                                                    "E11006Processor") {
    name = "E11006";
    //associate the processor with both the tac and scintillator event types
    associatedTypes.insert("tac");
    associatedTypes.insert("si");
    associatedTypes.insert("valid");
}

bool E11006Processor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    //get the list of all the tacs in the current event
    static const vector<ChanEvent *> &tacEvents =
            event.GetSummary("tac")->GetList();

    //get the list of all the scintillators with the subtype "de" in the
    //current event
    static const vector<ChanEvent *> &deEvents =
            event.GetSummary("si:de")->GetList();

    static const vector<ChanEvent *> &betaEvents =
            event.GetSummary("si:beta")->GetList();

    static const vector<ChanEvent *> &validEvents =
            event.GetSummary("valid")->GetList();

    bool isValidated = validEvents.size() > 0;


    for (vector<ChanEvent *>::const_iterator it = tacEvents.begin();
         it != tacEvents.end();
         it++) {
        //ChanEvent.hpp (under the long list of Get methods)
        //    unsigned int location = (*it)->GetChanID().GetLocation();
        string subType = (*it)->GetChanID().GetSubtype();
        double tac1_energy = 0.0;
        double tac2_energy = 0.0;
        double tac3_energy = 0.0;

        if (subType == "tac1") {
            tac1_energy = (*it)->GetEnergy();
            plot(D_TAC1, tac1_energy / 4.);
        }
        if (subType == "tac2") {
            tac2_energy = (*it)->GetEnergy();
            plot(D_TAC2, tac2_energy / 4.);
        }
        if (subType == "tac3") {
            tac3_energy = (*it)->GetEnergy();
            plot(D_TAC1, tac3_energy / 4.);
        }

        if (deEvents.size() > 0) {
            for (vector<ChanEvent *>::const_iterator itde = deEvents.begin();
                 itde != deEvents.end(); itde++) {
                if (subType == "tac1") {
                    plot(DD_TOF_TAC1, tac1_energy / 4.,
                         (*itde)->GetEnergy() / 4.);
                }
                if (subType == "tac2") {
                    plot(DD_TOF_TAC2, tac2_energy / 4.,
                         (*itde)->GetEnergy() / 4.);
                }
                if (subType == "tac3") {
                    plot(DD_TOF_TAC3, tac3_energy / 4.,
                         (*itde)->GetEnergy() / 4.);
                }
            }
        }
        if (deEvents.size() > 0 && isValidated == 1) {
            for (vector<ChanEvent *>::const_iterator itde = deEvents.begin();
                 itde != deEvents.end(); itde++) {
                if (subType == "tac1") {
                    plot(DD_TOF_TAC1_GATED, tac1_energy / 4.,
                         (*itde)->GetEnergy() / 4.);
                }
                if (subType == "tac2") {
                    plot(DD_TOF_TAC2_GATED, tac2_energy / 4.,
                         (*itde)->GetEnergy() / 4.);
                }
                if (subType == "tac3") {
                    plot(DD_TOF_TAC3_GATED, tac3_energy / 4.,
                         (*itde)->GetEnergy() / 4.);
                }
            }
        }

    }
    for (vector<ChanEvent *>::const_iterator it = deEvents.begin();
         it != deEvents.end();
         it++) {
        unsigned int location = (*it)->GetChanID().GetLocation();
        // string subType = (*it)->GetChanID().GetSubtype();

        if (location == 0)
            plot(D_DE, (*it)->GetEnergy() / 4.);
    }

    for (vector<ChanEvent *>::const_iterator it = validEvents.begin();
         it != validEvents.end(); it++) {
        // unsigned int location = (*it)->GetChanID().GetLocation();
        string subType = (*it)->GetChanID().GetSubtype();

        if (subType == "gate")
            plot(D_GATE, (*it)->GetEnergy() / 4.);
    }

    for (vector<ChanEvent *>::const_iterator it = betaEvents.begin();
         it != betaEvents.end();
         it++) {
        unsigned int location = (*it)->GetChanID().GetLocation();
        // string subType = (*it)->GetChanID().GetSubtype();

        if (location == 0)
            plot(D_BETA1, (*it)->GetEnergy() / 4.);
        if (location == 1)
            plot(D_BETA2, (*it)->GetEnergy() / 4.);

    }
    EndProcess();
    return true;
}
