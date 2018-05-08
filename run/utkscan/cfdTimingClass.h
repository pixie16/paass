//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue May  8 15:18:06 2018 by ROOT version 6.08/02
// from TTree timing/
// found on file: EJ299_TeflonWrappedMylar_shortbar_1100V_Sr90_16b_001_CFD.root
//////////////////////////////////////////////////////////

#ifndef cfdTimingClass_h
#define cfdTimingClass_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TF1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>
#include <TFitResult.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <iostream>


// Header file for the classes stored in the TTree if any.
#include "vector"
#include <vector>
#include <algorithm>
#include <utility>
#include <iterator> 

class cfdTimingClass {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Double_t        start1_qdc;
   Double_t        start1_amp;
   Double_t        start1_time;
   Double_t        start1_snr;
   Double_t        start1_wtime;
   Double_t        start1_phase;
   Double_t        start1_abase;
   Double_t        start1_sbase;
   UChar_t         start1_id;
   Double_t        stop1_qdc;
   Double_t        stop1_amp;
   Double_t        stop1_time;
   Double_t        stop1_snr;
   Double_t        stop1_wtime;
   Double_t        stop1_phase;
   Double_t        stop1_abase;
   Double_t        stop1_sbase;
   UChar_t         stop1_id;
   Double_t        start2_qdc;
   Double_t        start2_amp;
   Double_t        start2_time;
   Double_t        start2_snr;
   Double_t        start2_wtime;
   Double_t        start2_phase;
   Double_t        start2_abase;
   Double_t        start2_sbase;
   UChar_t         start2_id;
   Double_t        stop2_qdc;
   Double_t        stop2_amp;
   Double_t        stop2_time;
   Double_t        stop2_snr;
   Double_t        stop2_wtime;
   Double_t        stop2_phase;
   Double_t        stop2_abase;
   Double_t        stop2_sbase;
   UChar_t         stop2_id;
   vector<unsigned int> *trace_start1;
   vector<unsigned int> *trace_start2;
   vector<unsigned int> *trace_stop1;
   vector<unsigned int> *trace_stop2;
   Double_t        StartTimeStamp[2];
   Double_t        StopTimeStamp[2];
   Int_t           StartMaximum[2];
   Int_t           StopTimeMaximum[2];
   Double_t        StartChiSq;
   Double_t        StopChiSq;

   // List of branches
   TBranch        *b_start1;   //!
   TBranch        *b_stop1;   //!
   TBranch        *b_start2;   //!
   TBranch        *b_stop2;   //!
   TBranch        *b_trace_start1;   //!
   TBranch        *b_trace_start2;   //!
   TBranch        *b_trace_stop1;   //!
   TBranch        *b_trace_stop2;   //!
   TBranch        *b_StartTimestamp;   //!
   TBranch        *b_StopTimestamp;   //!
   TBranch        *b_StartMax;   //!
   TBranch        *b_StopMax;   //!
   TBranch        *b_StartChi;   //!
   TBranch        *b_StopChi;   //!

   // Variable to Save
   Double_t phase[4];
   Double_t max[4];
   Double_t qdc[4];
   Double_t time[4];
   Double_t sbase[4];
   Double_t abase[4];

   cfdTimingClass(TTree *tree=0);
   virtual ~cfdTimingClass();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop(Long64_t nentries =-1,const Char_t *filename=NULL);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
   virtual void     DigitalCFD(Long64_t entry = -1, Double_t fraction=0.4, Int_t delay=2, Bool_t kDraw = kFALSE);
   virtual void     PolyCFD(Long64_t entry = -1; Int_t Thresh = 0.5);
};

#endif

#ifdef cfdTimingClass_cxx
cfdTimingClass::cfdTimingClass(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("EJ299_TeflonWrappedMylar_shortbar_1100V_Sr90_16b_001_CFD.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("EJ299_TeflonWrappedMylar_shortbar_1100V_Sr90_16b_001_CFD.root");
      }
      f->GetObject("timing",tree);

   }
   Init(tree);

    

}

cfdTimingClass::~cfdTimingClass()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t cfdTimingClass::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t cfdTimingClass::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void cfdTimingClass::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   trace_start1 = 0;
   trace_start2 = 0;
   trace_stop1 = 0;
   trace_stop2 = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("start1", &start1_qdc, &b_start1);
   fChain->SetBranchAddress("stop1", &stop1_qdc, &b_stop1);
   fChain->SetBranchAddress("start2", &start2_qdc, &b_start2);
   fChain->SetBranchAddress("stop2", &stop2_qdc, &b_stop2);
   fChain->SetBranchAddress("trace_start1", &trace_start1, &b_trace_start1);
   fChain->SetBranchAddress("trace_start2", &trace_start2, &b_trace_start2);
   fChain->SetBranchAddress("trace_stop1", &trace_stop1, &b_trace_stop1);
   fChain->SetBranchAddress("trace_stop2", &trace_stop2, &b_trace_stop2);
   fChain->SetBranchAddress("StartTimeStamp[2]", StartTimeStamp, &b_StartTimestamp);
   fChain->SetBranchAddress("StopTimeStamp[2]", StopTimeStamp, &b_StopTimestamp);
   fChain->SetBranchAddress("StartMaximum[2]", StartMaximum, &b_StartMax);
   fChain->SetBranchAddress("StopTimeMaximum[2]", StopTimeMaximum, &b_StopMax);
   fChain->SetBranchAddress("StartChiSq", &StartChiSq, &b_StartChi);
   fChain->SetBranchAddress("StopChiSq", &StopChiSq, &b_StopChi);
   Notify();
}

