/** @file MtasImplantSipmProcessor.cpp
 * @brief  Basic MtasImplantSipmProcessor for MTAS at FRIB
 * @authors T.T. King, I. Cox, D. Hoskins (WHO ELSE?)
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

namespace dammIds {
namespace mtasimplant {
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
const unsigned int DD_SIPM_HIRES_IMAGE_LG = 14;
const unsigned int DD_SIPM_HIRES_IMAGE_HG = 15;
const unsigned int DD_SIPM_HIRES_IMAGE_LG_QDC = 16;
const unsigned int DD_SIPM_HIRES_IMAGE_HG_QDC = 17;
const unsigned int DD_SIPM_HIRES_IMAGE_LG_QDC_THRESH = 18;
const unsigned int DD_SIPM_HIRES_IMAGE_HG_QDC_THRESH = 19;
const unsigned int DD_SIPM_HIRES_IMAGE_LG_THRESH = 20;
const unsigned int DD_SIPM_HIRES_IMAGE_HG_THRESH = 21;
const unsigned int DD_HG_DY_HG_ANODE = 22;
const unsigned int DD_LG_DY_LG_ANODE = 23;
const unsigned int DD_HG_DY_LG_ANODE = 24;
const unsigned int DD_LG_DY_HG_ANODE = 25;
const unsigned int DD_MULTIS = 30;
}  // namespace mtasimplant
}  // namespace dammIds

using namespace std;
using namespace dammIds::mtasimplant;

MtasImplantSipmProcessor::MtasImplantSipmProcessor(double yso_scale_, double yso_offset_, double yso_thresh_, double oqdc_yso_thresh_, double dyh_thresh_, double dyh_qdc_thresh_, double dyl_thresh_, double dyl_qdc_thresh_, double dyh_upperThresh_) : EventProcessor(OFFSET, RANGE, "MtasImplantSipmProcessor") {
    associatedTypes.insert("mtasimplantsipm");
    EandQDC_down_scaling_ = 100.0;
    dammSiPm_pixelShifts = {3, 12};  // shift is first + xpos and second - ypos

    yso_scale = yso_scale_;
    yso_offset = yso_offset_;
    yso_thresh = yso_thresh_;
    oqdc_yso_thresh = oqdc_yso_thresh_;
    dyh_thresh =  dyh_thresh_;
    dyh_qdc_thresh =  dyh_qdc_thresh_;  
    dyl_thresh =  dyl_thresh_;
    dyl_qdc_thresh =  dyl_qdc_thresh_;
    dyh_upperThresh = dyh_upperThresh_;
}

void MtasImplantSipmProcessor::DeclarePlots(void) {
    DeclareHistogram2D(DD_ANODES_H_ENERGY, SD, S7, "Anode HG Pix Energy/10 vs DetLoc");
    DeclareHistogram2D(DD_ANODES_H_OQDC, SD, S7, "Anode HG OnBoard QDC/10 vs DetLoc");
    DeclareHistogram2D(DD_ANODES_H_TQDC, SD, S7, "Anode HG TQDC/10 vs DetLoc");
    DeclareHistogram2D(DD_ANODES_L_ENERGY, SD, S7, "Anode LG Pix Energy/10 vs DetLoc");
    DeclareHistogram2D(DD_ANODES_L_OQDC, SD, S7, "Anode LG OnBoard QDC/10 vs DetLoc");
    DeclareHistogram2D(DD_ANODES_L_TQDC, SD, S7, "Anode LG TQDC/10 vs DetLoc");
    DeclareHistogram2D(DD_DY_L_ENERGY, SD, S2, "DY LG Pix Energy/10 vs DetLoc");
    DeclareHistogram2D(DD_DY_L_OQDC, SD, S2, "DY LG OnBoard QDC/10 vs DetLoc");
    DeclareHistogram2D(DD_DY_L_TQDC, SD, S2, "DY LG TQDC/10 vs DetLoc");
    DeclareHistogram2D(DD_DY_H_ENERGY, SD, S2, "DY HG Pix Energy/10 vs DetLoc");
    DeclareHistogram2D(DD_DY_H_OQDC, SD, S2, "DY HG OnBoard QDC/10 vs DetLoc");
    DeclareHistogram2D(DD_DY_H_TQDC, SD, S2, "DY HG TQDC/10 vs DetLoc");
    DeclareHistogram2D(DD_SIPM_PIXEL_IMAGE_HG, S4, S4, "SiPM HG Hit Pattern");
    DeclareHistogram2D(DD_SIPM_PIXEL_IMAGE_LG, S4, S4, "SiPM LG Hit Pattern");
    DeclareHistogram2D(DD_SIPM_HIRES_IMAGE_LG, SB, SB, " High Res LG Image Filter Energy");
    DeclareHistogram2D(DD_SIPM_HIRES_IMAGE_HG, SB, SB, " High Res HG Image Filter Energy");
    DeclareHistogram2D(DD_SIPM_HIRES_IMAGE_LG_QDC, SB, SB, " High Res LG QDC");
    DeclareHistogram2D(DD_SIPM_HIRES_IMAGE_HG_QDC, SB, SB, " High Res HG QDC");

    DeclareHistogram2D(DD_SIPM_HIRES_IMAGE_LG_THRESH, SB, SB, "THRESHL:: High Res LG Image Filter Energy");
    DeclareHistogram2D(DD_SIPM_HIRES_IMAGE_HG_THRESH, SB, SB, "THRESHL:: High Res HG Image Filter Energy");
    DeclareHistogram2D(DD_SIPM_HIRES_IMAGE_LG_QDC_THRESH, SB, SB, "THRESHL:: High Res LG QDC");
    DeclareHistogram2D(DD_SIPM_HIRES_IMAGE_HG_QDC_THRESH, SB, SB, "THRESHL:: High Res HG QDC");
    DeclareHistogram2D(DD_HG_DY_HG_ANODE,SC,SC,"High Gain Dynode High Gain Anode");
    DeclareHistogram2D(DD_LG_DY_LG_ANODE,SC,SC,"Low Gain Dynode Low Gain Anode");
    DeclareHistogram2D(DD_LG_DY_HG_ANODE,SC,SC,"Low Gain Dynode High Gain Anode");
    DeclareHistogram2D(DD_HG_DY_LG_ANODE,SC,SC,"High Gain Dynode Low Gain Anode");
      DeclareHistogram2D(DD_MULTIS,S9,S3, "SiPm Multis: DYH,DYL,ANH,ANL");
}

bool MtasImplantSipmProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;
    // cout<<"start Preprocess"<<endl;
    const vector<ChanEvent *> &Anode_L = event.GetSummary("mtasimplantsipm:anode_l", true)->GetList();
    const vector<ChanEvent *> &Anode_H = event.GetSummary("mtasimplantsipm:anode_h", true)->GetList();

    const vector<ChanEvent *> &Dynode_H = event.GetSummary("mtasimplantsipm:dyn_h", true)->GetList();
    const vector<ChanEvent *> &Dynode_L = event.GetSummary("mtasimplantsipm:dyn_l", true)->GetList();

    plot(DD_MULTIS,Dynode_H.size(),0);
    plot(DD_MULTIS,Dynode_L.size(),1);
    plot(DD_MULTIS,Anode_H.size(),2);
    plot(DD_MULTIS,Anode_L.size(),3);

    vector<double> anodeL_energyList_for_calculations(64, 0.0);
    vector<double> anodeH_energyList_for_calculations(64, 0.0);
    vector<vector<double>> anode_L_positionMatrix(8, vector<double>(8, 0.0));  //! make a vector of vectors initialized to 0 (note the "stacked" vector constructor)
    vector<vector<double>> anode_H_positionMatrix(8, vector<double>(8, 0.0));  //! make a vector of vectors initialized to 0 (note the "stacked" vector constructor)
    vector<vector<double>> anode_L_HarmonicMatrix(8, vector<double>(8, 0.0));  //! make a vector of vectors initialized to 0 (note the "stacked" vector constructor)
    vector<vector<double>> anode_H_HarmonicMatrix(8, vector<double>(8, 0.0));  //! make a vector of vectors initialized to 0 (note the "stacked" vector constructor)
    vector<vector<double>> anode_L_positionMatrixQDC(8, vector<double>(8, 0.0));  //! make a vector of vectors initialized to 0 (note the "stacked" vector constructor)
    vector<vector<double>> anode_H_positionMatrixQDC(8, vector<double>(8, 0.0));  //! make a vector of vectors initialized to 0 (note the "stacked" vector constructor)
    
    double dyh_max=0;
    double dyh_qdc_max=0;
    double dyl_max =0;
    double dyl_qdc_max =0;
    
    pair<int,int> sipmPixels = {-99,-99};
    
    if (!Dynode_H.empty()){
        dyh_max = event.GetSummary("mtasimplantsipm:dyn_h")->GetMaxEvent()->GetCalibratedEnergy() ;
        dyh_qdc_max = (event.GetSummary("mtasimplantsipm:dyn_h")->GetMaxEvent()->GetTrace().GetQdc());
        EventData MTASBetaEvent(event.GetSummary("mtasimplantsipm:dyn_h")->GetMaxEvent()->GetTimeSansCfd(),dyh_max,-1,true,"MTASImplantBeta");
        TreeCorrelator::get()->place("MTASBeta")->activate(MTASBetaEvent);
    }
     if (!Dynode_L.empty()){
        dyl_max = event.GetSummary("mtasimplantsipm:dyn_l")->GetMaxEvent()->GetCalibratedEnergy() ;
        dyl_qdc_max = (event.GetSummary("mtasimplantsipm:dyn_l")->GetMaxEvent()->GetTrace().GetQdc());
        EventData MTASIonEvent(event.GetSummary("mtasimplantsipm:dyn_l")->GetMaxEvent()->GetTimeSansCfd(),dyh_max,-1,true,"MTASImplantIon");
        TreeCorrelator::get()->place("MTASIon")->activate(MTASIonEvent);
     }
    //!#########################################
    //!       DYNODE LOW GAIN
    //!#########################################
    
    for (auto &itDyL : Dynode_L) {
        if (itDyL->IsSaturated() || itDyL->IsPileup()) {
            continue;
        }
        double energy = itDyL->GetCalibratedEnergy();
        int detLoc = itDyL->GetChanID().GetLocation();

        //! Get the qdc from the recorded trace if it exists and has been analyized.
        double tqdc = -999;
        if (itDyL->GetTrace().HasValidWaveformAnalysis()) {
            tqdc = itDyL->GetTrace().GetQdc();
        }
        
        //! calculate the qdc from the onboard sums. returns -999 if no qdcsums are in the data stream.
        double oqdc = CalOnboardQDC(0, 2, itDyL->GetQdc());

        //! Fill the root struct
        if (DetectorDriver::get()->GetSysRootOutput()) {
            FillRootStruct(itDyL, oqdc);
        }

        plot(DD_DY_L_ENERGY, energy / EandQDC_down_scaling_, detLoc);
        if (oqdc != -999) {
            plot(DD_DY_L_OQDC, oqdc / EandQDC_down_scaling_, detLoc);
        }
        if (tqdc != -999) {
            plot(DD_DY_L_TQDC, tqdc / EandQDC_down_scaling_, detLoc);
        }
    }

    //!#########################################
    //!       DYNODE HIGH GAIN
    //!#########################################

    for (auto &itDyH : Dynode_H) {
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

        //! Fill the root struct
        if (DetectorDriver::get()->GetSysRootOutput()) {
            FillRootStruct(itDyH, oqdc);
        }

        plot(DD_DY_H_ENERGY, energy / EandQDC_down_scaling_, detLoc );
        if (oqdc != -999) {
            plot(DD_DY_H_OQDC, oqdc / EandQDC_down_scaling_, detLoc );
        }
        if (tqdc != -999) {
            plot(DD_DY_H_TQDC, tqdc / EandQDC_down_scaling_, detLoc );
        }
    }

    //!#########################################
    //!       ANODE LOW GAIN
    //!#########################################

    for (auto &itAl : Anode_L) {
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

        //! Fill the root struct
        if (DetectorDriver::get()->GetSysRootOutput()) {
            FillRootStruct(itAl, oqdc, sipmPixels);
        }

        plot(DD_HG_DY_LG_ANODE,energy,dyh_max);
        plot(DD_LG_DY_LG_ANODE,energy,dyl_max);
        if (energy > yso_thresh) {
            anodeL_energyList_for_calculations.at(detLoc) += energy;
            (anode_L_positionMatrix.at(sipmPixels.first)).at(sipmPixels.second) += energy;
        }
	    if (oqdc > oqdc_yso_thresh) {
            (anode_L_positionMatrixQDC.at(sipmPixels.first)).at(sipmPixels.second) += oqdc;
        }

        plot(DD_ANODES_L_ENERGY, energy / EandQDC_down_scaling_, detLoc);
        if (oqdc != -999) {
            plot(DD_ANODES_L_OQDC, oqdc / EandQDC_down_scaling_, detLoc);
        }
        if (tqdc != -999) {
            plot(DD_ANODES_L_TQDC, tqdc / EandQDC_down_scaling_, detLoc);
        }

    }

    pair<double, double> LG_positions = CalculatePosition(anode_L_positionMatrix,sipmPixels);
    plot(DD_SIPM_PIXEL_IMAGE_LG, sipmPixels.first + dammSiPm_pixelShifts.first, dammSiPm_pixelShifts.second - sipmPixels.second);  // x+2 and 12-y should center the image in a S4 by S4 histo
    pair<double, double> LG_QDCpositions = CalculatePosition(anode_L_positionMatrixQDC,sipmPixels);

    if (dyl_max >= dyl_thresh ){
        plot(DD_SIPM_HIRES_IMAGE_LG_THRESH,LG_positions.first * yso_scale + yso_offset/2, LG_positions.second * yso_scale + yso_offset);
        plot(DD_SIPM_HIRES_IMAGE_LG_QDC_THRESH, LG_QDCpositions.first * yso_scale + yso_offset/2, LG_QDCpositions.second * yso_scale + yso_offset);
    }
    plot(DD_SIPM_HIRES_IMAGE_LG, LG_positions.first * yso_scale + yso_offset/2, LG_positions.second * yso_scale + yso_offset);
    plot(DD_SIPM_HIRES_IMAGE_LG_QDC, LG_QDCpositions.first * yso_scale + yso_offset/2, LG_QDCpositions.second * yso_scale + yso_offset);

    //!#########################################
    //!       ANODE HIGH GAIN
    //!#########################################

    for (auto &itAh : Anode_H) {
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

        // ! Fill the root struct
        if (DetectorDriver::get()->GetSysRootOutput()) {
            FillRootStruct(itAh, oqdc, sipmPixels);
        }

        plot(DD_HG_DY_HG_ANODE,energy,dyh_max);
        plot(DD_LG_DY_HG_ANODE,energy,dyl_max);
        if (energy > yso_thresh) {
            anodeH_energyList_for_calculations.at(detLoc) += energy;
            (anode_H_positionMatrix.at(sipmPixels.first)).at(sipmPixels.second) += energy;
        }
	    if (oqdc > oqdc_yso_thresh) {
            (anode_H_positionMatrixQDC.at(sipmPixels.first)).at(sipmPixels.second) += oqdc;
        }

        plot(DD_ANODES_H_ENERGY, energy / EandQDC_down_scaling_, detLoc);
        if (oqdc != -999) {
            plot(DD_ANODES_H_OQDC, oqdc / EandQDC_down_scaling_, detLoc);
        }
        if (tqdc != -999) {
            plot(DD_ANODES_H_TQDC, tqdc / EandQDC_down_scaling_, detLoc);
        }

    }

    pair<double, double> HG_positions = CalculatePosition(anode_H_positionMatrix,sipmPixels);
    pair<double, double> HG_QDCpositions = CalculatePosition(anode_H_positionMatrixQDC,sipmPixels);
    plot(DD_SIPM_PIXEL_IMAGE_HG, sipmPixels.first + dammSiPm_pixelShifts.first, dammSiPm_pixelShifts.second - sipmPixels.second);  // x+2 and 12-y should center the image in a S4 by S4 histo

    if (dyh_max <= dyh_upperThresh && dyh_max >= dyh_thresh){
        plot(DD_SIPM_HIRES_IMAGE_HG_THRESH, HG_positions.first * yso_scale + yso_offset/2, HG_positions.second * yso_scale + yso_offset);
        plot(DD_SIPM_HIRES_IMAGE_HG_QDC_THRESH, HG_QDCpositions.first * yso_scale + yso_offset/2, HG_QDCpositions.second * yso_scale + yso_offset);
    }


    plot(DD_SIPM_HIRES_IMAGE_HG, HG_positions.first * yso_scale + yso_offset/2, HG_positions.second * yso_scale + yso_offset);
    plot(DD_SIPM_HIRES_IMAGE_HG_QDC, HG_QDCpositions.first * yso_scale + yso_offset/2, HG_QDCpositions.second * yso_scale + yso_offset);

   
    EndProcess();
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

void MtasImplantSipmProcessor::FillRootStruct(ChanEvent *evt, double &onboardqdc, const std::pair<int, int> &positions) {
    mtasImplStruct = processor_struct::MTASIMPLANT_DEFAULT_STRUCT;
    mtasImplStruct.energy = evt->GetCalibratedEnergy();
    mtasImplStruct.oqdc = onboardqdc;
    /* if (!evt->GetTrace().empty()) { */
    /*     mtasImplStruct.hastraceInfile = true; */
    /*     mtasImplStruct.trace = evt->GetTrace(); */
    /*     if (evt->GetTrace().HasValidWaveformAnalysis()) { */
    /*         mtasImplStruct.tqdc = evt->GetTrace().GetQdc(); */
    /*         mtasImplStruct.hasValidWaveform = true; */
    /*     } */
    /* } */
    mtasImplStruct.timesans = evt->GetTimeSansCfd() * Globals::get()->GetClockInSeconds(evt->GetChanID().GetModFreq());
    mtasImplStruct.sipmloc = evt->GetChanID().GetLocation();
    mtasImplStruct.xpixel= positions.first;
    mtasImplStruct.ypixel = positions.second;
    mtasImplStruct.subtype = evt->GetChanID().GetSubtype();
    mtasImplStruct.group = evt->GetChanID().GetGroup();
    pixie_tree_event_->mtasimpl_vec_.emplace_back(mtasImplStruct);
}

