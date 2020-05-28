///@file MtasProcessor.cpp
///@brief Processes information from MTAS and silicon beta detectors. Based on PixieSlim MTAS scan code.
///@author M.Cooper, D.McKinnon
///@date March 13, 2020

//==============================: INCLUDES :================================================//
#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <limits>
#include <set>
#include <vector> // added for std::vector
#include <tuple>


#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "MtasProcessor.hpp"
#include "BarBuilder.hpp"

//------------------------------------------------------------------------------------------//


//==============================: NAMESPACE AND USING STATEMENTS :==========================//
/*
 * add mtas to dammIds namespace
 * OFFSET and RANGE set in DammPlotids.hpp
 */
namespace dammIds {
    namespace mtas {
//        const int SUMMED       = 100;
        const int NOGATE       = 200;
//        const int BETAGATE     = 300;
//        const int SILI         = 400;

        const int TOTAL        = 0;  // NOGATE, BETAGATE
        const int CENTRAL      = 10; // NOGATE, BETAGATE
        const int INNER        = 20; // NOGATE, BETAGATE
        const int MIDDLE       = 30; // NOGATE, BETAGATE
        const int OUTER        = 40; // NOGATE, BETAGATE
        const int GATE = 500;
        const int GATE_REAL = 501;
        const int D_BARMTASTOTAL_REALBETA = 503;
        const int D_BARMTASTOTAL_BETA = 502;
        const int D_BARMTASTOTAL_NOBETA = 504;
/*
        const int CYCLETIME    = 60; // NOGATE, BETAGATE
        const int CYCLENUMBER  = 70; // NOGATE
        const int GATE         = 80; // SILI
        const int TIME         = 80; // BETAGATE
        const int MULTIPLICITY = 90; // NOGATE, BETAGATE, SILI

        const int BACKGROUND   = 1; // NOGATE, BETAGATE
        const int LASER        = 2; // NOGATE, BETAGATE
        const int IRRADIATION  = 3; // NOGATE, BETAGATE
        const int SUM          = 5; // NOGATE, BETAGATE
*/
        const int RAW = 100;
        const int D_SILICON_UPPER = 400;
        const int D_SILICON_MTAS_GATE = 401;
        const int D_SILICON_MULTI_THRESH = 402;
        const int D_SILICON_MTAS_TDIFF = 403;
        const int DD_SILICON_ENERGY_CHAN = 410;
        const int DD_SILICON_ENERGY_CHAN_MTAS = 411;
        const int DD_SILICON_MTAS_TDIFF_MENERGY = 412;
        const int DD_SILICON_MTAS_TDIFF_SENERGY = 413;
        const int DD_SILICON_MTAS_TDIFF_SENERGY_INV = 414;
        const int DD_SILICON_MTAS_TDIFF_SENERGY_WALK = 415;
        const int DD_SILICON_MTAS_TDIFF_SENERGY_0 = 420;
        const int DD_SILICON_MTAS_TDIFF_SENERGY_1 = 421;
        const int DD_SILICON_MTAS_TDIFF_SENERGY_2 = 422;
        const int DD_SILICON_MTAS_TDIFF_SENERGY_3 = 423;
        const int DD_SILICON_MTAS_TDIFF_SENERGY_4 = 424;
        const int DD_SILICON_MTAS_TDIFF_SENERGY_5 = 425;
        const int DD_SILICON_MTAS_TDIFF_SENERGY_6 = 426;
        const int DD_SILICON_MTAS_TDIFF_SENERGY_7 = 427;
        const int DD_SILICON_MTAS_TDIFF_SENERGY_8 = 428;
        const int DD_SILICON_MTAS_TDIFF_SENERGY_9 = 429;
        const int DD_SILICON_MTAS_TDIFF_SENERGY_10 = 430;
        const int DD_SILICON_MTAS_TDIFF_SENERGY_11 = 431;
        const int DD_SILICON_MTAS_TDIFF_SENERGY_12 = 432;
        const int DD_SILICON_MTAS_TDIFF_SENERGY_13 = 433;
        const int DD_ENERGY_BARNUM = 434;
        const int DD_RAWENERGY_BARNUM = 435;
        const int DD_ENERGY_DETNUM = 436;
        const int DD_SILICON_MTAS_TDIFF_OUTSIDEBANANA = 437;
        const int DD_SILICON_MTAS_TDIFF_INSIDEBANANA = 438;
        const int DD_FRONTBACKENERGYDIFF_BARNUM = 439;
        const int DD_ENERGY_DETNUM_NOBETA = 440;
        const int DD_ENERGY_DETNUM_RAW = 441;


    }
}//namespace dammIds

