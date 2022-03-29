/** @file MtasProcessor.cpp
* @brief  Basic MtasProcessor for MTAS at FRIB
* @authors T.T. King, T. Ruland, B.C. Rasco
* @date 03/25/2022
*/

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "DetectorDriver.hpp"
#include "DetectorLibrary.hpp"
#include "HelperFunctions.hpp"
#include "StringManipulationFunctions.hpp"
#include "RawEvent.hpp"
#include "MtasProcessor.hpp"
#include "DammPlotIds.hpp"

namespace dammIds {
    namespace mtas {
        const unsigned TOTAL_OFFSET = 0;
        const unsigned CENTER_OFFSET = 10;
        const unsigned INNER_OFFSET = 20;
        const unsigned MIDDLE_OFFSET = 30;
        const unsigned OUTER_OFFSET = 40;
        const unsigned DD_OFFSET = 50;
        
        const unsigned INDIVIDUALS_OFFSET = 5; 
        const unsigned BETA_GATED_OFFSET = 100; 
        const unsigned D_MTAS_SUM_FB = 200;

        const unsigned D_MTAS_TOTAL = TOTAL_OFFSET;
        const unsigned D_MTAS_CENTER = CENTER_OFFSET;
        const unsigned D_MTAS_INNER = INNER_OFFSET;
        const unsigned D_MTAS_MIDDLE = MIDDLE_OFFSET;
        const unsigned D_MTAS_OUTER = OUTER_OFFSET;

        const unsigned D_MTAS_TOTAL_INDI = TOTAL_OFFSET + INDIVIDUALS_OFFSET;
        const unsigned D_MTAS_CENTER_INDI = CENTER_OFFSET + INDIVIDUALS_OFFSET;
        const unsigned D_MTAS_INNER_INDI = INNER_OFFSET + INDIVIDUALS_OFFSET;
        const unsigned D_MTAS_MIDDLE_INDI = MIDDLE_OFFSET + INDIVIDUALS_OFFSET;
        const unsigned D_MTAS_OUTER_INDI = OUTER_OFFSET + INDIVIDUALS_OFFSET;

        const unsigned DD_MTAS_CS_T = DD_OFFSET; //! MTAS  Center Segments vs Total Sum
        const unsigned DD_MTAS_CR_T = DD_OFFSET + 1; //! MTAS Center Ring Sum vs Total Sum
        const unsigned DD_MTAS_CS_CR = DD_OFFSET + 2; //! MTAS Center Segments vs Center Ring Sum

        
    }
}
using namespace std;
using namespace dammIds::mtas;

void MtasProcessor::DeclarePlots(void){
    DeclareHistogram1D(D_MTAS_TOTAL,SE, "Mtas TOTAL");
    DeclareHistogram1D(D_MTAS_CENTER,SE, "Mtas CENTER");
    DeclareHistogram1D(D_MTAS_INNER,SE, "Mtas INNER");
    DeclareHistogram1D(D_MTAS_MIDDLE,SE, "Mtas MIDDLE");
    DeclareHistogram1D(D_MTAS_OUTER,SE, "Mtas OUTER");

    DeclareHistogram1D(D_MTAS_TOTAL_INDI,SE, "Mtas TOTAL Individuals");
    DeclareHistogram1D(D_MTAS_CENTER_INDI,SE, "Mtas CENTER Individuals");
    DeclareHistogram1D(D_MTAS_INNER_INDI,SE, "Mtas INNER Individuals");
    DeclareHistogram1D(D_MTAS_MIDDLE_INDI,SE, "Mtas MIDDLE Individuals");
    DeclareHistogram1D(D_MTAS_OUTER_INDI,SE, "Mtas OUTER Individuals");

    for (unsigned i= 0; i < 6; ++i){
        std::string title = "Mtas Center " + to_string(i) + " Sum F+B";
        DeclareHistogram1D(D_MTAS_SUM_FB + CENTER_OFFSET + i , SE, title.c_str());
    }
     for (unsigned i= 0; i < 6; ++i){
        std::string title = "Mtas Inner " + to_string(i) + " Sum F+B";
        DeclareHistogram1D(D_MTAS_SUM_FB + INNER_OFFSET + i , SE, title.c_str());
    }
     for (unsigned i= 0; i < 6; ++i){
        std::string title = "Mtas Middle " + to_string(i) + " Sum F+B";
        DeclareHistogram1D(D_MTAS_SUM_FB + MIDDLE_OFFSET + i , SE, title.c_str());
    }
     for (unsigned i= 0; i < 6; ++i){
        std::string title = "Mtas Outer " + to_string(i) + " Sum F+B";
        DeclareHistogram1D(D_MTAS_SUM_FB + OUTER_OFFSET + i , SE, title.c_str());
    }

    DeclareHistogram2D(DD_MTAS_CR_T,SE,SE,"Mtas Center Ring vs Total");
    DeclareHistogram2D(DD_MTAS_CS_CR,SE,SE,"Mtas Center Segment vs Center Ring");
    DeclareHistogram2D(DD_MTAS_CS_T,SE,SE,"Mtas Center Segment vs Total");

}


