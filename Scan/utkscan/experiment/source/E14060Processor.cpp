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
        const int D_TRACE_MAX_DYNODE = 0;//!< Max value of the Dynode trace.
        const int DD_EPIN1_VS_TOF_PIN1_I2N = 1; //!<PIN1 vs. ToF between PIN1
//!< and I2N
        const int DD_EPIN1_VS_TOF_PIN1_I2S = 2; //!<PIN1 vs. ToF between PIN1
//!< and I2S
        const int DD_EPIN2_VS_TOF_PIN2_I2N = 3; //!<PIN2 vs. ToF between PIN2
//!< and I2N
        const int DD_EPIN2_VS_TOF_PIN2_I2S = 4; //!<PIN2 vs. ToF between PIN2
//!< and ISS
        const int DD_EPIN1_VS_TOF_I2N_I2S = 5; //!<PIN1 vs. ToF between I2N
//!< and I2S
        const int DD_TOF_I2NS_VS_TOF_PIN1_I2S = 6;
        const int DD_PIN1_VS_PIN2 = 7; //!< PIN1 vs. PIN2

        namespace DECAY_GATED {
            const int DD_QDCVSTOF = 8; //!<QDC vs ToF
            const int DD_HAGRID = 9; //!< Location vs. HAGRiD En
            const int DD_NAI = 10; //!< Location vs. NaI En
        }

        namespace PSPMT_GATED {
            const int DD_EPIN1_VS_TOF_PIN1_I2N = 11; //!<PIN1 vs. ToF between
//!< PIN1 and I2N anti-gated with the PSPMT signal
        }

        namespace IMPLANT_GATED {
            const int DD_QDCVSTOF = 12; //!<QDC vs ToF
            const int DD_EPIN1_VS_TOF_PIN1_I2N = 13; //!<PIN1 vs. ToF between
//!< PIN1 and I2N
        }

        namespace PIN_GATED {
            const int DD_PSPMT_POS = 14; //!< PSPMT Position Gated with PIN
        }

        namespace GE_GATED {
            const int DD_EPIN1_VS_TOF_PIN1_I2N = 15; //!<PIN1 vs. ToF between
//!< PIN1 and I2N
            const int DD_EPIN1_VS_GE = 16; //!< PIN1 Energy vs. Ge energy
        }
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::experiment;

