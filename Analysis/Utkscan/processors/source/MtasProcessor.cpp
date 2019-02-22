//
// MTAS PROCESSOR based on pixie_slim
// will update description later
//


//==============================: INCLUDES :================================================//
#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <limits>
#include <set>
#include <vector> // added for std::vector


#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "MtasProcessor.hpp"

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
    }
}//namespace dammIds

// functions from std
using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::string;
using std::vector;

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

    DeclareHistogram1D( RAW + TOTAL   + 0, EnergyBins,  "Raw Total MTAS"     );
    DeclareHistogram1D( RAW + CENTRAL + 0, EnergyBins,  "Raw Total Central"  );
    DeclareHistogram1D( RAW + INNER   + 0, EnergyBins,  "Raw Total Inner"    );
    DeclareHistogram1D( RAW + MIDDLE  + 0, EnergyBins,  "Raw Total Middle"   );
    DeclareHistogram1D( RAW + OUTER   + 0, EnergyBins,  "Raw Total Outer"    );
}
//------------------------------------------------------------------------------------------//



//==============================: PREPROCESSOR :============================================//
bool MtasProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event)) {
        return false;
    }
    if (DetectorDriver::get()->GetSysRootOutput()){
        MTASstruct = processor_struct::MTAS_DEFAULT_STRUCT;
    }
  // TESTING FOR ROOT OUTPUT
    static const vector<ChanEvent *> &mtasList = event.GetSummary("mtas") -> GetList();

    double rawEnergy= 0., signalEnergy=0., time=0.;
    for(auto it = mtasList.begin(); it != mtasList.end(); it++){
        rawEnergy = (*it)->GetEnergy();
        signalEnergy = (*it)->GetCalibratedEnergy();
        time = (*it)->GetTime();
        if (DetectorDriver::get()->GetSysRootOutput()){
            MTASstruct.rawEnergy = rawEnergy;
            MTASstruct.calEnergy = signalEnergy;
            MTASstruct.time = time;
            MTASstruct.subtype = (*it)->GetChanID().GetSubtype();

            pixie_tree_event_->mtas_vec_.emplace_back(MTASstruct);
            MTASstruct = processor_struct::MTAS_DEFAULT_STRUCT;
        }
    }



    return true;
}



//==============================: MAIN PROCESSING FUNCTION :================================//
bool MtasProcessor::Process(RawEvent &event){

    if(!EventProcessor::Process(event)){
        return false;
    }

    //::::::::::::::: SUMMARIES INITIALIZATION :::::::::::::::::::::::::://


    // IF first time through, grab the detector summaries
    if( mtasSummary == nullptr){
        mtasSummary = event.GetSummary("mtas");
    }

    vector<ChanEvent *> mtasList = mtasSummary -> GetList();

    map<string, struct MtasData> mtasMap;
    //...................................................................//


    //::::::::::::::: PAIRING PMTS :::::::::::::::::::::::::::::::::::::://
    for(vector<ChanEvent*>::const_iterator mtasListIt = mtasList.begin();
            mtasListIt != mtasList.end(); mtasListIt++){

        string subtype = (*mtasListIt)->GetChanID().GetSubtype();

        // Skip all events with multiple signals in a single event
        if(mtasMap.count(subtype)>0) {
            //cout << "Error: Detector" << subtype << " has " << mtasMap.count(subtype) + 1 <<
            //     " signals in one event" << endl;
            continue;
        }

        // ignore all events with energy of 0 or more than 30000
        if( (*mtasListIt)->GetEnergy() == 0 || (*mtasListIt)->GetEnergy() > 30000){
            continue;
        }

        // Pair the subtype with the ?? - COMMENT
        mtasMap.insert(make_pair( subtype,MtasData(*mtasListIt) ));
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


    for(map<string, struct MtasData>::const_iterator mtasMapIt = mtasMap.begin();
            mtasMapIt != mtasMap.end(); mtasMapIt++) {

        double rawEnergy = (*mtasMapIt).second.energy;
        double signalEnergy = (*mtasMapIt).second.calEnergy;
        //double location = (*mtasMapIt).second.location;
        //double time = (*mtasMapIt).second.time;
        //int moduleIndex = (location - 1) / 2;

        //""""""""""""""" SORTING BY RING """"""""""""""""""""""""""//
        // if Subtype 'Central', add energy to Total MTAS and Total Central, accounting for 12 PMTs
        if ((*mtasMapIt).first[0] == 'C') {
            totalMtasEnergy.at(1) += signalEnergy / 12;
            totalMtasEnergy.at(0) += signalEnergy / 12;
            rawTotalMtasEnergy.at(1) += rawEnergy / 12;
            rawTotalMtasEnergy.at(0) += rawEnergy / 12;
            nrOfCentralPMT++;
        }
            // if Subtype 'Inner', add energy to Total MTAS and Total Inner, accounting for 2 PMTs
        else if ((*mtasMapIt).first[0] == 'I') {
            totalMtasEnergy.at(2) += signalEnergy / 2;
            totalMtasEnergy.at(0) += signalEnergy / 2;
            rawTotalMtasEnergy.at(2) += rawEnergy / 2;
            rawTotalMtasEnergy.at(0) += rawEnergy / 2;
        }
            // if Subtype 'Middle', add energy to Total MTAS and Total Inner, accounting for 2 PMTs
        else if ((*mtasMapIt).first[0] == 'M') {
            totalMtasEnergy.at(3) += signalEnergy / 2;
            totalMtasEnergy.at(0) += signalEnergy / 2;
            rawTotalMtasEnergy.at(3) += rawEnergy / 2;
            rawTotalMtasEnergy.at(0) += rawEnergy / 2;
        }
            // if Subtype 'Outer', add energy to Total MTAS and Total Inner, accounting for 2 PMTs
        else if ((*mtasMapIt).first[0] == 'O') {
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





    // update the processing time
    EndProcess();

    return true;
}

MtasProcessor::MtasData::MtasData(ChanEvent *chan) {
    detSubtype = chan->GetChanID().GetSubtype();
    energy = chan->GetEnergy();
    calEnergy = chan->GetCalibratedEnergy();
    time = chan->GetTime();
    location = chan->GetChanID().GetLocation();
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
