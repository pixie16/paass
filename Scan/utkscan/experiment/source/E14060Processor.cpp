/** \file E14060Processor.cpp
 * \brief Experiment specific processor to handle the e14060 experiment at
 * the NSCL.
 *\author S. V. Paulauskas
 *\date September 15, 2016
 */
#include <iostream>
#include <TimingMapBuilder.hpp>

#include "DetectorDriver.hpp"
#include "GeProcessor.hpp"
#include "E14060Processor.hpp"
#include "PspmtProcessor.hpp"
#include "VandleProcessor.hpp"

namespace dammIds {
    namespace experiment {
        const int DD_QDCVSTOF = 0; //!<QDC vs ToF
        const int DD_EPIN1_VS_TOF_PIN1_I2N = 1; //!<PIN1 vs. ToF between PIN1 and I2N
        const int DD_EPIN1_VS_TOF_PIN1_I2S = 2; //!<PIN1 vs. ToF between PIN1 and I2S
        const int DD_EPIN2_VS_TOF_PIN2_I2N = 3; //!<PIN2 vs. ToF between PIN2 and I2N
        const int DD_EPIN2_VS_TOF_PIN2_I2S = 4; //!<PIN2 vs. ToF between PIN2 and ISS
        const int DD_EPIN1_VS_TOF_I2N_I2S = 5; //!<PIN1 vs. ToF between I2N and I2S
        const int DD_PIN1_VS_PIN2 = 6; //!< PIN1 vs. PIN2
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::experiment;

void E14060Processor::DeclarePlots(void) {
    DeclareHistogram2D(DD_QDCVSTOF, SC, SD, "QDC CTof- No Tape Move");
    DeclareHistogram2D(DD_EPIN1_VS_TOF_PIN1_I2N, SC, SC, "Si Energy vs. TAC1");
    DeclareHistogram2D(DD_EPIN1_VS_TOF_PIN1_I2S, SC, SC, "Si Energy vs. TAC2");
    DeclareHistogram2D(DD_EPIN2_VS_TOF_PIN2_I2N, SC, SC, "Si Energy vs. TAC3");
    DeclareHistogram2D(DD_EPIN2_VS_TOF_PIN2_I2S, SC, SC, "Si Energy vs. TAC4");
    DeclareHistogram2D(DD_EPIN1_VS_TOF_I2N_I2S, SC, SC, "Si Energy vs. TAC5");
}

E14060Processor::E14060Processor() : EventProcessor(OFFSET, RANGE,
                                                    "E14060PRocessor") {
    associatedTypes.insert("vandle");
    associatedTypes.insert("hagrid");
    associatedTypes.insert("pspmt");
    associatedTypes.insert("ge");
    associatedTypes.insert("tac");
    associatedTypes.insert("si");
}

bool E14060Processor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);

    static const double plotMult = 2;
    static const double plotOffset = 1000;

    BarMap vbars;
    vector<ChanEvent *> geEvts;
    vector<vector<AddBackEvent>> geAddback;
    pair<double, double> position;

    if (event.GetSummary("vandle")->GetList().size() != 0)
        vbars = ((VandleProcessor *) DetectorDriver::get()->
                GetProcessor("VandleProcessor"))->GetBars();
    if (event.GetSummary("pspmt:anode")->GetList().size() != 0)
        position = ((PspmtProcessor *) DetectorDriver::get()->
                GetProcessor("PspmtProcessor"))->GetPosition("qdc");
    if (event.GetSummary("ge")->GetList().size() != 0) {
        geEvts = ((GeProcessor *) DetectorDriver::get()->
                GetProcessor("GeProcessor"))->GetGeEvents();
        geAddback = ((GeProcessor *) DetectorDriver::get()->
                GetProcessor("GeProcessor"))->GetAddbackEvents();
    }

    static const vector<ChanEvent *> &dynode =
            event.GetSummary("pspmt:dynode")->GetList();

    TimingMapBuilder startbuilder(dynode);
    TimingMap tdynode = startbuilder.GetMap();

    static const vector<ChanEvent *> &hagrid =
            event.GetSummary("hagrid")->GetList();

    static const vector<ChanEvent *> &tac =
            event.GetSummary("tac", true)->GetList();

    static const vector<ChanEvent *> &pin =
            event.GetSummary("si:pin", true)->GetList();

    if (tac.size() != 0 && pin.size() != 0)
        PlotPid(tac, pin);

    //Here we will check some of the correlation information
    bool hasIon = pin.size() != 0;
    bool hasImplant = hasIon && dynode.size() != 0;
    bool hasDecay = !hasIon && dynode.size() != 0;


    for (BarMap::iterator it = vbars.begin(); it != vbars.end(); it++) {
        BarDetector bar = (*it).second;

        if (!bar.GetHasEvent())
            continue;

        TimingCalibration cal = bar.GetCalibration();

        for (TimingMap::iterator itStart = tdynode.begin();
             itStart != tdynode.end(); itStart++) {

            if (!(*itStart).second.GetIsValid())
                continue;

            unsigned int startLoc = (*itStart).first.first;
            HighResTimingData start = (*itStart).second;

            double tof = bar.GetCorTimeAve() -
                         start.GetCorrectedTime() + cal.GetTofOffset(startLoc);

            plot(DD_QDCVSTOF, tof * plotMult + plotOffset, bar.GetQdc());
        }//for(TimingMap::iterator start = tdynode.begin();
    } //for(BarMap::iterator it = vbars.begin()

    EndProcess();
    return (true);
}

void E14060Processor::PlotPid(const std::vector<ChanEvent *> &tacs,
                              const std::vector<ChanEvent *> &pins) {
    string tac = "", pin = "";
    double tac_energy = 0.0, pin_energy = 0.0;
    for (vector<ChanEvent *>::const_iterator it = tacs.begin();
         it != tacs.end(); it++) {
        tac = (*it)->GetChanID().GetSubtype();
        tac_energy = (*it)->GetCalEnergy();
        for (vector<ChanEvent *>::const_iterator iterator1 = pins.begin();
             iterator1 != pins.end(); iterator1++) {
            pin = (*iterator1)->GetChanID().GetSubtype();
            pin_energy = (*iterator1)->GetCalEnergy();
            if (tac == "pin1_i2n" && pin == "de1")
                plot(DD_EPIN1_VS_TOF_PIN1_I2N, tac_energy, pin_energy);
            if (tac == "pin1_i2s" && pin=="de1")
                plot(DD_EPIN1_VS_TOF_PIN1_I2S, tac_energy, pin_energy);
            if (tac == "pin2_i2n" && pin == "de2")
                plot(DD_EPIN2_VS_TOF_PIN2_I2N, tac_energy, pin_energy);
            if (tac == "pin2_i2s" && pin == "de2")
                plot(DD_EPIN2_VS_TOF_PIN2_I2S, tac_energy, pin_energy);
        }
    }
}