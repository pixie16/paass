/** \file TwoChanTimingProcessor.cpp
 * \brief Analyzes data from a simple Two channel Timing setup
 * \author D. P. Loureiro
 * \date July 24, 2018
 */
#include <fstream>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "TraceDumpData.hpp"
#include "TraceDump_PSPMT.hpp"


#include <TFile.h>
#include <TTree.h>
#include <TH1I.h>
#include <TH2I.h>
#include <vector>

static TraceDumpData::HrtRoot ldynode;
static TraceDumpData::HrtRoot rdynode;
static TraceDumpData::HrtRoot lbeta;
static TraceDumpData::HrtRoot rbeta;



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

//Array of vectors to store the traces of the PSMPMT: [0] dynodes [1-4] anodes
static vector <UInt_t> trace_left_dynode;
static vector <UInt_t> trace_right_dynode;
static vector <UInt_t> trace_left_beta;
static vector <UInt_t> trace_right_beta;

TraceDump_PSPMT::TraceDump_PSPMT() :
        EventProcessor(OFFSET, RANGE, "TraceDump_PSPMT") {
    associatedTypes.insert("pspmt");

      std::cout<<"TraceDump_PSPMT::TraceDump_PSPMT()"<<std::endl;



    stringstream rootname;

    rootname << Globals::get()->outputPath(Globals::get()->outputFile())
             << ".root";
    prootfile_ = new TFile(rootname.str().c_str(),"RECREATE");


    proottree_ = new TTree("PSPMT", "");
    proottree_->Branch("left_start_time", &lbeta,"qdc/D:amp/D:snr:abase:sbase:id/b");
    proottree_->Branch("right_start_time", &rbeta,"qdc/D:amp/D:snr:abase:sbase:id/b");
    proottree_->Branch("left_stop_time", &ldynode,"qdc/D:amp/D:snr:abase:sbase:id/b");
    proottree_->Branch("right_stop_time", &rdynode,"qdc/D:amp/D:snr:abase:sbase:id/b");
    proottree_->Branch("left_qdc[4]", &left_qdc,"left_qdc[4]/I");
    proottree_->Branch("right_qdc[4]", &right_qdc,"right_qdc[4]/I");
    proottree_->Branch("left_max[4]", &left_max,"left_max[4]/I");
    proottree_->Branch("right_max[4]", &right_max,"right_max[4]/I");
    proottree_->Branch("nLeft",&nLeft,"nLeft/i");
    proottree_->Branch("nRight",&nRight,"nRight/i");
    proottree_->Branch("leftStartTimeStamp",&leftStartTimeStamp,"left_starttimeS/D");
    proottree_->Branch("rightStartTimeStamp",&rightStartTimeStamp,"right_starttimeS/D");
    proottree_->Branch("leftStopTimeStamp",&leftStopTimeStamp,"left_stoptimeS/D");
    proottree_->Branch("rightStopTimeStamp",&rightStopTimeStamp,"right_stoptimeS/D");
    proottree_->Branch("trace_left_dynode",&trace_left_dynode);
    proottree_->Branch("trace_right_dynode",&trace_right_dynode);
    proottree_->Branch("trace_left_beta",&trace_left_beta);
    proottree_->Branch("trace_right_beta",&trace_right_beta);
   codes = new TH1I("codes", "", 40, 0, 40);

    std::cout<<"TraceDump_PSPMT::TraceDump_PSPMT()->done"<<std::endl;

}

TraceDump_PSPMT::~TraceDump_PSPMT() {
          std::cout<<"TraceDump_PSPMT::~TraceDump_PSPMT()"<<std::endl;

    codes->Write();
    prootfile_->Write();
    prootfile_->Close();
    
}

