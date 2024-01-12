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
#include "Globals.hpp"
#include "HelperFunctions.hpp"
#include "StringManipulationFunctions.hpp"
#include "RawEvent.hpp"
#include "MtasProcessor.hpp"
#include "DammPlotIds.hpp"

namespace dammIds {
	namespace mtas {
		//7200 is beginning
		const unsigned TOTAL_OFFSET = 0;
		const unsigned CENTER_OFFSET = 10;
		const unsigned INNER_OFFSET = 20;
		const unsigned MIDDLE_OFFSET = 30;
		const unsigned OUTER_OFFSET = 40;
		const unsigned DD_OFFSET = 50;
		const unsigned DD_LIGHT_SHARE_OFFSET = 60;
		const unsigned D_SINGLE_CENTER_OFFSET = 70;

		const unsigned INDIVIDUALS_OFFSET = 5; 
		const unsigned BETA_GATED_OFFSET = 100; 
		const unsigned D_MTAS_SUM_FB = 200;
		const unsigned D_MTAS_TDIFF_OFFSET = 300;
		const unsigned D_ONE_OFFS = 400; 
		const unsigned D_CONICIDENCE = 500;

		const unsigned D_MTAS_TOTAL = TOTAL_OFFSET;
		const unsigned D_MTAS_CENTER = CENTER_OFFSET;
		const unsigned D_MTAS_INNER = INNER_OFFSET;
		const unsigned D_MTAS_MIDDLE = MIDDLE_OFFSET;
		const unsigned D_MTAS_OUTER = OUTER_OFFSET;

		const unsigned D_MTAS_TOTAL_BETA = TOTAL_OFFSET + BETA_GATED_OFFSET;
		const unsigned D_MTAS_CENTER_BETA = CENTER_OFFSET + BETA_GATED_OFFSET;
		const unsigned D_MTAS_INNER_BETA = INNER_OFFSET + BETA_GATED_OFFSET;
		const unsigned D_MTAS_MIDDLE_BETA = MIDDLE_OFFSET + BETA_GATED_OFFSET;
		const unsigned D_MTAS_OUTER_BETA = OUTER_OFFSET + BETA_GATED_OFFSET;
		
		const unsigned D_MTAS_TOTAL_ION = TOTAL_OFFSET + BETA_GATED_OFFSET + 1;
		const unsigned D_MTAS_CENTER_ION = CENTER_OFFSET + BETA_GATED_OFFSET + 1;
		const unsigned D_MTAS_INNER_ION = INNER_OFFSET + BETA_GATED_OFFSET + 1;
		const unsigned D_MTAS_MIDDLE_ION = MIDDLE_OFFSET + BETA_GATED_OFFSET + 1;
		const unsigned D_MTAS_OUTER_ION = OUTER_OFFSET + BETA_GATED_OFFSET + 1;

		const unsigned D_MTAS_TOTAL_INDI = TOTAL_OFFSET + INDIVIDUALS_OFFSET;
		const unsigned D_MTAS_CENTER_INDI = CENTER_OFFSET + INDIVIDUALS_OFFSET;
		const unsigned D_MTAS_INNER_INDI = INNER_OFFSET + INDIVIDUALS_OFFSET;
		const unsigned D_MTAS_MIDDLE_INDI = MIDDLE_OFFSET + INDIVIDUALS_OFFSET;
		const unsigned D_MTAS_OUTER_INDI = OUTER_OFFSET + INDIVIDUALS_OFFSET;

		const unsigned DD_MTAS_CS_T = DD_OFFSET; //! MTAS  Center Segments vs Total Sum
		const unsigned DD_MTAS_CR_T = DD_OFFSET + 1; //! MTAS Center Ring Sum vs Total Sum
		const unsigned DD_MTAS_CS_CR = DD_OFFSET + 2; //! MTAS Center Segments vs Center Ring Sum
		const unsigned DD_MTAS_IMO_T = DD_OFFSET + 3;  //! MTAS I,M,O Segments vs Total

		const unsigned DD_MTAS_CS_T_BETA =  BETA_GATED_OFFSET + DD_OFFSET; //! MTAS  Center Segments vs Total Sum
		const unsigned DD_MTAS_CR_T_BETA =  BETA_GATED_OFFSET + DD_OFFSET + 1; //! MTAS Center Ring Sum vs Total Sum
		const unsigned DD_MTAS_CS_CR_BETA = BETA_GATED_OFFSET + DD_OFFSET + 2; //! MTAS Center Segments vs Center Ring Sum
		const unsigned DD_MTAS_IMO_T_BETA = BETA_GATED_OFFSET + DD_OFFSET + 3;  //! MTAS I,M,O Segments vs Total

		const unsigned DD_MTAS_CS_T_ION =  BETA_GATED_OFFSET + DD_OFFSET + 5; //! MTAS  Center Segments vs Total Sum
		const unsigned DD_MTAS_CR_T_ION =  BETA_GATED_OFFSET + DD_OFFSET + 6; //! MTAS Center Ring Sum vs Total Sum
		const unsigned DD_MTAS_CS_CR_ION = BETA_GATED_OFFSET + DD_OFFSET + 7; //! MTAS Center Segments vs Center Ring Sum
		const unsigned DD_MTAS_IMO_T_ION = BETA_GATED_OFFSET + DD_OFFSET + 8;  //! MTAS I,M,O Segments vs Total

		const unsigned DD_MTAS_BETA_T_BETA = BETA_GATED_OFFSET + DD_OFFSET + 10;
		const unsigned DD_MTAS_BETA_CS_BETA = BETA_GATED_OFFSET + DD_OFFSET + 11;
		const unsigned DD_MTAS_BETA_IS_BETA = BETA_GATED_OFFSET + DD_OFFSET + 12;
		const unsigned DD_MTAS_BETA_MS_BETA = BETA_GATED_OFFSET + DD_OFFSET + 13;
		const unsigned DD_MTAS_BETA_OS_BETA = BETA_GATED_OFFSET + DD_OFFSET + 14;
		
		const unsigned DD_MTAS_BETA_CR_BETA = BETA_GATED_OFFSET + DD_OFFSET + 15;
		const unsigned DD_MTAS_BETA_IR_BETA = BETA_GATED_OFFSET + DD_OFFSET + 16;
		const unsigned DD_MTAS_BETA_MR_BETA = BETA_GATED_OFFSET + DD_OFFSET + 17;
		const unsigned DD_MTAS_BETA_OR_BETA = BETA_GATED_OFFSET + DD_OFFSET + 18;