void E14060Processor::DeclarePlots(void) {
    DeclareHistogram1D(D_TRACE_MAX_DYNODE, SC, "Trace Max Dynode");
    DeclareHistogram2D(DD_EPIN1_VS_TOF_PIN1_I2N, SB, SB, "EPIN1 vs. ToF"
            "(I2N-PIN1)");
    DeclareHistogram2D(DD_EPIN1_VS_TOF_PIN1_I2S, SB, SB, "EPIN1 vs. ToF"
            "(I2S-PIN1)");
    DeclareHistogram2D(DD_EPIN2_VS_TOF_PIN2_I2N, SB, SB, "EPIN2 vs. ToF"
            "(I2N-PIN2)");
    DeclareHistogram2D(DD_EPIN2_VS_TOF_PIN2_I2S, SB, SB, "EPIN2 vs. ToF"
            "(I2S-PIN2)");
    DeclareHistogram2D(DD_EPIN1_VS_TOF_I2N_I2S, SB, SB, "Si Energy vs. TOF"
            "(I2N-I2S)");
    //DeclareHistogram2D(DD_TOF_I2NS_VS_TOF_PIN1_I2S, )

    DeclareHistogram2D(DD_PIN1_VS_PIN2, SB, SB, "EPin1 vs. EPin2");

    //----- Histograms gated with decays
    DeclareHistogram2D(DECAY_GATED::DD_QDCVSTOF, SC, SD, "Decay - QDC vs. ToF");

    //----- Histograms gated on implants
    DeclareHistogram2D(IMPLANT_GATED::DD_QDCVSTOF, SC, SD, "Implant - QDC vs. "
            "ToF");
    DeclareHistogram2D(IMPLANT_GATED::DD_EPIN1_VS_TOF_PIN1_I2N, SB, SB,
                       "EPIN1 vs. ToF (I2N-PIN1) - YAP");

    //---------- Histograms Gated with the PIN
    DeclareHistogram2D(PIN_GATED::DD_PSPMT_POS, SB, SB,
                       "PSPMT Pos - Pin Gated");

    //----------- Histograms gated with the Clover
    DeclareHistogram2D(GE_GATED::DD_EPIN1_VS_TOF_PIN1_I2N, SB, SB,
                       "EPIN1 vs. ToF (I2N-PIN1) - YAP");
    DeclareHistogram2D(GE_GATED::DD_EPIN1_VS_GE, SC, SB, "EPIN1 vs. Ge");

    //----------- Histograms gated with the PSPMT
    DeclareHistogram2D(PSPMT_GATED::DD_EPIN1_VS_TOF_PIN1_I2N, SB, SB,
                       "EPIN1 vs. ToF (I2N-PIN1)");
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

    //Check to see if we had a veto detector in the event.
    static bool hasVeto = event.GetSummary("generic:veto")->GetMult() != 0;

    //Loop over the Ge events to ensure that we had ourselves something in
    // the region of interest. We also take the time to plot the PIN1 energy
    // vs. Ge Energy
    bool hasGe = false;
    for (vector<ChanEvent *>::const_iterator iterator2 = geEvts.begin();
         iterator2 != geEvts.end(); iterator2++) {
        if ((*iterator2)->GetCalEnergy() > energyRange_.first &&
            (*iterator2)->GetCalEnergy() < energyRange_.second)
            hasGe = true;
        for (vector<ChanEvent *>::const_iterator iterator1 = pin.begin();
             iterator1 != pin.end(); iterator1++)
            if ((*iterator1)->GetChanID().GetSubtype() == "de1")
                plot(GE_GATED::DD_EPIN1_VS_GE, (*iterator2)->GetCalEnergy(),
                     (*iterator1)->GetCalEnergy());
    }

    //Loop over the dynode events to plot the maximum value
    for (TimingMap::const_iterator iterator3 = tdynode.begin();
         iterator3 != tdynode.end(); iterator3++)
        plot(D_TRACE_MAX_DYNODE, iterator3->second.GetMaximumValue());

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
    string tac = "", pin = "";
    double tac_energy = 0.0, pin_energy = 0.0;
    double i2ns = 0.0, pin1_i2n = 0.0;
    for (vector<ChanEvent *>::const_iterator it = tacs.begin();
         it != tacs.end(); it++) {
        tac = (*it)->GetChanID().GetSubtype();
        tac_energy = (*it)->GetCalEnergy();
        if(tac == "i2n_i2s")
            i2ns = tac_energy;
        if(tac == "pin1_i2n")
            pin1_i2n = tac_energy;
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
                    plot(IMPLANT_GATED::DD_EPIN1_VS_TOF_PIN1_I2N, tac_energy,
                         pin_energy);
                if(!hasImplant)
                    plot(PSPMT_GATED::DD_EPIN1_VS_TOF_PIN1_I2N, tac_energy,
                        pin_energy);
            }
            if (tac == "pin1_i2s" && pin == "de1")
                plot(DD_EPIN1_VS_TOF_PIN1_I2S, tac_energy, pin_energy);
            if (tac == "pin2_i2n" && pin == "de2")
                plot(DD_EPIN2_VS_TOF_PIN2_I2N, tac_energy, pin_energy);
            if (tac == "pin2_i2s" && pin == "de2")
                plot(DD_EPIN2_VS_TOF_PIN2_I2S, tac_energy, pin_energy);
        }
    }
    if(i2ns != 0.0 && pin1_i2n != 0.0)
        plot(DD_TOF_I2NS_VS_TOF_PIN1_I2S, pin1_i2n, i2ns);
}