MtasProcessor::MtasProcessor() : EventProcessor(OFFSET, RANGE, "MtasProcessor") {
    associatedTypes.insert("mtas");
}

bool MtasProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    static const auto &chanEvents = event.GetSummary("mtas", true)->GetList();

    vector<MtasSegment> MtasSegVec(24, MtasSegment());
    vector<short> MtasSegMulti(48,0); // MTAS segment multiplicity "map"

    for (auto chanEvtIter = chanEvents.begin(); chanEvtIter != chanEvents.end(); ++chanEvtIter){
        
        //! needs try/catch for non numeric string in group
        int segmentNum = stoi((*chanEvtIter)->GetChanID().GetGroup().c_str());
        string Ring = StringManipulation::StringLower((*chanEvtIter)->GetChanID().GetSubtype());
        int RingOffset;
        if (strcmp(Ring.c_str(),"center") == 0 ){
            RingOffset = -1;
        } else if (strcmp(Ring.c_str(),"inner") == 0 ){
            RingOffset = 5;
        } else if (strcmp(Ring.c_str(),"middle") == 0 ){
            RingOffset = 11;
        } else if (strcmp(Ring.c_str(),"outer") == 0 ){
            RingOffset = 17;
        } else {
            RingOffset = -9999;
        }
        
        bool isFront = (*chanEvtIter)->GetChanID().HasTag("front");
        bool isBack = (*chanEvtIter)->GetChanID().HasTag("back");
        int chanOffset;
        if (isFront) {
            chanOffset = 0;
        } else if (isBack) {
            chanOffset = 1;
        } else {
            chanOffset = -9999;
        }
        if (chanOffset == -9999 || RingOffset == -9999){
            cout<<"ERROR::MtasProcessor:PreProcess Channel ("<<(*chanEvtIter)->GetModuleNumber()<<" , " << (*chanEvtIter)->GetChannelNumber() << ") found which doesnt have a front or back tag, or you didnt set the Ring right (xml subtype). This means the XML is not right so you need to fix it!!"<<endl;
            return false;
        }

        int GlobalMtasSegID = RingOffset + segmentNum;
        int GlobalMtasChanID = (segmentNum + RingOffset) * 2 + chanOffset;

        if((*chanEvtIter)->GetCalibratedEnergy() > 30000 || (*chanEvtIter)->IsSaturated() || (*chanEvtIter)->IsPileup()){
            continue;
        } else {
            
            MtasSegMulti.at(GlobalMtasChanID)++; // increment the multipliciy "map" based on GlobalMtasSegID

            MtasSegVec.at(GlobalMtasSegID).gMtasSegID_ = GlobalMtasSegID;
            if(isFront && MtasSegVec.at(GlobalMtasSegID).segFront_ == nullptr){  
                MtasSegVec.at(GlobalMtasSegID).segFront_ = (*chanEvtIter);
            } else if (isBack && MtasSegVec.at(GlobalMtasSegID).segFront_ == nullptr) {
                MtasSegVec.at(GlobalMtasSegID).segBack_ = (*chanEvtIter);
            }
        }
    }  //! end loop over chanEvents.

    //! begin loop over segments for sums
    double centerSum = 0;
    double totalSum = 0;
    for (auto segIter = MtasSegVec.begin(); segIter != MtasSegVec.end(); ++segIter) {
        int segmentID = segIter->gMtasSegID_;
        if (segmentID >= 0 && segmentID <= 5) {
            if (segIter->segFront_ != nullptr && segIter->segBack_ != nullptr) {
                double segmentAvg = (segIter->segFront_->GetCalibratedEnergy() + segIter->segBack_->GetCalibratedEnergy()) / 2.0;
                totalSum += segmentAvg;
                centerSum += segmentAvg; 
            }
        }
    }
        //! loop over segments for ploting
    for (auto segIter = MtasSegVec.begin(); segIter != MtasSegVec.end(); ++segIter) {
        int segmentID = segIter->gMtasSegID_;
        if (segmentID >= 0 && segmentID <= 5) {
            if (segIter->segFront_ != nullptr && segIter->segBack_ != nullptr) {
                double segmentAvg = (segIter->segFront_->GetCalibratedEnergy() + segIter->segBack_->GetCalibratedEnergy()) / 2.0;

                //! per segment plots
                plot(D_MTAS_SUM_FB + CENTER_OFFSET + segmentID, segmentAvg);
                plot(D_MTAS_CENTER_INDI, segmentAvg);
                plot(DD_MTAS_CS_T, totalSum, segmentAvg);
                plot(DD_MTAS_CS_CR,centerSum,segmentAvg);
            }
        }
    }

    //! sum spectra
    plot(D_MTAS_CENTER, centerSum);
    plot(DD_MTAS_CR_T, totalSum, centerSum);

    return true;
}

