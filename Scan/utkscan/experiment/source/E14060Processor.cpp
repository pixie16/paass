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
        const int DD_EPIN1_VS_TOF_PIN1_I2N = 0; //!<PIN1 vs. ToF between PIN1
//!< and I2N
        const int DD_EPIN1_VS_TOF_PIN1_I2S = 1; //!<PIN1 vs. ToF between PIN1
//!< and I2S
        const int DD_EPIN2_VS_TOF_PIN2_I2N = 2; //!<PIN2 vs. ToF between PIN2
//!< and I2N
        const int DD_EPIN2_VS_TOF_PIN2_I2S = 3; //!<PIN2 vs. ToF between PIN2
//!< and ISS
        const int DD_EPIN1_VS_TOF_I2N_I2S = 4; //!<PIN1 vs. ToF between I2N
//!< and I2S
        const int DD_PIN1_VS_PIN2 = 5; //!< PIN1 vs. PIN2

        namespace DECAY_GATED {
            const int DD_QDCVSTOF = 6; //!<QDC vs ToF
            const int DD_HAGRID = 7; //!< Location vs. HAGRiD En
            const int DD_NAI = 8; //!< Location vs. NaI En
        }

        namespace IMPLANT_GATED {
            const int DD_QDCVSTOF = 7; //!<QDC vs ToF
        }

        namespace PIN_GATED {
            const int DD_PSPMT_POS = 8; //!< PSPMT Position Gated with PIN
        }

        namespace PSPMT_GATED {
            const int DD_EPIN1_VS_TOF_PIN1_I2N = 9; //!<PIN1 vs. ToF between
//!< PIN1 and I2N
            const int DD_EPIN1_VS_TOF_PIN1_I2S = 10; //!<PIN1 vs. ToF between
//!< PIN1 and I2S
            const int DD_EPIN2_VS_TOF_PIN2_I2N = 11; //!<PIN2 vs. ToF between
//!< PIN2 and I2N
            const int DD_EPIN2_VS_TOF_PIN2_I2S = 12; //!<PIN2 vs. ToF between
//!< PIN2 and ISS
        }

        namespace GE_GATED {
            const int DD_EPIN1_VS_TOF_PIN1_I2N = 13; //!<PIN1 vs. ToF between
//!< PIN1 and I2N
            const int DD_EPIN1_VS_TOF_PIN1_I2S = 14; //!<PIN1 vs. ToF between
//!< PIN1 and I2S
            const int DD_EPIN2_VS_TOF_PIN2_I2N = 15; //!<PIN2 vs. ToF between
//!< PIN2 and I2N
            const int DD_EPIN2_VS_TOF_PIN2_I2S = 16; //!<PIN2 vs. ToF between
//!< PIN2 and ISS
        }
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::experiment;

void E14060Processor::DeclarePlots(void) {
    DeclareHistogram2D(DD_EPIN1_VS_TOF_PIN1_I2N, SC, SC, "EPIN1 vs. ToF"
            "(I2N-PIN1)");
    DeclareHistogram2D(DD_EPIN1_VS_TOF_PIN1_I2S, SC, SC, "EPIN1 vs. ToF"
            "(I2S-PIN1)");
    DeclareHistogram2D(DD_EPIN2_VS_TOF_PIN2_I2N, SC, SC, "EPIN2 vs. ToF"
            "(I2N-PIN2)");
    DeclareHistogram2D(DD_EPIN2_VS_TOF_PIN2_I2S, SC, SC, "EPIN2 vs. ToF"
            "(I2S-PIN2)");
    DeclareHistogram2D(DD_EPIN1_VS_TOF_I2N_I2S, SC, SC, "Si Energy vs. TOF"
            "(I2N-I2S)");
    DeclareHistogram2D(DD_PIN1_VS_PIN2, SC, SC, "EPin1 vs. EPin2");

    //----- Histograms gated with decays
    DeclareHistogram2D(DECAY_GATED::DD_QDCVSTOF, SC, SD, "Decay - QDC vs. ToF");

    //----- Histograms gated on implants
    DeclareHistogram2D(IMPLANT_GATED::DD_QDCVSTOF, SC, SD, "Implant - QDC vs. "
            "ToF");

    //---------- Histograms Gated with the PIN
    DeclareHistogram2D(PIN_GATED::DD_PSPMT_POS, SC, SC,
                       "PSPMT Pos - Pin Gated");

    //---------- Histograms Gated with the PSPMT
    DeclareHistogram2D(PSPMT_GATED::DD_EPIN1_VS_TOF_PIN1_I2N, SC, SC,
                       "EPIN1 vs. ToF (I2N-PIN1) - YAP");
    DeclareHistogram2D(PSPMT_GATED::DD_EPIN1_VS_TOF_PIN1_I2S, SC, SC,
                       "EPIN1 vs. ToF (I2S-PIN1) - YAP");
    DeclareHistogram2D(PSPMT_GATED::DD_EPIN2_VS_TOF_PIN2_I2N, SC, SC,
                       "EPIN2 vs. ToF (I2N-PIN2) - YAP");
    DeclareHistogram2D(PSPMT_GATED::DD_EPIN2_VS_TOF_PIN2_I2S, SC, SC,
                       "EPIN2 vs. ToF (I2S-PIN2) - YAP");

    //----------- Histograms gated with the Clover
    DeclareHistogram2D(GE_GATED::DD_EPIN1_VS_TOF_PIN1_I2N, SC, SC,
                       "EPIN1 vs. ToF (I2N-PIN1) - YAP");
    DeclareHistogram2D(GE_GATED::DD_EPIN1_VS_TOF_PIN1_I2S, SC, SC,
                       "EPIN1 vs. ToF (I2S-PIN1) - YAP");
    DeclareHistogram2D(GE_GATED::DD_EPIN2_VS_TOF_PIN2_I2N, SC, SC,
                       "EPIN2 vs. ToF (I2N-PIN2) - YAP");
    DeclareHistogram2D(GE_GATED::DD_EPIN2_VS_TOF_PIN2_I2S, SC, SC,
                       "EPIN2 vs. ToF (I2S-PIN2) - YAP");
}

