/** \file TwoChanTimingProcessor.cpp
 * \brief Analyzes data from a simple Two channel Timing setup
 * \author D. P. Loureiro
 * \date June 1, 2017
 */
#include <fstream>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "TraceDumpData.hpp"
#include "TraceDump.hpp"

#include <TFile.h>
#include <TTree.h>
#include <TH1I.h>
#include <TH2I.h>

static TraceDumpData::HrtRoot rstart1;
static TraceDumpData::HrtRoot rstop1;
static TraceDumpData::HrtRoot rstart2;
static TraceDumpData::HrtRoot rstop2;
static std::vector <UInt_t> trace_start1;
static std::vector <UInt_t> trace_start2;
static std::vector <UInt_t> trace_stop1;
static std::vector <UInt_t> trace_stop2;



std::ofstream trcfile;
TFile *rootfile;
TTree *tree;
TH1I *codes;
TH2I *traces;
TH2I *traces_stop;
TH2I *superpulse_start;
TH2I *superpulse_stop;
enum CODES {
    PROCESS_CALLED,
    WRONG_NUM
};

namespace dammIds {
    namespace experiment {
    }
}

using namespace std;
using namespace dammIds::experiment;

TraceDump::TraceDump() :
        EventProcessor(OFFSET, RANGE, "TraceDump") {
    associatedTypes.insert("pulser");
    associatedTypes.insert("beta");

      std::cout<<"TraceDump::TraceDump()"<<std::endl;


    trcfile.open(Globals::get()->outputPath("trace.dat").c_str());

    stringstream rootname;

    rootname << Globals::get()->outputPath(Globals::get()->outputFile())
             << ".root";
    rootfile = new TFile(rootname.str().c_str(),"RECREATE");


    tree = new TTree("timing", "");
    tree->Branch("start1", &rstart1,"qdc/D:amp/D:snr:abase:sbase:id/b");
    tree->Branch("stop1", &rstop1, "qdc/D:amp/D:snr:abase:sbase:id/b");
    tree->Branch("start2", &rstart2,"qdc/D:amp/D:snr:abase:sbase:id/b");
    tree->Branch("stop2", &rstop2, "qdc/D:amp/D:snr:abase:sbase:id/b");
//    tree->Branch("start1", &rstart1,"qdc/D:time:snr:wtime:phase:abase:sbase:id/b");
//    tree->Branch("stop1", &rstop1, "qdc/D:time:snr:wtime:phase:abase:sbase:id/b");
//    tree->Branch("start2", &rstart2,"qdc/D:time:snr:wtime:phase:abase:sbase:id/b");
//    tree->Branch("stop2", &rstop2, "qdc/D:time:snr:wtime:phase:abase:sbase:id/b");
    tree->Branch("trace_start1",&trace_start1);
    tree->Branch("trace_start2",&trace_start2);
    tree->Branch("trace_stop1",&trace_stop1);
    tree->Branch("trace_stop2",&trace_stop2);
    tree->Branch("StartTimeStamp[2]",&StartTimeStamp,"StartTimestamp[2]/D");
    tree->Branch("StopTimeStamp[2]",&StopTimeStamp,"StopTimestamp[2]/D");
    tree->Branch("StartMaximum[2]",&StartMaxBin,"StartMax[2]/I");
    tree->Branch("StopTimeMaximum[2]",&StopMaxBin,"StopMax[2]/I");
    tree->Branch("StartChiSq",&StartChiSq,"StartChi/D");
    tree->Branch("StopChiSq",&StopChiSq,"StopChi/D");
    codes = new TH1I("codes", "", 40, 0, 40);
    StartTimeStamp[0]=-9999;
    StartTimeStamp[1]=-9999;
    StopTimeStamp[0]=-9999;
    StopTimeStamp[1]=-9999;
    StartMaxBin[0]=-999;
    StartMaxBin[1]=-999;
    StopMaxBin[0]=-999;
    StopMaxBin[1]=-999;
}

TraceDump::~TraceDump() {
          std::cout<<"TraceDump::~TraceDump()"<<std::endl;

    codes->Write();
    rootfile->Write();
    rootfile->Close();
    trcfile.close();
}

bool TraceDump::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;
      //std::cout<<"TraceDump::Process()"<<std::endl;

    //Define a map to hold the information
    TimingMap pulserMap;

    //plot the number of times we called the function
    codes->Fill(PROCESS_CALLED);

    static const vector<ChanEvent *> &pulserEvents =
            event.GetSummary("pulser")->GetList();
            //event.GetSummary("beta")->GetList();

    //if(pulserEvents.size()==4)
    //std::cout<<"TraceDump::Process()"<<pulserEvents.size()<<std::endl;

    for (vector<ChanEvent *>::const_iterator itPulser = pulserEvents.begin();
         itPulser != pulserEvents.end(); itPulser++) {
        int location = (*itPulser)->GetChanID().GetLocation();
        string subType = (*itPulser)->GetChanID().GetSubtype();
	//std::cout<<"TraceDump::Process()"<<location<<" "<<subType<<std::endl;

		TimingDefs::TimingIdentifier key(location, subType);
		pulserMap.insert(make_pair(key, TraceDumpData(*(*itPulser))));
	    }

    if (pulserMap.empty()) {
//    if (pulserMap.empty() || pulserMap.size() % 2 != 0) {    /// 2 Fold Coincidences
//    if (pulserMap.empty() || pulserMap.size() != 4) {         ///  4 Fold Coincidences
        //If the map is empty or size isn't even we return and increment
        // error code
        codes->Fill(WRONG_NUM);
        EndProcess();
        return false;
    }