// functions from std
/*using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::string;
using std::vector; */

using namespace std;
using namespace dammIds::mtas;

//------------------------------------------------------------------------------------------//


//==============================: CLASS INITIALIZATION :====================================//
MtasProcessor::MtasProcessor():
        EventProcessor(OFFSET, RANGE, "MtasProcessor"), //pulls offset and range from DammPlotids.hpp
        mtasSummary(nullptr)
{
    firstTime = -1.0;

    // the below are declared in the class definition and defined below if following the
            // Template on the wiki  - DVM20180319
    associatedTypes.insert("mtas");
    associatedTypes.insert("silicon");
}
//------------------------------------------------------------------------------------------//


//==============================: DECLARING PLOT IDS FUNCTION :=============================//
void MtasProcessor::DeclarePlots() { // (void) const in pixie_slim

    const int EnergyBins = SE;

    DeclareHistogram1D( NOGATE + TOTAL   + 0, EnergyBins,  "Total MTAS"     );
    DeclareHistogram1D( NOGATE + CENTRAL + 0, EnergyBins,  "Total Central"  );
    DeclareHistogram1D( NOGATE + INNER   + 0, EnergyBins,  "Total Inner"    );
    DeclareHistogram1D( NOGATE + MIDDLE  + 0, EnergyBins,  "Total Middle"   );
    DeclareHistogram1D( NOGATE + OUTER   + 0, EnergyBins,  "Total Outer"    );

    DeclareHistogram1D( GATE + TOTAL   + 0, EnergyBins,  "Total MTAS Gated"     );
    DeclareHistogram1D( GATE + CENTRAL + 0, EnergyBins,  "Total Central Gated"  );
    DeclareHistogram1D( GATE + INNER   + 0, EnergyBins,  "Total Inner Gated"    );
    DeclareHistogram1D( GATE + MIDDLE  + 0, EnergyBins,  "Total Middle Gated"   );
    DeclareHistogram1D( GATE + OUTER   + 0, EnergyBins,  "Total Outer Gated"    );

    DeclareHistogram1D( GATE_REAL + TOTAL   + 0, EnergyBins,  "Total MTAS Gated (Real Beta)"     );
    DeclareHistogram1D( GATE_REAL + CENTRAL + 0, EnergyBins,  "Total Central Gated (Real Beta)"  );
    DeclareHistogram1D( GATE_REAL + INNER   + 0, EnergyBins,  "Total Inner Gated (Real Beta)"    );
    DeclareHistogram1D( GATE_REAL + MIDDLE  + 0, EnergyBins,  "Total Middle Gated (Real Beta)"   );
    DeclareHistogram1D( GATE_REAL + OUTER   + 0, EnergyBins,  "Total Outer Gated (Real Beta)"    );

    DeclareHistogram1D( RAW + TOTAL   + 0, EnergyBins,  "Raw Total MTAS"     );
    DeclareHistogram1D( RAW + CENTRAL + 0, EnergyBins,  "Raw Total Central"  );
    DeclareHistogram1D( RAW + INNER   + 0, EnergyBins,  "Raw Total Inner"    );
    DeclareHistogram1D( RAW + MIDDLE  + 0, EnergyBins,  "Raw Total Middle"   );
    DeclareHistogram1D( RAW + OUTER   + 0, EnergyBins,  "Raw Total Outer"    );
    DeclareHistogram1D(D_SILICON_UPPER,S4,"Total Silicon Counts");
    DeclareHistogram1D(D_SILICON_MULTI_THRESH,S4,"Total Silicon Counts");
    DeclareHistogram1D(D_SILICON_MTAS_GATE,S4,"Total Silicon Counts MTAS Gated"); 
    DeclareHistogram1D(D_SILICON_MTAS_TDIFF,SB,"Time Difference Between MTAS and Silicons");
    DeclareHistogram1D(D_BARMTASTOTAL_REALBETA, SE, "MTAS Total Energy Bar Gated Real Beta");
    DeclareHistogram1D(D_BARMTASTOTAL_BETA, SE, "MTAS Total Energy Bar Gated Normal Beta");
    DeclareHistogram1D(D_BARMTASTOTAL_NOBETA, SE, "MTAS Total Energy Bar No Beta Gate");
    DeclareHistogram2D(DD_SILICON_ENERGY_CHAN,S4,SD,"Silicon Channel vs Energy 2D");
    DeclareHistogram2D(DD_SILICON_ENERGY_CHAN_MTAS,S4,SD,"Silicon Channel vs Energy 2D");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_MENERGY,SB,SE,"Time Difference Between MTAS and Silicons, Y_axis MTAS Energy");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_SENERGY,SB,SE,"Time Difference Between MTAS and Silicons, Y_axis Silicon Energy");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_SENERGY_INV,SE,SB,"Time Difference Between MTAS and Silicons, X_axis Silicon Energy");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_SENERGY_WALK,SE,SB,"Time Difference Between MTAS and Silicons, X_axis Silicon Energy");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_SENERGY_0,SA,SE,"Time Difference Between MTAS and Silicons, Y_axis Silicon Energy");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_SENERGY_1,SA,SE,"Time Difference Between MTAS and Silicons, Y_axis Silicon Energy");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_SENERGY_2,SA,SE,"Time Difference Between MTAS and Silicons, Y_axis Silicon Energy");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_SENERGY_3,SA,SE,"Time Difference Between MTAS and Silicons, Y_axis Silicon Energy");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_SENERGY_4,SA,SE,"Time Difference Between MTAS and Silicons, Y_axis Silicon Energy");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_SENERGY_5,SA,SE,"Time Difference Between MTAS and Silicons, Y_axis Silicon Energy");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_SENERGY_6,SA,SE,"Time Difference Between MTAS and Silicons, Y_axis Silicon Energy");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_SENERGY_7,SA,SE,"Time Difference Between MTAS and Silicons, Y_axis Silicon Energy");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_SENERGY_8,SA,SE,"Time Difference Between MTAS and Silicons, Y_axis Silicon Energy");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_SENERGY_9,SA,SE,"Time Difference Between MTAS and Silicons, Y_axis Silicon Energy");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_SENERGY_10,SA,SE,"Time Difference Between MTAS and Silicons, Y_axis Silicon Energy");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_SENERGY_11,SA,SE,"Time Difference Between MTAS and Silicons, Y_axis Silicon Energy");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_SENERGY_12,SA,SE,"Time Difference Between MTAS and Silicons, Y_axis Silicon Energy");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_SENERGY_13,SA,SE,"Time Difference Between MTAS and Silicons, Y_axis Silicon Energy");
    DeclareHistogram2D(DD_ENERGY_BARNUM,SE,S5,"Bar Energy Versus Bar Number");
    DeclareHistogram2D(DD_RAWENERGY_BARNUM,SE,S5,"Bar Raw Energy Versus Bar Number");
    DeclareHistogram2D(DD_ENERGY_DETNUM,SE,S7,"Energy vs Det Location (left right)");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_OUTSIDEBANANA,SA,SE, "Time Difference Between MTAS and Silicons, Y Axis MTAS Energy Outside Banana");
    DeclareHistogram2D(DD_SILICON_MTAS_TDIFF_INSIDEBANANA,SA,SE, "Time Difference Between MTAS and Silicons, Y Axis MTAS Energy Inside Banana");
    DeclareHistogram2D(DD_FRONTBACKENERGYDIFF_BARNUM,SE,S7,"Energy Difference Front/Back vs Bar Number");
    DeclareHistogram2D(DD_ENERGY_DETNUM_NOBETA,SE,S7,"No BG Energy vs Det Location (left right)");
    DeclareHistogram2D(DD_ENERGY_DETNUM_RAW,SE,S7,"No Bar No BG Energy vs Det Location (left right)");
    
    


    
}
//------------------------------------------------------------------------------------------//