bool TraceDump_PSPMT::Process(RawEvent &event) {
 
  //std::cout<<"TraceDump::Process()"<<std::endl;
 
   if (!EventProcessor::Process(event))
        return false;
 
    //plot the number of times we called the function
    codes->Fill(PROCESS_CALLED);

        //read in anode & dynode signals
    static const vector<ChanEvent *> &left_beta =event.GetSummary("beta:left")->GetList();
    static const vector<ChanEvent *> &right_beta =event.GetSummary("beta:right")->GetList();
    static const vector<ChanEvent *> &left_dynode =event.GetSummary("pspmt:dynode_left")->GetList();
    static const vector<ChanEvent *> &right_dynode =event.GetSummary("pspmt:dynode_right")->GetList();
    static const vector<ChanEvent *> &left_anode =event.GetSummary("pspmt:anode_left")->GetList();
    static const vector<ChanEvent *> &right_anode =event.GetSummary("pspmt:anode_right")->GetList();


//    std::cout<<"TraceDump_PSPMT::Process()"<<std::endl;
//    std::cout<<"left_dynode-> "<<left_dynode.size()<<std::endl;
//    std::cout<<"right_dynode-> "<<right_dynode.size()<<std::endl;
//    std::cout<<"left_anode-> "<<left_anode.size()<<std::endl;
//    std::cout<<"right_anode-> "<<right_anode.size()<<std::endl;
   nLeft = left_anode.size();
   nRight = right_anode.size();
 
    //Get the betas and dynodes
  if(left_beta.size()>0)
     trace_left_beta=left_beta.front()->GetTrace();
  if(right_beta.size()>0)
     trace_right_beta=right_beta.front()->GetTrace();
  if(left_dynode.size()>0)
     trace_left_dynode=left_dynode.front()->GetTrace();
  if(right_dynode.size()>0)
     trace_right_dynode=right_dynode.front()->GetTrace();

//   if (left_dynode.size()>0 && right_dynode.size()>0){}      											// 2 Dynode signals
//   if (left_dynode.size()>0 && right_dynode.size()>0 && nLeft==4 && nRight==4){}								// 2 Dynodes and 4 position signals
   if ((left_beta.size()>0 || right_beta.size()>0) && left_dynode.size()>0 && right_dynode.size()>0 && nLeft==4 && nRight==4){		// Triples (1 start && 2 stop) and 4 position signals
//   if (left_beta.size()>0 && right_beta.size()>0 && left_dynode.size()>0 && right_dynode.size()>0){}						// Quadruples (2 start && 2 stop)
    //Loop on left anodes
    for (vector<ChanEvent *>::const_iterator itLeft_anode = left_anode.begin();
         itLeft_anode != left_anode.end(); itLeft_anode++) {
      if((*itLeft_anode)->GetChanID().HasTag("v1") ){
	 left_qdc[0] = (*itLeft_anode)->GetTrace().GetQdc();
         left_max[0] = (*itLeft_anode)->GetTrace().GetMaxInfo().second;
      }
      if((*itLeft_anode)->GetChanID().HasTag("v2") ){
	 left_qdc[1] = (*itLeft_anode)->GetTrace().GetQdc();
         left_max[1] = (*itLeft_anode)->GetTrace().GetMaxInfo().second;
      }
      if((*itLeft_anode)->GetChanID().HasTag("v3") ){
	 left_qdc[2] = (*itLeft_anode)->GetTrace().GetQdc();
         left_max[2] = (*itLeft_anode)->GetTrace().GetMaxInfo().second;
      }
      if((*itLeft_anode)->GetChanID().HasTag("v4") ){
	 left_qdc[3] = (*itLeft_anode)->GetTrace().GetQdc();
         left_max[3] = (*itLeft_anode)->GetTrace().GetMaxInfo().second;
      }
    }
    
    //Loop on right anodes
     for (vector<ChanEvent *>::const_iterator itRight_anode = right_anode.begin();
         itRight_anode != right_anode.end(); itRight_anode++) {
      if((*itRight_anode)->GetChanID().HasTag("v1") ){
	 right_qdc[0] = (*itRight_anode)->GetTrace().GetQdc();
         right_max[0] = (*itRight_anode)->GetTrace().GetMaxInfo().second;
      }
      if((*itRight_anode)->GetChanID().HasTag("v2") ){
	 right_qdc[1] = (*itRight_anode)->GetTrace().GetQdc();
         right_max[1] = (*itRight_anode)->GetTrace().GetMaxInfo().second;
      }
      if((*itRight_anode)->GetChanID().HasTag("v3") ){
	 right_qdc[2] = (*itRight_anode)->GetTrace().GetQdc();
         right_max[2] = (*itRight_anode)->GetTrace().GetMaxInfo().second;
      }
      if((*itRight_anode)->GetChanID().HasTag("v4") ){
	 right_qdc[3] = (*itRight_anode)->GetTrace().GetQdc();
         right_max[3] = (*itRight_anode)->GetTrace().GetMaxInfo().second;
      }
   }




    TraceDumpData *startleft=0;
    TraceDumpData *startright=0;
    TraceDumpData *stopleft=0;
    TraceDumpData *stopright=0;
    if(left_beta.size()>0){
      startleft=new TraceDumpData(*(left_beta.front()));
      leftStartTimeStamp=startleft->GetTimeSansCfd()*Globals::get()->filterClockInSeconds()*1e9;
    }
    if(right_beta.size()>0){  
      startright=new TraceDumpData(*(right_beta.front()));
      rightStartTimeStamp=startright->GetTimeSansCfd()*Globals::get()->filterClockInSeconds()*1e9;
    }
    if(left_dynode.size()>0){
      stopleft=new TraceDumpData(*(left_dynode.front()));
      leftStopTimeStamp=stopleft->GetTimeSansCfd()*Globals::get()->filterClockInSeconds()*1e9;
    }
    if(right_dynode.size()>0){  
      stopright=new TraceDumpData(*(right_dynode.front()));
      rightStopTimeStamp=stopright->GetTimeSansCfd()*Globals::get()->filterClockInSeconds()*1e9;
    }

    bool aleft,aright,bleft,bright;

    if(startleft){
      startleft->FillRootStructure(lbeta);
      aleft=true;
    }
    else
      aleft=false;
    if(startright){
      startright->FillRootStructure(rbeta);
      aright=true;
    }
    else
      bright=false;
    if(stopleft){
      stopleft->FillRootStructure(ldynode);
      bleft=true;
    }
    else
      bleft=false;
    if(stopright){
      stopright->FillRootStructure(rdynode);
      bright=true;
    }
    else
      bright=false;
    
   if (bleft && bright) { //DPL: Should be &&!!
//   if (aleft && aright && bleft && bright) {} //DPL: Should be &&!!

     proottree_->Fill();

     if(aleft){
      startleft->ZeroRootStructure(lbeta);
      leftStartTimeStamp=-9999;
     }
     if(aright){
       startright->ZeroRootStructure(rbeta);
       rightStartTimeStamp=-9999;
     }
     if(bleft){
      stopleft->ZeroRootStructure(ldynode);
      leftStopTimeStamp=-9999;
     }
     if(bright){
       stopright->ZeroRootStructure(rdynode);
       rightStopTimeStamp=-9999;
     }
   } 
    }
      for (int i=0; i<4; i++){right_qdc[i]=0; right_max[i]=0;}
      for (int i=0; i<4; i++){left_qdc[i]=0; left_max[i]=0;}
      nLeft = 0;
      nRight = 0;

     trace_left_beta.clear();
     trace_right_beta.clear();
     trace_left_dynode.clear();
     trace_right_dynode.clear();
    
   EndProcess();
   //std::cout<<"********************************"<<std::endl;
   return true;
}