TraceDumpData start1 =   (*pulserMap.find(make_pair(0, "start1"))).second;
TraceDumpData stop1 =    (*pulserMap.find(make_pair(0, "stop1"))).second;
TraceDumpData start2 =   (*pulserMap.find(make_pair(0, "start2"))).second;
TraceDumpData stop2 =    (*pulserMap.find(make_pair(0, "stop2"))).second;

 bool bstart1 = false;
 bool bstart2 = false;
 bool bstop1 = false;
 bool bstop2 = false;

if(pulserMap.count(make_pair(0, "start1"))>0)
  bstart1=true;
if(pulserMap.count(make_pair(0, "start2"))>0)
  bstart2=true;
if(pulserMap.count(make_pair(0, "stop1"))>0)
  bstop1=true;
if(pulserMap.count(make_pair(0, "stop2"))>0)
  bstop2=true;

    //std::cout<<"Start Max is "<< StartMaxValue<<std::endl;
    //std::cout<<"Stop Max is "<< StopMaxValue<<std::endl;
    
 if(bstart1)
   trace_start1=start1.GetTrace();
 if(bstart2)
   trace_start2=start2.GetTrace();
 if(bstop1)
   trace_stop1=stop1.GetTrace();
 if(bstop2)
   trace_stop2=stop2.GetTrace();
 
 //We only plot and analyze the data if the data is validated
//if (start1.GetIsValid() && stop1.GetIsValid()&&start2.GetIsValid() && stop2.GetIsValid() ) { //DPL: Should be &&!!
// if (start1.GetIsValid() && stop1.GetIsValid()&&start2.GetIsValid() ) { //DPL: Should be &&!!
   if(bstart1){
     if(start1.GetIsValid()){
       start1.FillRootStructure(rstart1);
       StartTimeStamp[0]=start1.GetTimeSansCfd()*Globals::get()->filterClockInSeconds()*1e9;
       StartMaxBin[0]= start1.GetTrace().GetMaxInfo().first;
     }
     else
       bstart1=false;
   }
   if(bstart2){
     if(start2.GetIsValid()){
       start2.FillRootStructure(rstart2);
       StartTimeStamp[1]=start2.GetTimeSansCfd()*Globals::get()->filterClockInSeconds()*1e9;
       StartMaxBin[1]=start2.GetTrace().GetMaxInfo().first;;
 
     }
     else
       bstart2=false;
   }
   if(bstop1){
     if(stop1.GetIsValid()){	
       stop1.FillRootStructure(rstop1);
       StopTimeStamp[0]=stop1.GetTimeSansCfd()*Globals::get()->filterClockInSeconds()*1e9;
       StopMaxBin[0]=stop1.GetTrace().GetMaxInfo().first;;
     }

      else
       bstop1=false;
  }
   if(bstop2){
     if(stop2.GetIsValid()){
       stop2.FillRootStructure(rstop2);
       StopTimeStamp[1]=stop2.GetTimeSansCfd()*Globals::get()->filterClockInSeconds()*1e9;
       StopMaxBin[1]=stop2.GetTrace().GetMaxInfo().first;;
    }
     else
       bstop2=false;

   }
   

   if (bstart1||bstart2||bstop1||bstop2) { //DPL: Should be &&!!
     tree->Fill();
     if(bstart1){
       start1.ZeroRootStructure(rstart1);
       trace_start1.clear();
     }
     if(bstart2){
       start2.ZeroRootStructure(rstart2);
       trace_start2.clear();
     }
     if(bstop1){
       stop1.ZeroRootStructure(rstop1);
       trace_stop1.clear();
     }
     if(bstop2){
       stop2.ZeroRootStructure(rstop2);
       trace_stop2.clear();
     }
   } // if (bstart1||bstart2||bstop1||bstop2) 

   for(Int_t k=0;k<2;k++){
     StartTimeStamp[k]=-9999;
     StopTimeStamp[k]=-9999;
     StartMaxBin[k]=-999;
     StopMaxBin[k]=-999;
  }
        StartChiSq = -9999;    
        StopChiSq = -9999;    

    EndProcess();
    //std::cout<<"********************************"<<std::endl;
    return true;
}