E14060Processor::E14060Processor(std::pair<double, double> &energyRange) :
        EventProcessor(OFFSET, RANGE, "E14060Processor") {
    SetAssociatedTypes();
    energyRange_ = energyRange;
}

void E14060Processor::SetAssociatedTypes() {
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
    pair<unsigned int, unsigned int> pixel;

    if (event.GetSummary("vandle")->GetList().size() != 0)
        vbars = ((VandleProcessor *) DetectorDriver::get()->
                GetProcessor("VandleProcessor"))->GetBars();
    if (event.GetSummary("pspmt:anode")->GetList().size() != 0) {
        position = ((PspmtProcessor *) DetectorDriver::get()->
                GetProcessor("PspmtProcessor"))->GetPosition("pixie");
        pixel = ((PspmtProcessor *) DetectorDriver::get()->
                GetProcessor("PspmtProcessor"))->GetPosition("pixie");
    }
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

    static const vector<ChanEvent *> &nai =
            event.GetSummary("nai")->GetList();

    static const vector<ChanEvent *> &tac =
            event.GetSummary("tac", true)->GetList();

    static const vector<ChanEvent *> &pin =
            event.GetSummary("pin", true)->GetList();

    bool hasGe = false;
    for(vector<ChanEvent *>::const_iterator iterator2 = geEvts.begin();
        iterator2 != geEvts.end(); iterator2++) {
        if((*iterator2)->GetCalEnergy() > energyRange_.first &&
           (*iterator2)->GetCalEnergy() < energyRange_.second)
            hasGe = true;
    }

    //Basic correlation information
    bool hasIon = pin.size() != 0;
    bool hasImplant = hasIon && dynode.size() != 0;
    bool hasDecay = !hasIon && dynode.size() != 0;

    if (tac.size() != 0 && pin.size() != 0)
        PlotPid(tac, pin, hasGe, hasImplant);

    if (hasDecay) {
        for (vector<ChanEvent *>::const_iterator iterator1 = hagrid.begin();
             iterator1 != hagrid.end(); iterator1++)
            plot(DECAY_GATED::DD_HAGRID, (*iterator1)->GetCalEnergy(),
                 (*iterator1)->GetChanID().GetLocation());
        for (vector<ChanEvent *>::const_iterator iterator1 = nai.begin();
             iterator1 != nai.end(); iterator1++)
            plot(DECAY_GATED::DD_NAI, (*iterator1)->GetCalEnergy(),
                 (*iterator1)->GetChanID().GetLocation());
    }

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

            if (hasDecay)
                plot(DECAY_GATED::DD_QDCVSTOF, tof * plotMult + plotOffset,
                     bar.GetQdc());
            if (hasImplant)
                plot(IMPLANT_GATED::DD_QDCVSTOF, tof * plotMult + plotOffset,
                     bar.GetQdc());
        }//for(TimingMap::iterator start = tdynode.begin();
    } //for(BarMap::iterator it = vbars.begin()

    EndProcess();
    return (true);
}

void E14060Processor::PlotPid(const std::vector<ChanEvent *> &tacs,
                              const std::vector<ChanEvent *> &pins,
                              const bool &hasGe, const bool &hasImplant) {

    //Need to have YAP and Ge Gated spectra as well.
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
            if (tac == "pin1_i2n" && pin == "de1") {
                plot(DD_EPIN1_VS_TOF_PIN1_I2N, tac_energy, pin_energy);
                if (hasGe)
                    plot(GE_GATED::DD_EPIN1_VS_TOF_PIN1_I2N, tac_energy,
                         pin_energy);
                if (hasImplant)
                    plot(PSPMT_GATED::DD_EPIN1_VS_TOF_PIN1_I2N, tac_energy,
                         pin_energy);
            }
            if (tac == "pin1_i2s" && pin == "de1") {
                plot(DD_EPIN1_VS_TOF_PIN1_I2S, tac_energy, pin_energy);
                if (hasGe)
                    plot(GE_GATED::DD_EPIN1_VS_TOF_PIN1_I2S, tac_energy,
                         pin_energy);
                if (hasImplant)
                    plot(PSPMT_GATED::DD_EPIN1_VS_TOF_PIN1_I2S, tac_energy,
                         pin_energy);
            }
            if (tac == "pin2_i2n" && pin == "de2") {
                plot(DD_EPIN2_VS_TOF_PIN2_I2N, tac_energy, pin_energy);
                if (hasGe)
                    plot(GE_GATED::DD_EPIN2_VS_TOF_PIN2_I2N, tac_energy,
                         pin_energy);
                if (hasImplant)
                    plot(PSPMT_GATED::DD_EPIN2_VS_TOF_PIN2_I2N, tac_energy,
                         pin_energy);
            }
            if (tac == "pin2_i2s" && pin == "de2") {
                plot(DD_EPIN2_VS_TOF_PIN2_I2S, tac_energy, pin_energy);
                if (hasGe)
                    plot(GE_GATED::DD_EPIN2_VS_TOF_PIN2_I2S, tac_energy,
                         pin_energy);
                if (hasImplant)
                    plot(PSPMT_GATED::DD_EPIN2_VS_TOF_PIN2_I2S, tac_energy,
                         pin_energy);
            }
        }
    }
}