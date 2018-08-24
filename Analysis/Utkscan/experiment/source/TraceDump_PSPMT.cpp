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
static vector <UInt_t> trace_left_anode_1;
static vector <UInt_t> trace_left_anode_2;
static vector <UInt_t> trace_left_anode_3;
static vector <UInt_t> trace_left_anode_4;
static vector <UInt_t> trace_right_dynode;
static vector <UInt_t> trace_right_anode_1;
static vector <UInt_t> trace_right_anode_2;
static vector <UInt_t> trace_right_anode_3;
static vector <UInt_t> trace_right_anode_4;


TraceDump_PSPMT::TraceDump_PSPMT() :
        EventProcessor(OFFSET, RANGE, "TraceDump_PSPMT") {
    associatedTypes.insert("pspmt");

      std::cout<<"TraceDump_PSPMT::TraceDump_PSPMT()"<<std::endl;



    stringstream rootname;

    rootname << Globals::get()->outputPath(Globals::get()->outputFile())
             << ".root";
    prootfile_ = new TFile(rootname.str().c_str(),"RECREATE");


    proottree_ = new TTree("PSPMT", "");
    proottree_->Branch("left_time", &ldynode,"qdc/D:amp/D:snr:abase:sbase:id/b");
    proottree_->Branch("right_time", &rdynode,"qdc/D:amp/D:snr:abase:sbase:id/b");
    proottree_->Branch("leftTimeStamp",&leftTimeStamp,"left_timeS/D");
    proottree_->Branch("rightTimeStamp",&rightTimeStamp,"right_timeS/D");
    proottree_->Branch("trace_left_dynode",&trace_left_dynode);
    proottree_->Branch("trace_left_anode_1",&trace_left_anode_1);
    proottree_->Branch("trace_left_anode_2",&trace_left_anode_2);
    proottree_->Branch("trace_left_anode_3",&trace_left_anode_2);
    proottree_->Branch("trace_left_anode_4",&trace_left_anode_4);
    proottree_->Branch("trace_right_dynode",&trace_right_dynode);
    proottree_->Branch("trace_right_anode_1",&trace_right_anode_1);
    proottree_->Branch("trace_right_anode_2",&trace_right_anode_2);
    proottree_->Branch("trace_right_anode_3",&trace_right_anode_3);
    proottree_->Branch("trace_right_anode_4",&trace_right_anode_4);
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
    static const vector<ChanEvent *> &left_dynode =event.GetSummary("pspmt:dynode_left")->GetList();
    static const vector<ChanEvent *> &right_dynode =event.GetSummary("pspmt:dynode_right")->GetList();
    static const vector<ChanEvent *> &left_anode =event.GetSummary("pspmt:anode_left")->GetList();
    static const vector<ChanEvent *> &right_anode =event.GetSummary("pspmt:anode_right")->GetList();


    std::cout<<"TraceDump_PSPMT::Process()"<<std::endl;
    std::cout<<"left_dynode-> "<<left_dynode.size()<<std::endl;
    std::cout<<"right_dynode-> "<<right_dynode.size()<<std::endl;
    std::cout<<"left_anode-> "<<left_anode.size()<<std::endl;
    std::cout<<"right_anode-> "<<right_anode.size()<<std::endl;


 
    //Get the dynodes
   if(left_dynode.size()>0)
     trace_left_dynode=left_dynode.front()->GetTrace();
  if(right_dynode.size()>0)
     trace_right_dynode=right_dynode.front()->GetTrace();

    //Loop on left anodes
    for (vector<ChanEvent *>::const_iterator itLeft_anode = left_anode.begin();
         itLeft_anode != left_anode.end(); itLeft_anode++) {
      if((*itLeft_anode)->GetChanID().HasTag("v1") && trace_left_anode_1.size()==0){
	 trace_left_anode_1=(*itLeft_anode)->GetTrace();
      }
      if((*itLeft_anode)->GetChanID().HasTag("v2") && trace_left_anode_2.size()==0){
	 trace_left_anode_2=(*itLeft_anode)->GetTrace();
      }
      if((*itLeft_anode)->GetChanID().HasTag("v3") && trace_left_anode_3.size()==0){
	 trace_left_anode_3=(*itLeft_anode)->GetTrace();
      }
      if((*itLeft_anode)->GetChanID().HasTag("v4") && trace_left_anode_4.size()==0){
	 trace_left_anode_4=(*itLeft_anode)->GetTrace();
      }

    }
    
    //Loop on right anodes
     for (vector<ChanEvent *>::const_iterator itRight_anode = right_anode.begin();
         itRight_anode != right_anode.end(); itRight_anode++) {
      if((*itRight_anode)->GetChanID().HasTag("v1") && trace_right_anode_1.size()==0){
	 trace_right_anode_1=(*itRight_anode)->GetTrace();
      }
      if((*itRight_anode)->GetChanID().HasTag("v2") && trace_right_anode_2.size()==0){
	 trace_right_anode_2=(*itRight_anode)->GetTrace();
      }
      if((*itRight_anode)->GetChanID().HasTag("v3") && trace_right_anode_3.size()==0){
	 trace_right_anode_3=(*itRight_anode)->GetTrace();
      }
      if((*itRight_anode)->GetChanID().HasTag("v4") && trace_right_anode_4.size()==0){
	 trace_right_anode_4=(*itRight_anode)->GetTrace();
      }

   }




    TraceDumpData *left=0;
    TraceDumpData *right=0;
    if(left_dynode.size()>0){
      left=new TraceDumpData(*(left_dynode.front()));
      leftTimeStamp=left->GetTimeSansCfd()*Globals::get()->filterClockInSeconds()*1e9;
    }
    if(right_dynode.size()>0){  
      right=new TraceDumpData(*(right_dynode.front()));
      rightTimeStamp=right->GetTimeSansCfd()*Globals::get()->filterClockInSeconds()*1e9;
    }

    bool bleft,bright;

    if(left){
      left->FillRootStructure(ldynode);
      bleft=true;
    }
    else
      bleft=false;
    if(right){
      right->FillRootStructure(rdynode);
      bright=true;
    }
    else
      bright=false;
    
   if (bleft ||bright) { //DPL: Should be &&!!

     proottree_->Fill();

     if(bleft){
      left->ZeroRootStructure(ldynode);
      leftTimeStamp=-999;
     }
     if(bright){
       right->ZeroRootStructure(rdynode);
       rightTimeStamp=-999;
     }
   } // if (bstart1||bstart2||bstop1||bstop2) 

     trace_left_dynode.clear();
     trace_left_anode_1.clear();
     trace_left_anode_2.clear();
     trace_left_anode_3.clear();
     trace_left_anode_4.clear();
     trace_right_dynode.clear();
     trace_right_anode_1.clear();
     trace_right_anode_2.clear();
     trace_right_anode_3.clear();
     trace_right_anode_4.clear();
    
   EndProcess();
   //std::cout<<"********************************"<<std::endl;
   return true;
}