pair<double, double> MtasImplantSipmProcessor::CalculatePosition(const std::vector<std::vector<double>> &data,std::pair<int,int>& pixel) const{
    // x = energy(1,1)*1 + energy(1,2) * 2 ... + energy (2,1)*1 + energy (2,2) *2 ../sumE
    // y = energy (1,1)*1 + energy(2,1)*2 .. + energy (1,2) * 1 + energy (2,2) *2 ../sumE
    double x_tmp_ = 0;
    double y_tmp_ = 0;
    double energy_sum = 0;
    double erg_max = 0.0;

    //TODO: REWRITE INTO USING FOR_EACH TYPE LOOPS
    for (unsigned int iter = 0; iter < data.size(); ++iter) {
        for (unsigned int iter2 = 0; iter2 < data.at(iter).size(); ++iter2) {
            double erg =  (data.at(iter)).at(iter2);
            if( erg > erg_max ){
		    erg_max = erg;
		    pixel.first = iter+1;
		    pixel.second = iter2+1;
	    }
            energy_sum += erg;
            x_tmp_ += (data.at(iter)).at(iter2) * (iter + 1);
            y_tmp_ += (data.at(iter)).at(iter2) * (iter2 + 1);
            /* x_tmp_ += (data.at(iter)).at(iter2) * (iter2 + 1); */
            /* y_tmp_ += (data.at(iter)).at(iter2) * (iter + 1); */
        }
    }
    return make_pair(x_tmp_ / energy_sum, (8 - y_tmp_)  / energy_sum );
}