bool MtasProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    //! This is ROOT output stuff
    // for (auto it = Events.begin(); it != Events.end(); it++) {
    //     Mtasstruct.energy = (*it)->GetCalibratedEnergy();
    //     Mtasstruct.rawEnergy = (*it)->GetEnergy();
    //     if (Rev == "F") {
    //         Mtasstruct.timeSansCfd = (*it)->GetTimeSansCfd() * Globals::get()->GetClockInSeconds((*it)->GetChanID().GetModFreq()) * 1e9;
    //         Mtasstruct.time = (*it)->GetTime() * Globals::get()->GetAdcClockInSeconds((*it)->GetChanID().GetModFreq()) * 1e9;
    //     } else {
    //         Mtasstruct.timeSansCfd = (*it)->GetTimeSansCfd() * Globals::get()->GetClockInSeconds() * 1e9;
    //         Mtasstruct.time = (*it)->GetTime() * Globals::get()->GetAdcClockInSeconds() * 1e9;
    //     }
    //     Mtasstruct.detNum = (*it)->GetChanID().GetLocation();
    //     Mtasstruct.modNum = (*it)->GetModuleNumber();
    //     Mtasstruct.chanNum = (*it)->GetChannelNumber();
    //     Mtasstruct.subtype = (*it)->GetChanID().GetSubtype();
    //     Mtasstruct.group = (*it)->GetChanID().GetGroup();
    //     Mtasstruct.pileup = (*it)->IsPileup();
    //     Mtasstruct.saturation = (*it)->IsSaturated();

    //     pixie_tree_event_->mtas_vec_.emplace_back(Mtasstruct);
    //     Mtasstruct = processor_struct::Mtas_DEFAULT_STRUCT;
    // }

    //!---- END root output stuff

    EndProcess();
    return true;
}