//==============================: PREPROCESSOR :============================================//
bool MtasProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event)) {
        return false;
    }
    // maxEvent_ = NULL;

    vector<ChanEvent *> siliconList = event.GetSummary("mtas:silicon")-> GetList();
    vector<ChanEvent *> mtasList = event.GetSummary("mtas:nai")-> GetList();
    /*for (auto it=siliconList.begin();it!=siliconList.end();++it){
        if (maxEvent_ == NULL || (*it)->GetCalibratedEnergy() > maxEvent_->GetCalibratedEnergy())
        maxEvent_ = (*it);

    } */
    if (DetectorDriver::get()->GetSysRootOutput()){
        MTASstruct = processor_struct::MTAS_DEFAULT_STRUCT;
    
        // TESTING FOR ROOT OUTPUT

        double rawEnergy= 0., signalEnergy=0., time=0.;
        for(auto it = mtasList.begin(); it != mtasList.end(); it++){
            rawEnergy = (*it)->GetEnergy();
            signalEnergy = (*it)->GetCalibratedEnergy();
            time = (*it)->GetTime();
            MTASstruct.rawEnergy = rawEnergy;
            MTASstruct.calEnergy = signalEnergy;
            MTASstruct.time = time;
            MTASstruct.subtype = (*it)->GetChanID().GetSubtype();
            pixie_tree_event_->mtas_vec_.emplace_back(MTASstruct);
            MTASstruct = processor_struct::MTAS_DEFAULT_STRUCT;
        }
    }

    bars_.clear();

    BarBuilder billy(mtasList);
    billy.BuildBars();
    bars_ = billy.GetMtasBarMap();
    // cout << "barsize = " << bars_.size() << endl;


    return true;
}
// pixie_tree_event_->logic_vec_.emplace_back(MTASstruct); previously was -> mtas_vec This can be verified in PassRootStruct.hpp