		//isomer plot
		const unsigned DD_MTAS_BETA_ISOMER_T = BETA_GATED_OFFSET + DD_OFFSET + 20;

		const unsigned DD_MTAS_BETA_T_BETA_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 21;
		const unsigned DD_MTAS_BETA_CS_BETA_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 22;
		const unsigned DD_MTAS_BETA_IS_BETA_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 23;
		const unsigned DD_MTAS_BETA_MS_BETA_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 24;
		const unsigned DD_MTAS_BETA_OS_BETA_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 25;
		
		const unsigned DD_MTAS_BETA_CR_BETA_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 26;
		const unsigned DD_MTAS_BETA_IR_BETA_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 27;
		const unsigned DD_MTAS_BETA_MR_BETA_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 28;
		const unsigned DD_MTAS_BETA_OR_BETA_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 29;

		const unsigned DD_MTAS_ION_ISOMER_T = BETA_GATED_OFFSET + DD_OFFSET + 30;

		const unsigned DD_MTAS_BETA_T_ION_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 31;
		const unsigned DD_MTAS_BETA_CS_ION_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 32;
		const unsigned DD_MTAS_BETA_IS_ION_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 33;
		const unsigned DD_MTAS_BETA_MS_ION_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 34;
		const unsigned DD_MTAS_BETA_OS_ION_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 35;
		
		const unsigned DD_MTAS_BETA_CR_ION_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 36;
		const unsigned DD_MTAS_BETA_IR_ION_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 37;
		const unsigned DD_MTAS_BETA_MR_ION_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 38;
		const unsigned DD_MTAS_BETA_OR_ION_ISOMER = BETA_GATED_OFFSET + DD_OFFSET + 39;
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

	DeclareHistogram1D(D_MTAS_TOTAL_BETA,SE, "Mtas TOTAL Beta-Gated");
	DeclareHistogram1D(D_MTAS_CENTER_BETA,SE, "Mtas CENTER Beta-Gated");
	DeclareHistogram1D(D_MTAS_INNER_BETA,SE, "Mtas INNER Beta-Gated");
	DeclareHistogram1D(D_MTAS_MIDDLE_BETA,SE, "Mtas MIDDLE Beta-Gated");
	DeclareHistogram1D(D_MTAS_OUTER_BETA,SE, "Mtas OUTER Beta-Gated");

	DeclareHistogram1D(D_MTAS_TOTAL_ION,SE, "Mtas TOTAL Ion-Gated");
	DeclareHistogram1D(D_MTAS_CENTER_ION,SE, "Mtas CENTER Ion-Gated");
	DeclareHistogram1D(D_MTAS_INNER_ION,SE, "Mtas INNER Ion-Gated");
	DeclareHistogram1D(D_MTAS_MIDDLE_ION,SE, "Mtas MIDDLE Ion-Gated");
	DeclareHistogram1D(D_MTAS_OUTER_ION,SE, "Mtas OUTER Ion-Gated");

	DeclareHistogram1D(D_MTAS_TOTAL_INDI,SE, "Mtas TOTAL Individuals");
	DeclareHistogram1D(D_MTAS_CENTER_INDI,SE, "Mtas CENTER Individuals");
	DeclareHistogram1D(D_MTAS_INNER_INDI,SE, "Mtas INNER Individuals");
	DeclareHistogram1D(D_MTAS_MIDDLE_INDI,SE, "Mtas MIDDLE Individuals");
	DeclareHistogram1D(D_MTAS_OUTER_INDI,SE, "Mtas OUTER Individuals");

	for (unsigned i= 0; i < 6; ++i){ //! to_string(i+1) to account for 0 counting
		std::string title = "Mtas Center " + to_string(i+1) + " Sum F+B";
		DeclareHistogram1D(D_MTAS_SUM_FB + CENTER_OFFSET + i , SE, title.c_str());
		std::string title2 = "Mtas Center " + to_string(i+1) + " FB-TDIFF";
		DeclareHistogram1D(D_MTAS_TDIFF_OFFSET + CENTER_OFFSET + i , SE, title2.c_str());
		std::string title3 = "Mtas Center " + to_string(i+1) + " front when both";
		std::string title4 = "Mtas Center " + to_string(i+1) + " back when both";
		DeclareHistogram1D(D_CONICIDENCE + (2*i), SE, title3.c_str());
		DeclareHistogram1D(D_CONICIDENCE + (2*i + 1), SE, title4.c_str());
		std::string title5 = "Mtas Center " + to_string(i+1) + " single segment fire";
		DeclareHistogram1D(D_SINGLE_CENTER_OFFSET + i,SE,title5.c_str());
		std::string title6 = "Mtas Center " + to_string(i+1) + " no neighbors fire";
		DeclareHistogram1D(D_SINGLE_CENTER_OFFSET + i + 6,SE,title6.c_str());
	}
	for (unsigned i= 0; i < 6; ++i){
		std::string title = "Mtas Inner " + to_string(i+1) + " Sum F+B";
		DeclareHistogram1D(D_MTAS_SUM_FB + INNER_OFFSET + i , SE, title.c_str());
		std::string title2 = "Mtas Inner " + to_string(i+1) + " FB-TDIFF";
		DeclareHistogram1D(D_MTAS_TDIFF_OFFSET + INNER_OFFSET + i , SE, title2.c_str());
		std::string title3 = "Mtas Inner " + to_string(i+1) + " front when both";
		std::string title4 = "Mtas Inner " + to_string(i+1) + " back when both";
		DeclareHistogram1D(D_CONICIDENCE + 12 + (2*i), SE, title3.c_str());
		DeclareHistogram1D(D_CONICIDENCE + 12 + (2*i + 1) , SE, title4.c_str());
	}
	for (unsigned i= 0; i < 6; ++i){
		std::string title = "Mtas Middle " + to_string(i+1) + " Sum F+B";
		DeclareHistogram1D(D_MTAS_SUM_FB + MIDDLE_OFFSET + i , SE, title.c_str());
		std::string title2 = "Mtas Middle " + to_string(i+1) + " FB-TDIFF";
		DeclareHistogram1D(D_MTAS_TDIFF_OFFSET + MIDDLE_OFFSET + i , SE, title2.c_str());
		std::string title3 = "Mtas Middle " + to_string(i+1) + " front when both";
		std::string title4 = "Mtas Middle " + to_string(i+1) + " back when both";
		DeclareHistogram1D(D_CONICIDENCE + 24 + (2*i), SE, title3.c_str());
		DeclareHistogram1D(D_CONICIDENCE + 24 + (2*i + 1), SE, title4.c_str());
	}
	for (unsigned i= 0; i < 6; ++i){
		std::string title = "Mtas Outer " + to_string(i+1) + " Sum F+B";
		DeclareHistogram1D(D_MTAS_SUM_FB + OUTER_OFFSET + i , SE, title.c_str());
		std::string title2 = "Mtas Outer " + to_string(i+1) + " FB-TDIFF";
		DeclareHistogram1D(D_MTAS_TDIFF_OFFSET + OUTER_OFFSET + i , SE, title2.c_str());
		std::string title3 = "Mtas Outer " + to_string(i+1) + " front when both";
		std::string title4 = "Mtas Outer " + to_string(i+1) + " back when both";
		DeclareHistogram1D(D_CONICIDENCE + 36+ (2*i), SE, title3.c_str());
		DeclareHistogram1D(D_CONICIDENCE + 36+ (2*i + 1), SE, title4.c_str());
	}

