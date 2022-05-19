/** @file MtasProcessor.cpp
 * @brief  Basic MtasProcessor for MTAS at FRIB
 * @authors T.T. King, T. Ruland, B.C. Rasco
 * @date 03/25/2022
 */

#include "MtasImplantSipmProcessor.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "DetectorLibrary.hpp"
#include "Globals.hpp"
#include "HelperFunctions.hpp"
#include "RawEvent.hpp"
#include "StringManipulationFunctions.hpp"

namespace dammIds::mtasimplant {
const unsigned int DD_ANODES_H_ENERGY = 0;
const unsigned int DD_ANODES_H_OQDC = 1;
const unsigned int DD_ANODES_H_TQDC = 2;
const unsigned int DD_ANODES_L_ENERGY = 3;
const unsigned int DD_ANODES_L_OQDC = 4;
const unsigned int DD_ANODES_L_TQDC = 5;
const unsigned int DD_DY_L_ENERGY = 6;
const unsigned int DD_DY_L_OQDC = 7;
const unsigned int DD_DY_L_TQDC = 8;
const unsigned int DD_DY_H_ENERGY = 9;
const unsigned int DD_DY_H_OQDC = 10;
const unsigned int DD_DY_H_TQDC = 11;
const unsigned int DD_SIPM_PIXEL_IMAGE_HG = 12;
const unsigned int DD_SIPM_PIXEL_IMAGE_LG = 13;
}  // namespace dammIds::mtasimplant

using namespace std;
using namespace dammIds::mtasimplant;

MtasImplantSipmProcessor::MtasImplantSipmProcessor() : EventProcessor(OFFSET, RANGE, "MtasImplantSipmProcessor") {
    associatedTypes.insert("mtasimplantsipm");
    EandQDC_down_scaling_ = 10.0;
    dammSiPm_pixelShifts = {3, 12};  // shift is first + xpos and second - ypos
}

void MtasImplantSipmProcessor::DeclarePlots(void) {
    DeclareHistogram2D(DD_ANODES_H_ENERGY, SD, S6, "Anode HG Pix Energy/10 vs DetLoc");
    DeclareHistogram2D(DD_ANODES_H_OQDC, SD, S6, "Anode HG OnBoard QDC/10 vs DetLoc");
    DeclareHistogram2D(DD_ANODES_H_TQDC, SD, S6, "Anode HG TQDC/10 vs DetLoc");
    DeclareHistogram2D(DD_ANODES_L_ENERGY, SD, S6, "Anode LG Pix Energy/10 vs DetLoc");
    DeclareHistogram2D(DD_ANODES_L_OQDC, SD, S6, "Anode LG OnBoard QDC/10 vs DetLoc");
    DeclareHistogram2D(DD_ANODES_L_TQDC, SD, S6, "Anode LG TQDC/10 vs DetLoc");
    DeclareHistogram2D(DD_DY_L_ENERGY, SD, S2, "DY LG Pix Energy/10 vs DetLoc");
    DeclareHistogram2D(DD_DY_L_OQDC, SD, S2, "DY LG OnBoard QDC/10 vs DetLoc");
    DeclareHistogram2D(DD_DY_L_TQDC, SD, S2, "DY LG TQDC/10 vs DetLoc");
    DeclareHistogram2D(DD_DY_H_ENERGY, SD, S2, "DY HG Pix Energy/10 vs DetLoc");
    DeclareHistogram2D(DD_DY_H_OQDC, SD, S2, "DY HG OnBoard QDC/10 vs DetLoc");
    DeclareHistogram2D(DD_DY_H_TQDC, SD, S2, "DY HG TQDC/10 vs DetLoc");
    DeclareHistogram2D(DD_SIPM_PIXEL_IMAGE_HG, S4, S4, "SiPM HG Hit Pattern");
    DeclareHistogram2D(DD_SIPM_PIXEL_IMAGE_LG, S4, S4, "SiPM LG Hit Pattern");
}