//==============================: MAIN PROCESSING FUNCTION :================================//
bool MtasProcessor::Process(RawEvent &event){
    beta_ev = false;
    if(!EventProcessor::Process(event)){
        return false;
    }

    //::::::::::::::: SUMMARIES INITIALIZATION :::::::::::::::::::::::::://

    /*
    // IF first time through, grab the detector summaries
    if( mtasSummary == nullptr){
        mtasSummary = event.GetSummary("mtas:nai");
        siliSummary = event.GetSummary("mtas:silicon");
    }
    */
    vector<ChanEvent *> mtasList = event.GetSummary("mtas:nai")-> GetList();
    vector<ChanEvent *> siliconList = event.GetSummary("mtas:silicon")-> GetList();
        // std::string group = (*siliconListIt)->GetChanID().GetGroup();
    bool MTAS_event = false;
    if(!mtasList.empty()){
        MTAS_event = true;
    }

    bool real_beta = false;




    map<int, struct MtasData> mtasMap;
    // //...................................................................//
    for (auto it = bars_.begin(); it != bars_.end(); ++it){
        unsigned int barId = (*it).first.first;

        BarDetector bar = (*it).second;
        plot(DD_ENERGY_DETNUM_NOBETA,bar.GetLeftSide().GetEnergy(), barId * 2);
        plot(DD_ENERGY_DETNUM_NOBETA, bar.GetRightSide().GetEnergy(), barId * 2 + 1);
        }
    for (auto it = mtasList.begin(); it != mtasList.end();++it){
        plot(DD_ENERGY_DETNUM_RAW,(*it)->GetEnergy(), (*it)->GetChanID().GetLocation());
    }
    double bar_tot_MTAS_en = 0;
    for (auto it = bars_.begin(); it != bars_.end(); it++){
        bar_tot_MTAS_en += (*it).second.GetEnAvg();
    }
    plot(D_BARMTASTOTAL_NOBETA,bar_tot_MTAS_en);


    // Adding silicons

    if (!siliconList.empty()){
        //const ChanEvent* 
        silimax = event.GetSummary("mtas:silicon") -> GetMaxEvent();
        bool inMTAS = false;
        if (TreeCorrelator::get()->place("Measure")->status())
        {
                cout << "siliTime = " << siliTime << "              Cycle Start = " << endl; //CycleStart <<endl;       
                inMTAS = true;
        }

        if (silimax->GetEnergy() > 10 && silimax->GetEnergy() < 30000 && inMTAS==true){
            beta_ev = true;
            }
        // vector<double> energy_list(16);
        double CycleStart = TreeCorrelator::get()->place("Cycle")->last().time*Globals::get()->GetClockInSeconds();
        // cout << "Cycle Start = " << CycleStart << endl;
        siliTime = silimax->GetTimeSansCfd()*Globals::get()->GetClockInSeconds();
        max_sili_en = silimax->GetCalibratedEnergy();
        max_raw_sili_en = silimax->GetEnergy();
        bool flag_11 = true;
        if (silimax->GetChanID().GetGroup() == "d" && max_raw_sili_en < 750)
        {
            flag_11 = false;
        }

        if (!siliconList.empty() && !mtasList.empty() && max_sili_en > 200 && flag_11 == true)// && silimax->GetChanID().GetGroup() != "d")
        {
            // added silimax part 5/21/20 to test fitting when not including channel 12 (noise)
            MTAS_time = mtasList.front()->GetTimeSansCfd();
            double Sili_time2 = silimax->GetTimeSansCfd();
            double MTAS_time_walk = mtasList.front()->GetWalkCorrectedTime();
            double Sili_time_walk = silimax->GetWalkCorrectedTime();
            // T_diff_MTAS_Sili = MTAS_time - Sili_time2 + 75;
            int offset_1 = 100;
            T_diff_MTAS_Sili = Sili_time2 - MTAS_time + offset_1;
            double T_diff_MTAS_Sili_walk = Sili_time_walk - MTAS_time_walk + offset_1;
            // T_diff_MTAS_Sili = MTAS_time - (siliTime*(pow(10,9))) + 750;
            bool xy_beta_banana = false;
            if(max_raw_sili_en > 308 && T_diff_MTAS_Sili_walk > 96){
                xy_beta_banana = true;
                plot(DD_SILICON_MTAS_TDIFF_INSIDEBANANA, T_diff_MTAS_Sili,max_raw_sili_en);
                }else{
                    plot(DD_SILICON_MTAS_TDIFF_OUTSIDEBANANA, T_diff_MTAS_Sili,max_raw_sili_en);
                }
        //     // if(T_diff_MTAS_Sili_walk){
        //     //     T_dif
        //     // }


            plot(D_SILICON_MTAS_TDIFF, T_diff_MTAS_Sili);
            // for(auto it = mtasList.begin(); it !=mtasList.end();++it){
            // plot(DD_SILICON_MTAS_TDIFF_MENERGY, T_diff_MTAS_Sili,(*it)->GetCalibratedEnergy());
            // }
            for (auto it = bars_.begin(); it != bars_.end(); it++){
                unsigned int barId = (*it).first.first;

                BarDetector bar = (*it).second;
                plot(DD_ENERGY_BARNUM, bar.GetEnAvg(), barId);
                plot(DD_RAWENERGY_BARNUM, bar.GetRawEnAvg(), barId);
                plot(DD_ENERGY_DETNUM, bar.GetLeftSide().GetEnergy(), barId * 2);
                plot(DD_ENERGY_DETNUM, bar.GetRightSide().GetEnergy(), barId * 2 + 1);
                plot(DD_SILICON_MTAS_TDIFF_MENERGY, T_diff_MTAS_Sili,bar.GetLeftSide().GetCalibratedEnergy());
                plot(DD_SILICON_MTAS_TDIFF_MENERGY, T_diff_MTAS_Sili,bar.GetRightSide().GetCalibratedEnergy());
                double bar_en_diff = bar.GetLeftSide().GetCalibratedEnergy()*0.25 - bar.GetRightSide().GetCalibratedEnergy()*0.25 + 8000;
                plot(DD_FRONTBACKENERGYDIFF_BARNUM,bar_en_diff,barId);
            }

            plot(DD_SILICON_MTAS_TDIFF_SENERGY, T_diff_MTAS_Sili, max_raw_sili_en);
            plot(DD_SILICON_MTAS_TDIFF_SENERGY_INV, max_raw_sili_en, T_diff_MTAS_Sili);
            plot(DD_SILICON_MTAS_TDIFF_SENERGY_WALK, max_raw_sili_en, T_diff_MTAS_Sili_walk);
            if (T_diff_MTAS_Sili_walk < 100 && T_diff_MTAS_Sili_walk > 80)
            {
            real_beta = true;
            double bar_tot_MTAS_en = 0;
            for (auto it = bars_.begin(); it != bars_.end(); it++){
                bar_tot_MTAS_en += (*it).second.GetEnAvg();
                }
            plot(D_BARMTASTOTAL_REALBETA,bar_tot_MTAS_en);
            }
        }

        double startDelay = 16+2;
        double stopDelay = 16+2+40;
        // bool inMTAS = false;
        // if (TreeCorrelator::get()->place("Measure")->status())
        // {
        //         cout << "siliTime = " << siliTime << "              Cycle Start = " << CycleStart <<endl;       
        //         inMTAS = true;
        // }

        // if (silimax->GetEnergy() > 10 && silimax->GetEnergy() < 30000 && inMTAS==true){
        //     beta_ev = true;
        if (beta_ev){
            plot( D_SILICON_UPPER,silimap.find(silimax->GetChanID().GetGroup())->second);
            if (MTAS_event==true){
                plot( D_SILICON_MTAS_GATE,silimap.find(silimax->GetChanID().GetGroup())->second);
            }
            double bar_tot_MTAS_en = 0;
            for (auto it = bars_.begin(); it != bars_.end(); it++){
                bar_tot_MTAS_en += (*it).second.GetEnAvg();
                }
            plot(D_BARMTASTOTAL_BETA,bar_tot_MTAS_en);
        }

        for (auto it = siliconList.begin(); it != siliconList.end(); it++) {
        bool flag_silicon = false;
        double energy = (*it)->GetCalibratedEnergy();
        
        if ((*it)->GetEnergy() < siliconThresh) {
            continue;
        }
        plot(DD_SILICON_ENERGY_CHAN,silimap.find((*it)->GetChanID().GetGroup())->second,(*it)->GetEnergy());
        if (MTAS_event == true){
            plot(DD_SILICON_ENERGY_CHAN_MTAS,silimap.find((*it)->GetChanID().GetGroup())->second,(*it)->GetEnergy());
        }
        }
//         double currentTime2 = siliconList.front()->GetTimeSansCfd() * 10;
//         double lastTape = TreeCorrelator::get()->place("Beam_On")->last().time * Globals::get()->GetClockInSeconds();
// //      cout <<"lastTape= "<< lastTape<<endl;
//         double gTime = (*it)->GetTimeSansCfd() * Globals::get()->GetClockInSeconds();
//         double  CycleTdiff =  gTime - lastTape ;


        // std::string sub1 = (*it)->GetChanID().GetSubtype();

        // cout << temp_var;
        // cout << temp_var;
        //if (temp_var != 0){

        //}

        // if (silimax->GetChanID().GetGroup() == "a" && silimax->GetEnergy() > 150 && silimax->GetEnergy() < 30000) {
        //     // energy_list[9] = (*it)->GetCalibratedEnergy();
        //     plot(D_SILICON_MULTI_THRESH,9);
        // } 
        // else if (silimax->GetChanID().GetGroup() == "b" && silimax->GetEnergy() > 150 && silimax->GetEnergy() < 30000) {
        //     // energy_list[10] = (*it)->GetCalibratedEnergy();
        //     plot(D_SILICON_MULTI_THRESH,10);
        // } 
        // else if (silimax->GetChanID().GetGroup() == "c" && silimax->GetEnergy() > 75 && silimax->GetEnergy() < 30000) {
        //     // energy_list[11] = (*it)->GetCalibratedEnergy();
        //     plot(D_SILICON_MULTI_THRESH,11);
        // } 
        // else if (silimax->GetChanID().GetGroup() == "d" && silimax->GetEnergy() > 750 && silimax->GetEnergy() < 30000) {
        //     // energy_list[12] = (*it)->GetCalibratedEnergy();
        //     plot(D_SILICON_MULTI_THRESH,12);
        // } 
        // else if (silimax->GetChanID().GetGroup() == "e" && silimax->GetEnergy() > 250 && silimax->GetEnergy() < 30000) {
        //     // energy_list[13] = (*it)->GetCalibratedEnergy();
        //     plot(D_SILICON_MULTI_THRESH,13);
        // } 
        // else if (silimax->GetChanID().GetGroup() == "f" && silimax->GetEnergy() > 200 && silimax->GetEnergy() < 30000) {
        //     // energy_list[14] = (*it)->GetCalibratedEnergy();
        //     plot(D_SILICON_MULTI_THRESH,14);
        // } 
        // else if (silimax->GetChanID().GetGroup() == "g" && silimax->GetEnergy() > 150 && silimax->GetEnergy() < 30000) {
        //     // energy_list[15] = (*it)->GetCalibratedEnergy();
        //     plot(D_SILICON_MULTI_THRESH,15);
        // } 
        // else if (silimax->GetChanID().GetGroup() == "h" && silimax->GetEnergy() > 225 && silimax->GetEnergy() < 30000) {
        //     // energy_list[1] = (*it)->GetCalibratedEnergy();
        //     plot(D_SILICON_MULTI_THRESH,1);
        // } 
        // else if (silimax->GetChanID().GetGroup() == "i" && silimax->GetEnergy() > 225 && silimax->GetEnergy() < 30000) {
        //     // energy_list[2] = (*it)->GetCalibratedEnergy();
        //     plot(D_SILICON_MULTI_THRESH,2);
        // } 
        // else if (silimax->GetChanID().GetGroup() == "j" && silimax->GetEnergy() > 150 && silimax->GetEnergy() < 30000) {
        //     // energy_list[3] = (*it)->GetCalibratedEnergy();
        //     plot(D_SILICON_MULTI_THRESH,3);
        // } 
        // else if (silimax->GetChanID().GetGroup() == "k" && silimax->GetEnergy() > 150 && silimax->GetEnergy() < 30000) {
        //     // energy_list[4] = (*it)->GetCalibratedEnergy();
        //     plot(D_SILICON_MULTI_THRESH,4);
        // } 
        // else if (silimax->GetChanID().GetGroup() == "l" && silimax->GetEnergy() > 250 && silimax->GetEnergy() < 30000) {
        //     // energy_list[5] = (*it)->GetCalibratedEnergy();
        //     plot(D_SILICON_MULTI_THRESH,5);
        // } 
        // else if (silimax->GetChanID().GetGroup() == "m" && silimax->GetEnergy() > 200 && silimax->GetEnergy() < 30000) {
        //     // energy_list[6] = (*it)->GetCalibratedEnergy();
        //     plot(D_SILICON_MULTI_THRESH,6);
        // } 
        // else if (silimax->GetChanID().GetGroup() == "n" && silimax->GetEnergy() > 250 && silimax->GetEnergy() < 30000) {
        //     // energy_list[7] = (*it)->GetCalibratedEnergy();
        //     plot(D_SILICON_MULTI_THRESH,7);
        // } 

        
        // auto larg_beta = max_element(energy_list.begin(),energy_list.end());
        // int temp_var = distance(energy_list.begin(),larg_beta);

        
  

    }

    // End of silicon section




    //::::::::::::::: PAIRING PMTS :::::::::::::::::::::::::::::::::::::://
    for(vector<ChanEvent*>::const_iterator mtasListIt = mtasList.begin();
            mtasListIt != mtasList.end(); mtasListIt++){
        int channum = (*mtasListIt)->GetID();
        // string subtype = (*mtasListIt)->GetChanID().GetSubtype();
        // string group = (*mtasListIt)->GetChanID().GetGroup();
        std::pair<string,string> chan_detail;

        // Skip all events with multiple signals in a single event
        if(mtasMap.count(channum)>0) {
            //cout << "Error: Detector" << subtype << " has " << mtasMap.count(subtype) + 1 <<
            //     " signals in one event" << endl;
            continue;
        }
        if(channum>47) {
            //cout << "Error: Detector" << subtype << " has " << mtasMap.count(subtype) + 1 <<
            //     " signals in one event" << endl;
            continue;
        }

        // ignore all events with energy of 0 or more than 30000
        if( (*mtasListIt)->GetEnergy() == 0 || (*mtasListIt)->GetEnergy() > 30000){
            continue;
        }

        // Pair the subtype with the ?? - COMMENT
        mtasMap.insert(make_pair( channum,MtasData(*mtasListIt) ));
    }
    //...................................................................//


    //::::::::::::::: SPECTRA ::::::::::::::::::::::::::::::::::::::::::://

    /*
    0 - all mtas
    1 - Central
    2 - Inner
    3 - Middle
    4 - Outer
     */
    vector<double> totalMtasEnergy(5,-1);
    vector<double> rawTotalMtasEnergy(5,-1);
    /*
    0-5   - Central
    6-11  - Inner
    12-17 - Middle
    18-23 - Outer
     */
    vector<double> sumFrontBackEnergy(24,0);

    //-------------------------SPECTRA-VARIABLES-------------------------//
    int nrOfCentralPMT = 0;
    //double theSmallestCEnergy = 60000; // hardcoded? not sure why - dvm
    //-------------------------------------------------------------------//
    string location;
    for(map<int, struct MtasData>::const_iterator mtasMapIt = mtasMap.begin();
            mtasMapIt != mtasMap.end(); mtasMapIt++) {

        double rawEnergy = (*mtasMapIt).second.energy;
        double signalEnergy = (*mtasMapIt).second.calEnergy;
        //double location = (*mtasMapIt).second.location;
        //double time = (*mtasMapIt).second.time;
        //int moduleIndex = (location - 1) / 2;
        int x11 = (*mtasMapIt).first;
            if (x11 < 12){
                location = "C";
            }
            else if (x11 > 11 && x11 < 24){
                location = "I";
            }
            else if (x11 > 23 && x11 < 36){
                location = "M";
            }
            else if (x11 > 35){
                location = "O";
            }

        //""""""""""""""" SORTING BY RING """"""""""""""""""""""""""//
        // if Subtype 'Central', add energy to Total MTAS and Total Central, accounting for 12 PMTs
        if (location == "C") {
            totalMtasEnergy.at(1) += signalEnergy / 12;
            totalMtasEnergy.at(0) += signalEnergy / 12;
            rawTotalMtasEnergy.at(1) += rawEnergy / 12;
            rawTotalMtasEnergy.at(0) += rawEnergy / 12;
            nrOfCentralPMT++;
        }
            // if Subtype 'Inner', add energy to Total MTAS and Total Inner, accounting for 2 PMTs
        else if (location == "I") {
            totalMtasEnergy.at(2) += signalEnergy / 2;
            totalMtasEnergy.at(0) += signalEnergy / 2;
            rawTotalMtasEnergy.at(2) += rawEnergy / 2;
            rawTotalMtasEnergy.at(0) += rawEnergy / 2;
        }
            // if Subtype 'Middle', add energy to Total MTAS and Total Middle, accounting for 2 PMTs
        else if (location == "M") {
            totalMtasEnergy.at(3) += signalEnergy / 2;
            totalMtasEnergy.at(0) += signalEnergy / 2;
            rawTotalMtasEnergy.at(3) += rawEnergy / 2;
            rawTotalMtasEnergy.at(0) += rawEnergy / 2;
        }
            // if Subtype 'Outer', add energy to Total MTAS and Total Middle, accounting for 2 PMTs
        else if (location == "O") {
            totalMtasEnergy.at(4) += signalEnergy / 2;
            totalMtasEnergy.at(0) += signalEnergy / 2;
            rawTotalMtasEnergy.at(4) += rawEnergy / 2;
            rawTotalMtasEnergy.at(0) += rawEnergy / 2;
        }
        //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''//

        if (DetectorDriver::get()->GetSysRootOutput()){
            MTASstruct.rawEnergy = rawEnergy;
            MTASstruct.calEnergy = signalEnergy;
        }

    }


    for(unsigned int i = 0; i < 5; i++){
    plot( NOGATE + (i*10), totalMtasEnergy.at(i));
    plot( RAW + (i*10),rawTotalMtasEnergy.at(i));
    }
    for(unsigned int i = 0; i < 5; i++){
        if(beta_ev==true){
            plot( GATE + (i*10), totalMtasEnergy.at(i));
        }
    }
    for(unsigned int i = 0; i < 5; i++){
        if(real_beta==true){
            plot( GATE_REAL + (i*10), totalMtasEnergy.at(i));
        }
    }

    //:::::::::::::: TIME DIFFERENCE SILICON MTAS PLOTS :::::::::::::::::::::://
    






    // update the processing time
    EndProcess();

    return true;
}

MtasProcessor::MtasData::MtasData(ChanEvent *chan) {
    detSubtype = chan->GetChanID().GetSubtype();
    detGroup = chan ->GetChanID().GetGroup();
    energy = chan->GetEnergy();
    calEnergy = chan->GetCalibratedEnergy();
    time = chan->GetTime();
    location = chan->GetChanID().GetLocation();
    // bool upper = chan->GetChanID().HasTag("upper");
}
/*
//Setting initial states for flags. Can this be done in header? -dvm
bool MtasProcessor::isTapeMoveOn = false;
bool MtasProcessor::isMeasureOn = true;
bool MtasProcessor::isBkgOn = false;
bool MtasProcessor::isLaserOn = false;
bool MtasProcessor::isIrradOn = false;
double MtasProcessor::measureOnTime = -1;
unsigned MtasProcessor::cycleNumber = 0;
*/

// Light Pulser is now Laser - dvm