	DeclareHistogram2D(DD_MTAS_CR_T,SA,SA,"Mtas Center Ring vs Total");
	DeclareHistogram2D(DD_MTAS_CS_CR,SA,SA,"Mtas Center Segment vs Center Ring");
	DeclareHistogram2D(DD_MTAS_CS_T,SA,SA,"Mtas Center Segment vs Total");
	DeclareHistogram2D(DD_MTAS_IMO_T,SA,SA,"Mtas I,M,O Segment vs Total");

	DeclareHistogram2D(DD_MTAS_CR_T_BETA,SA,SA,"Mtas Center Ring vs Total Beta-Gated");
	DeclareHistogram2D(DD_MTAS_CS_CR_BETA,SA,SA,"Mtas Center Segment vs Center Ring Beta-Gated");
	DeclareHistogram2D(DD_MTAS_CS_T_BETA,SA,SA,"Mtas Center Segment vs Total Beta-Gated");
	DeclareHistogram2D(DD_MTAS_IMO_T_BETA,SA,SA,"Mtas I,M,O Segment vs Total Beta-Gated");

	DeclareHistogram2D(DD_MTAS_CR_T_ION,SA,SA,"Mtas Center Ring vs Total Ion-Gated");
	DeclareHistogram2D(DD_MTAS_CS_CR_ION,SA,SA,"Mtas Center Segment vs Center Ring Ion-Gated");
	DeclareHistogram2D(DD_MTAS_CS_T_ION,SA,SA,"Mtas Center Segment vs Total Ion-Gated");
	DeclareHistogram2D(DD_MTAS_IMO_T_ION,SA,SA,"Mtas I,M,O Segment vs Total Ion-Gated");

	DeclareHistogram2D(DD_MTAS_BETA_T_BETA,SA,SA,"Mtas Total vs Beta Energy");
	DeclareHistogram2D(DD_MTAS_BETA_CS_BETA,SA,SA,"Mtas Center Segment vs Beta Energy");
	DeclareHistogram2D(DD_MTAS_BETA_IS_BETA,SA,SA,"Mtas Inner Segment vs Beta Energy");
	DeclareHistogram2D(DD_MTAS_BETA_MS_BETA,SA,SA,"Mtas Middle Segment vs Beta Energy");
	DeclareHistogram2D(DD_MTAS_BETA_OS_BETA,SA,SA,"Mtas Outer Segment vs Beta Energy");

	DeclareHistogram2D(DD_MTAS_BETA_CR_BETA,SA,SA,"Mtas Center Ring vs Beta Energy");
	DeclareHistogram2D(DD_MTAS_BETA_IR_BETA,SA,SA,"Mtas Inner Ring vs Beta Energy");
	DeclareHistogram2D(DD_MTAS_BETA_MR_BETA,SA,SA,"Mtas Middle Ring vs Beta Energy");
	DeclareHistogram2D(DD_MTAS_BETA_OR_BETA,SA,SA,"Mtas Outer Ring vs Beta Energy");

	DeclareHistogram2D(DD_MTAS_BETA_ISOMER_T,SA,SD,"Mtas Total vs DeltaT Beta Isomer"); 

	DeclareHistogram2D(DD_MTAS_BETA_T_BETA_ISOMER ,SA,SA,"Mtas Total vs PrevBeta Energy");
	DeclareHistogram2D(DD_MTAS_BETA_CS_BETA_ISOMER ,SA,SA,"Mtas Center Segment vs PrevBeta Energy");
	DeclareHistogram2D(DD_MTAS_BETA_IS_BETA_ISOMER ,SA,SA,"Mtas Inner Segment vs PrevBeta Energy");
	DeclareHistogram2D(DD_MTAS_BETA_MS_BETA_ISOMER ,SA,SA,"Mtas Middle Segment vs PrevBeta Energy");
	DeclareHistogram2D(DD_MTAS_BETA_OS_BETA_ISOMER ,SA,SA,"Mtas Outer Segment vs PrevBeta Energy");

	DeclareHistogram2D(DD_MTAS_BETA_CR_BETA_ISOMER ,SA,SA,"Mtas Center Ring vs PrevBeta Energy");
	DeclareHistogram2D(DD_MTAS_BETA_IR_BETA_ISOMER ,SA,SA,"Mtas Inner Ring vs PrevBeta Energy");
	DeclareHistogram2D(DD_MTAS_BETA_MR_BETA_ISOMER ,SA,SA,"Mtas Middle Ring vs PrevBeta Energy");
	DeclareHistogram2D(DD_MTAS_BETA_OR_BETA_ISOMER ,SA,SA,"Mtas Outer Ring vs PrevBeta Energy");

	DeclareHistogram2D(DD_MTAS_ION_ISOMER_T,SA,SD,"Mtas Total vs DeltaT Ion Isomer"); 