bool MtasImplantSipmProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    static const vector<ChanEvent *> &Anode_L = event.GetSummary("mtasimplantsipm:anode_l", true)->GetList();
    static const vector<ChanEvent *> &Anode_H = event.GetSummary("mtasimplantsipm:anode_h", true)->GetList();

    static const vector<ChanEvent *> &Dynode_H = event.GetSummary("mtasimplantsipm:dyn_h", true)->GetList();
    static const vector<ChanEvent *> &Dynode_L = event.GetSummary("mtasimplantsipm:dyn_l", true)->GetList();

    //!#########################################
    //!       ANODE LOW GAIN
    //!#########################################

    for (auto itAl : Anode_L) {
        if (itAl->IsSaturated() || itAl->IsPileup()) {
            continue;
        }
        double energy = itAl->GetCalibratedEnergy();
        int detLoc = itAl->GetChanID().GetLocation();

        //! Get the qdc from the recorded trace if it exists and has been analyized.
        double tqdc;
        if (itAl->GetTrace().HasValidWaveformAnalysis()) {
            tqdc = itAl->GetTrace().GetQdc();
        } else {
            tqdc = -999;
        }

        //! calculate the qdc from the onboard sums. returns -999 if no qdcsums are in the data stream.
        double oqdc = CalOnboardQDC(0, 2, itAl->GetQdc());

        plot(DD_ANODES_L_ENERGY, energy / EandQDC_down_scaling_, detLoc);
        if (tqdc != -999) {
            plot(DD_ANODES_L_OQDC, oqdc / EandQDC_down_scaling_, detLoc);
        }
        if (oqdc != -999) {
            plot(DD_ANODES_L_TQDC, tqdc / EandQDC_down_scaling_, detLoc);
        }

        pair<int, int> sipmPixels = ComputeSiPmPixelLoc(detLoc);

        plot(DD_SIPM_PIXEL_IMAGE_LG, sipmPixels.first + dammSiPm_pixelShifts.first, dammSiPm_pixelShifts.second - sipmPixels.second);  // x+3 and 12-y should center the image in a S4 by S4 histo
    }

    //!#########################################
    //!       ANODE HIGH GAIN
    //!#########################################

    for (auto itAh : Anode_H) {
        if (itAh->IsSaturated() || itAh->IsPileup()) {
            continue;
        }
        double energy = itAh->GetCalibratedEnergy();
        int detLoc = itAh->GetChanID().GetLocation();

        //! Get the qdc from the recorded trace if it exists and has been analyized.
        double tqdc;
        if (itAh->GetTrace().HasValidWaveformAnalysis()) {
            tqdc = itAh->GetTrace().GetQdc();
        } else {
            tqdc = -999;
        }

        //! calculate the qdc from the onboard sums. returns -999 if no qdcsums are in the data stream.
        double oqdc = CalOnboardQDC(0, 2, itAh->GetQdc());

        plot(DD_ANODES_H_ENERGY, energy / EandQDC_down_scaling_, detLoc);
        if (tqdc != -999) {
            plot(DD_ANODES_H_OQDC, oqdc / EandQDC_down_scaling_, detLoc);
        }
        if (oqdc != -999) {
            plot(DD_ANODES_H_TQDC, tqdc / EandQDC_down_scaling_, detLoc);
        }

        pair<int, int> sipmPixels = ComputeSiPmPixelLoc(detLoc);

        plot(DD_SIPM_PIXEL_IMAGE_LG, sipmPixels.first + dammSiPm_pixelShifts.first, dammSiPm_pixelShifts.second - sipmPixels.second);  // x+3 and 12-y should center the image in a S4 by S4 histo
    }

    //!#########################################
    //!       DYNODE LOW GAIN
    //!#########################################

    for (auto itDyL : Dynode_L) {
        if (itDyL->IsSaturated() || itDyL->IsPileup()) {
            continue;
        }
        double energy = itDyL->GetCalibratedEnergy();
        int detLoc = itDyL->GetChanID().GetLocation();

        //! Get the qdc from the recorded trace if it exists and has been analyized.
        double tqdc;
        if (itDyL->GetTrace().HasValidWaveformAnalysis()) {
            tqdc = itDyL->GetTrace().GetQdc();
        } else {
            tqdc = -999;
        }

        //! calculate the qdc from the onboard sums. returns -999 if no qdcsums are in the data stream.
        double oqdc = CalOnboardQDC(0, 2, itDyL->GetQdc());

        plot(DD_DY_L_ENERGY, energy / EandQDC_down_scaling_, detLoc);
        if (tqdc != -999) {
            plot(DD_DY_L_OQDC, oqdc / EandQDC_down_scaling_, detLoc);
        }
        if (oqdc != -999) {
            plot(DD_DY_L_TQDC, tqdc / EandQDC_down_scaling_, detLoc);
        }
    }

    //!#########################################
    //!       DYNODE HIGH GAIN
    //!#########################################

    for (auto itDyH : Dynode_H) {
        if (itDyH->IsSaturated() || itDyH->IsPileup()) {
            continue;
        }
        double energy = itDyH->GetCalibratedEnergy();
        int detLoc = itDyH->GetChanID().GetLocation();

        //! Get the qdc from the recorded trace if it exists and has been analyized.
        double tqdc;
        if (itDyH->GetTrace().HasValidWaveformAnalysis()) {
            tqdc = itDyH->GetTrace().GetQdc();
        } else {
            tqdc = -999;
        }

        //! calculate the qdc from the onboard sums. returns -999 if no qdcsums are in the data stream.
        double oqdc = CalOnboardQDC(0, 2, itDyH->GetQdc());

        plot(DD_DY_L_ENERGY, energy / EandQDC_down_scaling_, detLoc);
        if (tqdc != -999) {
            plot(DD_DY_L_OQDC, oqdc / EandQDC_down_scaling_, detLoc);
        }
        if (oqdc != -999) {
            plot(DD_DY_L_TQDC, tqdc / EandQDC_down_scaling_, detLoc);
        }
    }

    return true;
}

bool MtasImplantSipmProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    EndProcess();
    return true;
}

double MtasImplantSipmProcessor::CalOnboardQDC(int bkg, int waveform, const std::vector<unsigned> oqdcs, double bkgTimeScaling, double waveformTimeScaling) {
    if (!oqdcs.empty()) {
        return ((waveformTimeScaling * oqdcs.at(waveform)) - (bkgTimeScaling * oqdcs.at(bkg)));
    } else {
        return -999;
    }
}

pair<int, int> MtasImplantSipmProcessor::ComputeSiPmPixelLoc(int xmlLocation_) {
    int shifted_XML_loc_ = xmlLocation_ + 1;    //! shifting to 1 counting because of the modulus. a 0 return for the modulus means its evenly divisable, but 0 % 8 also gives 0, so for clarity im shifting
    int y = floor(shifted_XML_loc_ / 8.0) + 1;  // 1 counting the rows and columns
    int x = shifted_XML_loc_ % 8;               //! Modulus returns the integer remainder i.e. 10 % 8 gives 2

    return (make_pair(x, y));  //! returning "raw" positions so the plot offets are clearly in the plot command.
}