Bool_t cfdTimingClass::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void cfdTimingClass::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t cfdTimingClass::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
void cfdTimingClass::DigitalCFD(Long64_t entry,Double_t fraction, Int_t delay, Bool_t kDraw){


    GetEntry(entry);
    
    TGraph *g[4];
    TGraph *gCFD[4];

//    Double_t baseline[4]={start1_abase,start2_abase,stop1_abase,stop2_abase};
   Double_t T_max[4] = {start1_amp,start2_amp,stop1_amp,stop2_amp};
   Double_t T_qdc[4] = {start1_qdc,start2_qdc,stop1_qdc,stop2_qdc};
   Double_t T_time[4] = {StartTimeStamp[0],StartTimeStamp[1],StopTimeStamp[0],StopTimeStamp[1]};
   Double_t T_sbase[4] = {start1_sbase,start2_sbase,stop1_sbase,stop2_sbase};
   Double_t T_abase[4] = {start1_abase,start2_abase,stop1_abase,stop2_abase};

    UInt_t size[4];
    size[0]=trace_start1->size();
    size[1]=trace_start2->size();
    size[2]=trace_stop1->size();
    size[3]=trace_stop2->size();
                                 
    std::vector<double> *cfd=new std::vector<double>();
    std::vector <unsigned int>*trace;
    int diff=0;
    
    //cout << size[0] << " " << size[1] << " " << size[2] << " " << size[3]<< endl;
    for (int iT=0; iT<4; iT++){
    if (size[iT]==0){
//          cout<< "No Trace for " << iT << endl; 
          phase[iT]=-9999;
          continue;}
    else {
      g[iT]=new TGraph();
      gCFD[iT]=new TGraph();
      switch(iT){
      case 0:
	trace = trace_start1;
	break;
      case 1:
	trace = trace_start2;
	break;
      case 2:
	trace = trace_stop1;
	break;
      case 3:
	trace = trace_stop2;
	break;
      default:
	break;
      }
      Double_t minimum=9999;
      Double_t maximum=-9999;
      Int_t minimumX=0;
      Int_t maximumX=0;
      //cout<<"iT is "<<iT<<endl;
      for (unsigned int i = 0; i < trace->size() - delay; i++){
        cfd->push_back(fraction * (double)trace->at(i) - (double)trace->at(i + delay));
	if(cfd->at(i)>maximum){
	  maximum=cfd->at(i);
	  maximumX=i;
	}
	if(cfd->at(i)<minimum){
	  minimum=cfd->at(i);
	  minimumX=i;
	}
	gCFD[iT]->SetPoint(i,i,cfd->at(i));
	g[iT]->SetPoint(i,i,trace->at(i));
      }
      TF1 fun("fun","pol1", minimumX+1,maximumX-1);
      TF1 fun2("fun2","pol0",0,minimumX-10);
      gCFD[iT]->Fit(&fun,"RQ");    
      gCFD[iT]->Fit(&fun2,"RQ+");
      phase[iT]= (fun2.GetParameter(0)-fun.GetParameter(0))/fun.GetParameter(1);
     // cout<<iT<<" phase: "<< phase[iT]<< "\t";
      cfd->clear();
    }
     //cout << endl;

    max[iT] = T_max[iT];
    qdc[iT] = T_qdc[iT];
    abase[iT] = T_abase[iT];
    sbase[iT] = T_sbase[iT];
    time[iT] = T_time[iT]+phase[iT]*4.0;

    }//end for loop
    if(kDraw){
      TCanvas *c=new TCanvas();
      c->Divide(2,2);
      for(Int_t j=2;j<4;j++){
	c->cd(j+1);
	if(g[j])
	  g[j]->Draw("AL*");
	if(gCFD[j]){
	  gCFD[j]->SetLineColor(4);
	  gCFD[j]->SetMarkerColor(4);
	  gCFD[j]->Draw("AL*");
	}
	//gPad->WaitPrimitive();
      }
    }
}

void cfdTimingClass::PolyCFD(Long64_t entry; Int_t Thresh){
 GetEntry(entry);

   Double_t T_max[4] = {start1_amp,start2_amp,stop1_amp,stop2_amp};
   Double_t T_qdc[4] = {start1_qdc,start2_qdc,stop1_qdc,stop2_qdc};
   Double_t T_time[4] = {StartTimeStamp[0],StartTimeStamp[1],StopTimeStamp[0],StopTimeStamp[1]};
   Double_t T_sbase[4] = {start1_sbase,start2_sbase,stop1_sbase,stop2_sbase};
   Double_t T_abase[4] = {start1_abase,start2_abase,stop1_abase,stop2_abase};

    UInt_t size[4];
    size[0]=trace_start1->size();
    size[1]=trace_start2->size();
    size[2]=trace_stop1->size();
    size[3]=trace_stop2->size();

 for (int m=0;m<4;m++){

  vector <UInt_t>::iterator it;
  UInt_t max_position=0;
  switch(m){
  case 0:
    if(trace_start1->size()!=0){
      it=max_element(trace_start1->begin(),trace_start1->end());
      max_position=distance(trace_start1->begin(),it);
    }
    break;
  case 1:
    if(trace_start2->size()!=0){
      it=max_element(trace_start2->begin(),trace_start2->end());
      max_position=distance(trace_start2->begin(),it);
    }
    break;
  case 2:
    if(trace_stop1->size()!=0){
      it=max_element(trace_stop1->begin(),trace_stop1->end());
      max_position=distance(trace_stop1->begin(),it);
    }
    break;
  case 3:
    if(trace_stop2->size()!=0){
      it=max_element(trace_stop2->begin(),trace_stop2->end());
      max_position=distance(trace_stop2->begin(),it);
    }
    break;
  default:
    break;
  }
     


}
#endif // #ifdef cfdTimingClass_cxx