	DeclareHistogram2D(DD_MTAS_BETA_T_ION_ISOMER ,SA,SA,"Mtas Total vs PrevIon Energy");
	DeclareHistogram2D(DD_MTAS_BETA_CS_ION_ISOMER ,SA,SA,"Mtas Center Segment vs PrevIon Energy");
	DeclareHistogram2D(DD_MTAS_BETA_IS_ION_ISOMER ,SA,SA,"Mtas Inner Segment vs PrevIon Energy");
	DeclareHistogram2D(DD_MTAS_BETA_MS_ION_ISOMER ,SA,SA,"Mtas Middle Segment vs PrevIon Energy");
	DeclareHistogram2D(DD_MTAS_BETA_OS_ION_ISOMER ,SA,SA,"Mtas Outer Segment vs PrevIon Energy");

	DeclareHistogram2D(DD_MTAS_BETA_CR_ION_ISOMER ,SA,SA,"Mtas Center Ring vs PrevIon Energy");
	DeclareHistogram2D(DD_MTAS_BETA_IR_ION_ISOMER ,SA,SA,"Mtas Inner Ring vs PrevIon Energy");
	DeclareHistogram2D(DD_MTAS_BETA_MR_ION_ISOMER ,SA,SA,"Mtas Middle Ring vs PrevIon Energy");
	DeclareHistogram2D(DD_MTAS_BETA_OR_ION_ISOMER ,SA,SA,"Mtas Outer Ring vs PrevIon Energy");

	DeclareHistogram2D(D_ONE_OFFS + 0, S6, S6, "Mtas Multiplicity x=ID y=multi");
}

MtasProcessor::MtasProcessor(bool HasBeta,double BetaMinEnergy,double BetaMaxEnergy,double IonMinEnergy,double IonMaxEnergy) : EventProcessor(OFFSET, RANGE, "MtasProcessor") {
	associatedTypes.insert("mtas");
	PixieRevision = Globals::get()->GetPixieRevision();
	BetaMin = BetaMinEnergy;
	BetaMax = BetaMaxEnergy;
	IonMin = IonMinEnergy;
	IonMax = IonMaxEnergy;
	HasBetaInfo = HasBeta;
	IsPrevBetaTriggered = false;
	if( HasBetaInfo ){
		if( !TreeCorrelator::get()->checkPlace(BETANAME) ){
			string errormsg = "MtasProcessor::Error You need to add the following lines to the xml";
			errormsg += "<TreeCorrelator name=\"root\" verbose=\"true\">";
			errormsg += "\t<Place type=\"PlaceDetector\" name=\""+BETANAME+"\" reset=\"true\" fifo=\"1\"/>";
			errormsg += "</TreeCorrelator>";
			cout << errormsg << endl;
			throw errormsg;
		}
        if( !TreeCorrelator::get()->checkPlace(IONNAME) ){
			string errormsg = "MtasProcessor::Error You need to add the following lines to the xml";
			errormsg += "<TreeCorrelator name=\"root\" verbose=\"true\">";
			errormsg += "\t<Place type=\"PlaceDetector\" name=\""+IONNAME+"\" reset=\"true\" fifo=\"1\"/>";
			errormsg += "</TreeCorrelator>";
			cout << errormsg << endl;
			throw errormsg;
		}
	}
    //this is reset at the end of Process
    MtasCenterSegVec = vector<MtasSegment>(6,MtasSegment());
    MtasInnerSegVec = vector<MtasSegment>(6,MtasSegment());
    MtasMiddleSegVec = vector<MtasSegment>(6,MtasSegment());
    MtasOuterSegVec = vector<MtasSegment>(6,MtasSegment());
    for( int ii = 0; ii < 6; ++ii ){
        MtasCenterSegVec.at(ii).segRing_ = "center";
        MtasCenterSegVec.at(ii).gMtasSegID_ = ii;
        MtasCenterSegVec.at(ii).RingSegNum_ = ii + 1;

        MtasInnerSegVec.at(ii).segRing_ = "inner";
        MtasInnerSegVec.at(ii).gMtasSegID_ = ii + 6;
        MtasInnerSegVec.at(ii).RingSegNum_ = ii + 1;

        MtasMiddleSegVec.at(ii).segRing_ = "middle";
        MtasMiddleSegVec.at(ii).gMtasSegID_ = ii + 12;
        MtasMiddleSegVec.at(ii).RingSegNum_ = ii + 1;

        MtasOuterSegVec.at(ii).segRing_ = "outer";
        MtasOuterSegVec.at(ii).gMtasSegID_ = ii + 18; 
        MtasOuterSegVec.at(ii).RingSegNum_ = ii + 1;

    }
}

bool MtasProcessor::PreProcess(RawEvent &event) {
	if (!EventProcessor::PreProcess(event))
		return false;
    //start_time = std::chrono::high_resolution_clock::now();

	const auto &chanEvents = event.GetSummary("mtas", true)->GetList();

	vector<short> MtasSegMulti(48,0); // MTAS segment multiplicity "map"

    for (auto chanEvtIter = chanEvents.begin(); chanEvtIter != chanEvents.end(); ++chanEvtIter){

        if( (*chanEvtIter)->IsSaturated() || (*chanEvtIter)->IsPileup()){
            continue;
        } else {
            //! needs try/catch for non numeric string in group
            int segmentNum = stoi((*chanEvtIter)->GetChanID().GetGroup().c_str());
            int vecindex = segmentNum - 1;
            string Ring = StringManipulation::StringLower((*chanEvtIter)->GetChanID().GetSubtype());
            int RingOffset = -9999;
            bool isFront = (*chanEvtIter)->GetChanID().HasTag("front");
            bool isBack = (*chanEvtIter)->GetChanID().HasTag("back");

            if (strcmp(Ring.c_str(), "center") == 0) {
                RingOffset = -1;
            }
            if (strcmp(Ring.c_str(), "inner") == 0) {
                RingOffset = 5;
            }
            if (strcmp(Ring.c_str(), "middle") == 0) {
                RingOffset = 11;
            }
            if (strcmp(Ring.c_str(), "outer") == 0) {
                RingOffset = 17;
            }
            int chanOffset;
            if (isFront) {
                chanOffset = 0;
            } else if (isBack) {
                chanOffset = 1;
            } else if ((isFront && isBack) || (!isBack && !isFront) ) {
                cout<<"ERROR::MtasProcessor:PreProcess ("<<(*chanEvtIter)->GetModuleNumber()<<" , " << (*chanEvtIter)->GetChannelNumber() << ") HAS BOTH FRONT AND BACK TAG OR NEITHER FRONT OR BACK TAG!"<<endl;
                return false;
            } else {
                cout<<"ERROR::MtasProcessor:PreProcess Channel ("<<(*chanEvtIter)->GetModuleNumber()<<" , " << (*chanEvtIter)->GetChannelNumber() << ") found which doesnt have a front or back tag, or you didnt set the Ring right (xml subtype). This means the XML is not right so you need to fix it!!"<<endl;
                return false;
            }

            int GlobalMtasSegID = RingOffset + segmentNum;
            int GlobalMtasChanID = (GlobalMtasSegID) * 2 + chanOffset;

            MtasSegMulti.at(GlobalMtasChanID)++;  // increment the multipliciy "map" based on GlobalMtasSegID
            if( RingOffset == -1 ){
                if (isFront && MtasCenterSegVec.at(vecindex).GetSegFront() == nullptr) {
                    MtasCenterSegVec.at(vecindex).SetSegFront((*chanEvtIter));
                    MtasCenterSegVec.at(vecindex).SetPixieRev(PixieRevision);
                }else if (isBack && MtasCenterSegVec.at(vecindex).GetSegBack() == nullptr) {
                    MtasCenterSegVec.at(vecindex).SetSegBack((*chanEvtIter));
                    MtasCenterSegVec.at(vecindex).SetPixieRev(PixieRevision);
                }
            }else if( RingOffset == 5 ){
                if (isFront && MtasInnerSegVec.at(vecindex).GetSegFront() == nullptr) {
                    MtasInnerSegVec.at(vecindex).SetSegFront((*chanEvtIter));
                    MtasInnerSegVec.at(vecindex).SetPixieRev(PixieRevision);
                }else if (isBack && MtasInnerSegVec.at(vecindex).GetSegBack() == nullptr) {
                    MtasInnerSegVec.at(vecindex).SetSegBack((*chanEvtIter));
                    MtasInnerSegVec.at(vecindex).SetPixieRev(PixieRevision);
                }
            }else if( RingOffset == 11 ){
                if (isFront && MtasMiddleSegVec.at(vecindex).GetSegFront() == nullptr) {
                    MtasMiddleSegVec.at(vecindex).SetSegFront((*chanEvtIter));
                    MtasMiddleSegVec.at(vecindex).SetPixieRev(PixieRevision);
                }else if (isBack && MtasMiddleSegVec.at(vecindex).GetSegBack() == nullptr) {
                    MtasMiddleSegVec.at(vecindex).SetSegBack((*chanEvtIter));
                    MtasMiddleSegVec.at(vecindex).SetPixieRev(PixieRevision);
                }
            }else if(RingOffset == 17 ){
                if (isFront && MtasOuterSegVec.at(vecindex).GetSegFront() == nullptr) {
                    MtasOuterSegVec.at(vecindex).SetSegFront((*chanEvtIter));
                    MtasOuterSegVec.at(vecindex).SetPixieRev(PixieRevision);
                }else if (isBack && MtasOuterSegVec.at(vecindex).GetSegBack() == nullptr) {
                    MtasOuterSegVec.at(vecindex).SetSegBack((*chanEvtIter));
                    MtasOuterSegVec.at(vecindex).SetPixieRev(PixieRevision);
                }
            }else{
                //whoops
            }

        }
    }  //! end loop over chanEvents.

    //! begin loop over segments for sums
	MTASCenter = 0;
	MTASInner = 0;
	MTASMiddle = 0;
	MTASOuter = 0;
	MTASTotal = 0;
	MTASFirstTime = 1.0e99;
    for( int ii = 0; ii < 6; ++ii ){
        MTASTotal += MtasCenterSegVec.at(ii).GetSegAvgEnergy();
        MTASTotal += MtasInnerSegVec.at(ii).GetSegAvgEnergy();
        MTASTotal += MtasMiddleSegVec.at(ii).GetSegAvgEnergy();
        MTASTotal += MtasOuterSegVec.at(ii).GetSegAvgEnergy();
        
        MTASCenter += MtasCenterSegVec.at(ii).GetSegAvgEnergy();

        MTASInner += MtasInnerSegVec.at(ii).GetSegAvgEnergy();

        MTASMiddle += MtasMiddleSegVec.at(ii).GetSegAvgEnergy();

        MTASOuter += MtasOuterSegVec.at(ii).GetSegAvgEnergy();

        if( MtasCenterSegVec.at(ii).GetEarliestSegTimeSansCfd() < MTASFirstTime )
            MTASFirstTime = MtasCenterSegVec.at(ii).GetEarliestSegTimeSansCfd();
        if( MtasInnerSegVec.at(ii).GetEarliestSegTimeSansCfd() < MTASFirstTime )
            MTASFirstTime = MtasInnerSegVec.at(ii).GetEarliestSegTimeSansCfd();
        if( MtasMiddleSegVec.at(ii).GetEarliestSegTimeSansCfd() < MTASFirstTime )
            MTASFirstTime = MtasMiddleSegVec.at(ii).GetEarliestSegTimeSansCfd();
        if( MtasOuterSegVec.at(ii).GetEarliestSegTimeSansCfd() < MTASFirstTime )
            MTASFirstTime = MtasOuterSegVec.at(ii).GetEarliestSegTimeSansCfd();
    }
    if( DetectorDriver::get()->GetSysRootOutput() ){
		MtasTotalsstruct.Total = MTASTotal;
		MtasTotalsstruct.Center = MTASCenter;
		MtasTotalsstruct.Inner = MTASInner;
		MtasTotalsstruct.Middle = MTASMiddle;
		MtasTotalsstruct.Outer = MTASOuter;	
		pixie_tree_event_->mtastotals_vec_.emplace_back(MtasTotalsstruct);
		MtasTotalsstruct = processor_struct::MTASTOTALS_DEFAULT_STRUCT; 
	}
	//! loop over segments for ploting

    double segmentAvg = -1.0;
    double segTdiff = -1.0;
    double position = 0.0;
    int segmentID = -1;
    for( int ii = 0; ii < 6; ++ii ){
        segmentID = ii + 1;
        if( MtasCenterSegVec.at(ii).IsValidSegment() ){
                segmentAvg = MtasCenterSegVec.at(ii).GetSegAvgEnergy();
                segTdiff = MtasCenterSegVec.at(ii).GetSegTdiffInNs();
                position = ((SD/2)*(1.0 + MtasCenterSegVec.at(ii).GetSegPosition()));

                plot(D_MTAS_TDIFF_OFFSET + CENTER_OFFSET + segmentID, segTdiff + SE/2 );

                plot(D_CONICIDENCE+ (2 * segmentID), MtasCenterSegVec.at(ii).GetSegFront()->GetCalibratedEnergy());
                plot(D_CONICIDENCE+ (2 * segmentID + 1), MtasCenterSegVec.at(ii).GetSegBack()->GetCalibratedEnergy());

                //! per segment plots
                plot(D_MTAS_SUM_FB + CENTER_OFFSET + segmentID, segmentAvg);
                plot(D_MTAS_CENTER_INDI, segmentAvg);
                plot(DD_MTAS_CS_T, MTASTotal/10.0, segmentAvg/10.0);
                plot(DD_MTAS_CS_CR,MTASCenter/10.0,segmentAvg/10.0);
        }
        if( MtasInnerSegVec.at(ii).IsValidSegment() ){
                segmentAvg = MtasInnerSegVec.at(ii).GetSegAvgEnergy();
                segTdiff = MtasInnerSegVec.at(ii).GetSegTdiffInNs();

                plot(D_MTAS_TDIFF_OFFSET + INNER_OFFSET + (segmentID - 6), segTdiff + SE / 2);

                plot(D_CONICIDENCE+ 12 + (2 * (segmentID-6)), MtasInnerSegVec.at(ii).GetSegFront()->GetCalibratedEnergy());
                plot(D_CONICIDENCE+ 12 + (2 * (segmentID-6) + 1), MtasInnerSegVec.at(ii).GetSegBack()->GetCalibratedEnergy());

                //! per segment plots
                plot(D_MTAS_SUM_FB + INNER_OFFSET + (segmentID - 6), segmentAvg);  //! subtract 12 to put it back in segments 1 - 6
                plot(DD_MTAS_IMO_T, MTASTotal/10.0, segmentAvg/10.0);
        }
        if( MtasMiddleSegVec.at(ii).IsValidSegment() ){
                segmentAvg = MtasMiddleSegVec.at(ii).GetSegAvgEnergy();
                segTdiff = MtasMiddleSegVec.at(ii).GetSegTdiffInNs();

                plot(D_MTAS_TDIFF_OFFSET + MIDDLE_OFFSET +  (segmentID-12), segTdiff + SE/2 );

                plot(D_CONICIDENCE+ 24 + (2 * (segmentID - 12)), MtasMiddleSegVec.at(ii).GetSegFront()->GetCalibratedEnergy());
                plot(D_CONICIDENCE+ 24 + (2 * (segmentID - 12) + 1), MtasMiddleSegVec.at(ii).GetSegBack()->GetCalibratedEnergy());

                //! per segment plots
                plot(D_MTAS_SUM_FB + MIDDLE_OFFSET + (segmentID-12), segmentAvg); //! subtract 12 to put it back in segments 1 - 6 
                plot(DD_MTAS_IMO_T,MTASTotal/10.0,segmentAvg/10.0);
        }
        if( MtasOuterSegVec.at(ii).IsValidSegment() ){
                segmentAvg = MtasOuterSegVec.at(ii).GetSegAvgEnergy();
                segTdiff = MtasOuterSegVec.at(ii).GetSegTdiffInNs();

                plot(D_MTAS_TDIFF_OFFSET + OUTER_OFFSET +  (segmentID-18), segTdiff + SE/2 );

                plot(D_CONICIDENCE+ 36 + (2 * (segmentID-18)), MtasOuterSegVec.at(ii).GetSegFront()->GetCalibratedEnergy());
                plot(D_CONICIDENCE+ 36 + (2 * (segmentID-18) + 1), MtasOuterSegVec.at(ii).GetSegBack()->GetCalibratedEnergy());

                //! per segment plots
                plot(D_MTAS_SUM_FB + OUTER_OFFSET + (segmentID-18), segmentAvg); //! subtract 12 to put it back in segments 1 - 6 
                plot(DD_MTAS_IMO_T,MTASTotal/10.0,segmentAvg/10.0);
        }

        //! Begin Root Output stuff.
        //if (DetectorDriver::get()->GetSysRootOutput()) {
        //    Mtasstruct.energy = segIter->GetSegAvgEnergy();
        //    Mtasstruct.fEnergy = segIter->GetFrontEnergy();
        //    Mtasstruct.bEnergy = segIter->GetBackEnergy();
        //    Mtasstruct.time = segIter->GetSegTimeAvgInNs();
        //    Mtasstruct.tdiff = segIter->GetSegTdiffInNs();
        //    Mtasstruct.gSegmentID = segIter->gMtasSegID_;
        //    Mtasstruct.segmentNum = segIter->RingSegNum_;
        //    Mtasstruct.Ring = segIter->segRing_;
        //    if (strcmp(segIter->segRing_.c_str(), "center") == 0 ) {
        //        Mtasstruct.RingNum = 1;
        //    } else if (strcmp(segIter->segRing_.c_str(), "inner") == 0 ) {
        //        Mtasstruct.RingNum = 2;
        //    } else if (strcmp(segIter->segRing_.c_str(), "middle") == 0 ) {
        //        Mtasstruct.RingNum = 3;
        //    } else if (strcmp(segIter->segRing_.c_str(), "outer") == 0 ) {
        //        Mtasstruct.RingNum = 4;
        //    }

        //    pixie_tree_event_->mtas_vec_.emplace_back(Mtasstruct);
        //    Mtasstruct = processor_struct::MTAS_DEFAULT_STRUCT;
        //}
    }

    //! sum spectra
	plot(D_MTAS_TOTAL, MTASTotal);
	plot(D_MTAS_CENTER, MTASCenter);
	plot(D_MTAS_INNER, MTASInner);
	plot(D_MTAS_MIDDLE, MTASMiddle);
	plot(D_MTAS_OUTER, MTASOuter);

	plot(DD_MTAS_CR_T, MTASTotal/10.0, MTASCenter/10.0);

	//! Plot Multis 
	for (unsigned it = 0; it < MtasSegMulti.size(); ++it){
		plot(D_ONE_OFFS + 0, it, MtasSegMulti.at(it));
	}


    //stop_time = std::chrono::high_resolution_clock::now();
    //std::chrono::duration<double,std::milli> dur = stop_time - start_time;
    //preprocesstime += dur.count();
	return true;
}

bool MtasProcessor::Process(RawEvent &event) {
    //start_time = std::chrono::high_resolution_clock::now();
	if (!EventProcessor::Process(event))
		return false;

	IsBetaEvent = false;
	IsIonEvent = false;
	if( HasBetaInfo ){
		if( TreeCorrelator::get()->checkPlace(BETANAME) ){
			if( TreeCorrelator::get()->place(BETANAME)->status() ){
				//This was a beta triggered event
				PlaceDetector* BetaEvt = dynamic_cast<PlaceDetector*>(TreeCorrelator::get()->place(BETANAME));

				//Can do fancier logic for beta triggering by checking
				//BetaEvt->last().type (type of MTASBeta EventData object) like with the BSM and possibly for silicon
				//otherwise just check if the energy is in a min and max range
				if( BetaEvt->info_.size() > 0 ){
					if( BetaEvt->last().type == BETATYPE ){
						if( BetaEvt->last().energy >= BetaMin and BetaEvt->last().energy <= BetaMax ){
							IsBetaEvent = true;
						}else if( BetaEvt->last().energy >= IonMin and BetaEvt->last().energy <= IonMax ){
							IsIonEvent = true;
						}
					}
				}

				if( IsBetaEvent ){
					plot(D_MTAS_TOTAL_BETA,MTASTotal);
					plot(D_MTAS_CENTER_BETA,MTASCenter);
					plot(D_MTAS_INNER_BETA,MTASInner);
					plot(D_MTAS_MIDDLE_BETA,MTASMiddle);
					plot(D_MTAS_OUTER_BETA,MTASOuter);

					plot(DD_MTAS_CR_T_BETA,MTASTotal/10.0,MTASCenter/10.0);

					plot(DD_MTAS_BETA_T_BETA,MTASTotal/10.0,BetaEvt->last().energy/10.0);
					plot(DD_MTAS_BETA_CR_BETA,MTASCenter/10.0,BetaEvt->last().energy/10.0);
					plot(DD_MTAS_BETA_IR_BETA,MTASInner/10.0,BetaEvt->last().energy/10.0);
					plot(DD_MTAS_BETA_MR_BETA,MTASMiddle/10.0,BetaEvt->last().energy/10.0);
					plot(DD_MTAS_BETA_OR_BETA,MTASOuter/10.0,BetaEvt->last().energy/10.0);

					for( unsigned int ii = 0; ii < 6; ++ii ){
						plot(DD_MTAS_CS_CR_BETA,MTASCenter/10.0,MtasCenterSegVec.at(ii).GetSegAvgEnergy()/10.0);
						plot(DD_MTAS_CS_T_BETA,MTASTotal/10.0,MtasCenterSegVec.at(ii).GetSegAvgEnergy()/10.0);

						//IMO plot
						plot(DD_MTAS_IMO_T_BETA,MTASTotal/10.0,MtasInnerSegVec.at(ii).GetSegAvgEnergy()/10.0);
						plot(DD_MTAS_IMO_T_BETA,MTASTotal/10.0,MtasMiddleSegVec.at(ii).GetSegAvgEnergy()/10.0);
						plot(DD_MTAS_IMO_T_BETA,MTASTotal/10.0,MtasOuterSegVec.at(ii).GetSegAvgEnergy()/10.0);
						
						plot(DD_MTAS_BETA_CS_BETA,MtasCenterSegVec.at(ii).GetSegAvgEnergy()/10.0,BetaEvt->last().energy/10.0);
						plot(DD_MTAS_BETA_IS_BETA,MtasInnerSegVec.at(ii).GetSegAvgEnergy()/10.0,BetaEvt->last().energy/10.0);
						plot(DD_MTAS_BETA_MS_BETA,MtasMiddleSegVec.at(ii).GetSegAvgEnergy()/10.0,BetaEvt->last().energy/10.0);
						plot(DD_MTAS_BETA_OS_BETA,MtasOuterSegVec.at(ii).GetSegAvgEnergy()/10.0,BetaEvt->last().energy/10.0);
					}


					PrevBetaEnergy = BetaEvt->last().energy;
					PrevBetaTimeStamp = BetaEvt->last().time;
				}
				if( IsIonEvent ){
					plot(D_MTAS_TOTAL_ION,MTASTotal);
					plot(D_MTAS_CENTER_ION,MTASCenter);
					plot(D_MTAS_INNER_ION,MTASInner);
					plot(D_MTAS_MIDDLE_ION,MTASMiddle);
					plot(D_MTAS_OUTER_ION,MTASOuter);

					plot(DD_MTAS_CR_T_ION,MTASTotal/10.0,MTASCenter/10.0);
					for( unsigned int ii = 0; ii < 6; ++ii ){
						plot(DD_MTAS_CS_CR_ION,MTASCenter/10.0,MtasCenterSegVec.at(ii).GetSegAvgEnergy()/10.0);
						plot(DD_MTAS_CS_T_ION,MTASTotal/10.0,MtasCenterSegVec.at(ii).GetSegAvgEnergy()/10.0);

						//IMO plot
						plot(DD_MTAS_IMO_T_ION,MTASTotal/10.0,MtasInnerSegVec.at(ii).GetSegAvgEnergy()/10.0);
						plot(DD_MTAS_IMO_T_ION,MTASTotal/10.0,MtasMiddleSegVec.at(ii).GetSegAvgEnergy()/10.0);
						plot(DD_MTAS_IMO_T_ION,MTASTotal/10.0,MtasOuterSegVec.at(ii).GetSegAvgEnergy()/10.0);
					}

					//current is an ion event and the previous was an ion event
					//this might work
					if( IsPrevIonTriggered ){
						plot(DD_MTAS_ION_ISOMER_T,MTASTotal/10.0,abs(MTASFirstTime - PrevIonTimeStamp));
						plot(DD_MTAS_BETA_T_ION_ISOMER,MTASTotal/10.0,PrevIonEnergy/10.0);
						plot(DD_MTAS_BETA_CR_ION_ISOMER,MTASCenter/10.0,PrevIonEnergy/10.0);
						plot(DD_MTAS_BETA_IR_ION_ISOMER,MTASInner/10.0,PrevIonEnergy/10.0);
						plot(DD_MTAS_BETA_MR_ION_ISOMER,MTASMiddle/10.0,PrevIonEnergy/10.0);
						plot(DD_MTAS_BETA_OR_ION_ISOMER,MTASOuter/10.0,PrevIonEnergy/10.0);
						for( unsigned int ii = 0; ii < 6; ++ii ){
							plot(DD_MTAS_BETA_CS_ION_ISOMER,MtasCenterSegVec.at(ii).GetSegAvgEnergy()/10.0,PrevIonEnergy/10.0);
							plot(DD_MTAS_BETA_IS_ION_ISOMER,MtasInnerSegVec.at(ii).GetSegAvgEnergy()/10.0,PrevIonEnergy/10.0);
							plot(DD_MTAS_BETA_MS_ION_ISOMER,MtasMiddleSegVec.at(ii).GetSegAvgEnergy()/10.0,PrevIonEnergy/10.0);
							plot(DD_MTAS_BETA_OS_ION_ISOMER,MtasOuterSegVec.at(ii).GetSegAvgEnergy()/10.0,PrevIonEnergy/10.0);
						}
					}
					PrevIonEnergy = BetaEvt->last().energy;
					PrevIonTimeStamp = BetaEvt->last().time;
				}
				//TreeCorrelator::get()->place("MTASBeta")->reset();
				IsPrevBetaTriggered = IsBetaEvent;
				IsPrevIonTriggered = IsIonEvent;

			}else{
				//Not a beta event fill the isomer plot
				if( IsPrevBetaTriggered ){
					plot(DD_MTAS_BETA_ISOMER_T,MTASTotal/10.0,abs(MTASFirstTime - PrevBetaTimeStamp));

					plot(DD_MTAS_BETA_T_BETA_ISOMER,MTASTotal/10.0,PrevBetaEnergy/10.0);
					plot(DD_MTAS_BETA_CR_BETA_ISOMER,MTASCenter/10.0,PrevBetaEnergy/10.0);
					plot(DD_MTAS_BETA_IR_BETA_ISOMER,MTASInner/10.0,PrevBetaEnergy/10.0);
					plot(DD_MTAS_BETA_MR_BETA_ISOMER,MTASMiddle/10.0,PrevBetaEnergy/10.0);
					plot(DD_MTAS_BETA_OR_BETA_ISOMER,MTASOuter/10.0,PrevBetaEnergy/10.0);
					for( unsigned int ii = 0; ii < 6; ++ii ){
						plot(DD_MTAS_BETA_CS_BETA_ISOMER,MtasCenterSegVec.at(ii).GetSegAvgEnergy()/10.0,PrevBetaEnergy/10.0);
						plot(DD_MTAS_BETA_IS_BETA_ISOMER,MtasInnerSegVec.at(ii).GetSegAvgEnergy()/10.0,PrevBetaEnergy/10.0);
						plot(DD_MTAS_BETA_MS_BETA_ISOMER,MtasMiddleSegVec.at(ii).GetSegAvgEnergy()/10.0,PrevBetaEnergy/10.0);
						plot(DD_MTAS_BETA_OS_BETA_ISOMER,MtasOuterSegVec.at(ii).GetSegAvgEnergy()/10.0,PrevBetaEnergy/10.0);
					}

					IsPrevBetaTriggered = false;
				}
				if( IsPrevIonTriggered ){
					plot(DD_MTAS_ION_ISOMER_T,MTASTotal/10.0,abs(MTASFirstTime - PrevIonTimeStamp));
					plot(DD_MTAS_BETA_T_ION_ISOMER,MTASTotal/10.0,PrevIonEnergy/10.0);
					plot(DD_MTAS_BETA_CR_ION_ISOMER,MTASCenter/10.0,PrevIonEnergy/10.0);
					plot(DD_MTAS_BETA_IR_ION_ISOMER,MTASInner/10.0,PrevIonEnergy/10.0);
					plot(DD_MTAS_BETA_MR_ION_ISOMER,MTASMiddle/10.0,PrevIonEnergy/10.0);
					plot(DD_MTAS_BETA_OR_ION_ISOMER,MTASOuter/10.0,PrevIonEnergy/10.0);
					for( unsigned int ii = 0; ii < 6; ++ii ){
						plot(DD_MTAS_BETA_CS_ION_ISOMER,MtasCenterSegVec.at(ii).GetSegAvgEnergy()/10.0,PrevIonEnergy/10.0);
						plot(DD_MTAS_BETA_IS_ION_ISOMER,MtasInnerSegVec.at(ii).GetSegAvgEnergy()/10.0,PrevIonEnergy/10.0);
						plot(DD_MTAS_BETA_MS_ION_ISOMER,MtasMiddleSegVec.at(ii).GetSegAvgEnergy()/10.0,PrevIonEnergy/10.0);
						plot(DD_MTAS_BETA_OS_ION_ISOMER,MtasOuterSegVec.at(ii).GetSegAvgEnergy()/10.0,PrevIonEnergy/10.0);
					}

					IsPrevIonTriggered = false;
				}
			}	
		}
	}

    Reset();
	EndProcess();
    //stop_time = std::chrono::high_resolution_clock::now();
    //std::chrono::duration<double,std::milli> dur = stop_time - start_time;
    //processtime += dur.count();
	return true;
}

void MtasProcessor::Reset(){
    for( size_t ii = 0; ii < 6; ++ii ){
        MtasCenterSegVec.at(ii).SetSegFront(nullptr);
        MtasCenterSegVec.at(ii).SetSegBack(nullptr);

        MtasInnerSegVec.at(ii).SetSegFront(nullptr);
        MtasInnerSegVec.at(ii).SetSegBack(nullptr);

        MtasMiddleSegVec.at(ii).SetSegFront(nullptr);
        MtasMiddleSegVec.at(ii).SetSegBack(nullptr);

        MtasOuterSegVec.at(ii).SetSegFront(nullptr);
        MtasOuterSegVec.at(ii).SetSegBack(nullptr);
    }
}
