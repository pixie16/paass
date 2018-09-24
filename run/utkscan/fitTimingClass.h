//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri Aug 31 10:12:52 2018 by ROOT version 6.14/00
// from TTree timing/
// found on file: YSO_YSO_900_900_AMP1V_60Co.root
//////////////////////////////////////////////////////////

#ifndef fitTimingClass_h
#define fitTimingClass_h

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
#include <TLatex.h>

// Header file for the classes stored in the TTree if any.
#include "vector"
#include <vector>
#include <algorithm>
#include <utility>
#include <iterator> 
#include "SiPMTimingFunction.cpp"


class fitTimingClass {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Double_t        start1_qdc;
   Double_t        start1_amp;
   Double_t        start1_snr;
   Double_t        start1_abase;
   Double_t        start1_sbase;
   UChar_t         start1_id;
   Double_t        stop1_qdc;
   Double_t        stop1_amp;
   Double_t        stop1_snr;
   Double_t        stop1_abase;
   Double_t        stop1_sbase;
   UChar_t         stop1_id;
   Double_t        start2_qdc;
   Double_t        start2_amp;
   Double_t        start2_snr;
   Double_t        start2_abase;
   Double_t        start2_sbase;
   UChar_t         start2_id;
   Double_t        stop2_qdc;
   Double_t        stop2_amp;
   Double_t        stop2_snr;
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

   //My Stuff
   Double_t beta[4];
   Double_t gamma[4];
   Double_t ROOT_baseline[4];
   Double_t ROOT_phase[4];
   Double_t ROOT_CFDphase[4];
   Double_t ROOT_time[4];
   Double_t ROOT_beta[4];
   Double_t ROOT_gamma[4];
   Double_t ROOT_chisq[4];
   Double_t ROOT_qdc[4];
   Double_t ROOT_max[4];
   Double_t ROOT_delta[4];

//   Int_t fSamplingRate;

   Bool_t fFixParameters;

   TGraphErrors *fTraces[4];
   TF1 *fFits[4];
   TGraph *fResiduals[4];
   Int_t fStatus[4]={-1,-1,-1,-1};
   const Int_t factor=1;

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

   fitTimingClass(TTree *tree=0);
   virtual ~fitTimingClass();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void Loop(Long64_t nentries =-1,const Char_t *filename=NULL);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
   virtual void     Plot(Long64_t entry = -1,Bool_t draw=kFALSE);
   virtual void     Fit(Long64_t entry = -1, Bool_t kDraw=kFALSE);
   virtual Double_t   QDCcalc(vector <UInt_t> *dTrace, int chan, UInt_t maxpos, Double_t baseline);
   virtual Double_t   CalcBaseline(vector <UInt_t> *dTrace, UInt_t initialpos, UInt_t finalpos);
};

#endif

#ifdef fitTimingClass_cxx
fitTimingClass::fitTimingClass(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("YSO_YSO_900_900_AMP1V_60Co.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("YSO_YSO_900_900_AMP1V_60Co.root");
      }
      f->GetObject("timing",tree);

   }
   Init(tree);
   Char_t fname[256];
  SiPMTimingFunction sipmfunc;
  //PMTErfTimingFunction pmtfunc;

   for(Int_t j=0;j<4;j++){
   beta[j]=0.08;
   gamma[j]=0.26;
   sprintf(fname,"f_%d",j);
//   fTraces[j]=new TGraph();
   fTraces[j]=new TGraphErrors();
   fResiduals[j]=new TGraph();
   fFits[j]=new TF1(fname,sipmfunc,0,1,6);
   //fFits[j]=new TF1(fname,sipmfunc,0,1,5);
   }

}

fitTimingClass::~fitTimingClass()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
  for(Int_t j=0;j<4;j++){
    delete fResiduals[j];
    delete fTraces[j];
    delete fFits[j];
   }
  }

Int_t fitTimingClass::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t fitTimingClass::LoadTree(Long64_t entry)
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

void fitTimingClass::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

  //SetSamplingRate(4); //500 MSPS
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

Bool_t fitTimingClass::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void fitTimingClass::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t fitTimingClass::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
void fitTimingClass::Plot(Long64_t entry,Bool_t kDraw){

  GetEntry(entry);

  const UInt_t size0=trace_start1->size();
  const UInt_t size1=trace_start2->size();
  const UInt_t size2=trace_stop1->size();
  const UInt_t size3=trace_stop2->size();

  /* cout<<"Size "<<size0<<endl;  */
  /* cout<<"Size "<<size1<<endl;  */
  /* cout<<"Size "<<size2<<endl;  */
  /* cout<<"Size "<<size3<<endl;  */
  
  for(Int_t j=0;j<4;j++){
    fTraces[j]->Set(0); 
  }

  if(size0!=0){
    for(UInt_t j=0;j<size0;j++){
      fTraces[0]->SetPoint(j,j,trace_start1->at(j));
      fTraces[0]->SetPointError(j,0,start1_sbase);
    }
  }
  if(size1!=0){
    for(UInt_t j=0;j<size1;j++){
     fTraces[1]->SetPoint(j,j,trace_start2->at(j));
      fTraces[1]->SetPointError(j,0,start2_sbase);
    }
  }
  if(size2!=0){
    for(UInt_t j=0;j<size2;j++){
      fTraces[2]->SetPoint(j,j,trace_stop1->at(j));
      fTraces[2]->SetPointError(j,0,stop1_sbase);
    }
  }
  if(size3!=0){
    for(UInt_t j=0;j<size3;j++){
     fTraces[3]->SetPoint(j,j,trace_stop2->at(j));
      fTraces[3]->SetPointError(j,0,stop2_sbase);
    }
  }


  if(size0==0&&size1==0&&size0==0&&size3==0) return ;
  else if(kDraw){
          TCanvas *c;
      if(!gPad){
	c=new TCanvas();
	c->Divide(2,2); 
      }
      else{
	//  c=(TCanvas*)gPad;
	c=gPad->GetCanvas();
	c->Clear();
	c->Divide(2,2);     
      }
      for(Int_t i=0;i<4;i++){
	c->cd(i+1);   
	if(fTraces[i]){
	  fTraces[i]->Draw("AL*");
          fTraces[i]->GetXaxis()->SetRangeUser(10,130);
	}
	else
	  continue;
      }
    }
  return;
 }
void  fitTimingClass::Fit(Long64_t entry, Bool_t kDraw){

//  bool kDraw = false;
  Plot(entry,kDraw);
//  TF1 *fit[4];
  

  
  Double_t timestamp[4]={StartTimeStamp[0],StartTimeStamp[1],StopTimeStamp[0],StopTimeStamp[1]};
  Double_t delta[4]={4,4,4,4};


  Double_t fit_beta[4]={0.21,0.196,0.22,0.21};  // low Gain PMT 500 MSPS
  Double_t fit_gamma[4]={0.148,0.133,0.12,0.10};  // low Gain PMT 500 MSPS

  vector<pair<Int_t,Int_t>>fit_limits;
  fit_limits.push_back(make_pair(10,4));
  fit_limits.push_back(make_pair(10,4));
  fit_limits.push_back(make_pair(10,4));
  fit_limits.push_back(make_pair(10,4));
  UInt_t Tsize;
  Char_t fname[256];
  for(Int_t m=0;m<4;m++){
    Tsize = 0;
    ROOT_phase[m]=-100;
    ROOT_beta[m]=-100;
    ROOT_delta[m]=-100;
    ROOT_gamma[m]=-100;
    ROOT_baseline[m] = -100;
    ROOT_qdc[m] = -100;
    beta[m]=fit_beta[m];
    gamma[m]= fit_gamma[m];
    vector <UInt_t>::iterator it;
    UInt_t max_position=0;
    vector <UInt_t> *trace;
    switch(m){
    case 0:
      if(trace_start1->size()!=0){
	it=max_element(trace_start1->begin(),trace_start1->end());
	max_position=distance(trace_start1->begin(),it);
        trace = trace_start1;
        Tsize = trace_start1->size();
      }
      break;
    case 1:
      if(trace_start2->size()!=0){
	it=max_element(trace_start2->begin(),trace_start2->end());
	max_position=distance(trace_start2->begin(),it);
        trace = trace_start2;
       Tsize = trace_start2->size();
      }
      break;
    case 2:
      if(trace_stop1->size()!=0){
	it=max_element(trace_stop1->begin(),trace_stop1->end());
	max_position=distance(trace_stop1->begin(),it);
        trace = trace_stop1;
        Tsize = trace_stop1->size();
      }
      break;
    case 3:
      if(trace_stop2->size()!=0){
	it=max_element(trace_stop2->begin(),trace_stop2->end());
	max_position=distance(trace_stop2->begin(),it);
        trace = trace_stop2;
        Tsize = trace_stop2->size();
      }
      break;
    default:
      break;
    }



    if(fTraces[m]->GetN()>0&&max_position>40&&max_position<100){
      std::pair <Double_t,Double_t> range((max_position-fit_limits[m].first)*factor,(max_position+fit_limits[m].second)*factor);
      //fit[m] =new TF1(fname,sipmfunc,range.first,range.second,6);
      fFits[m]->SetRange(range.first,range.second);
      ROOT_baseline[m] = CalcBaseline(trace,0,max_position-19);
      ROOT_qdc[m] = QDCcalc(trace,m,max_position,ROOT_baseline[m]);
      fFits[m]->SetParameters(range.first,ROOT_qdc[m]*0.7,beta[m],gamma[m],ROOT_baseline[m],delta[m]);
      fFits[m]->SetParNames("#phi","#alpha","#beta","#gamma","Baseline","#delta");
      fFits[m]->FixParameter(4,ROOT_baseline[m]);
      fFits[m]->FixParameter(1,ROOT_qdc[m]*0.5);
      fFits[m]->FixParameter(2,0.00701);
      fFits[m]->FixParameter(3,0.350);
      if(fFixParameters){
	fFits[m]->FixParameter(2,0.2);
        fFits[m]->FixParameter(3,0.2);
        }
//      fFits[m]->FixParameter(5,delta[m]);
      fFits[m]->FixParameter(5,4.0);
      TFitResultPtr status = fTraces[m]->Fit(fFits[m],"RNQSW");
      if(kDraw) status=fTraces[m]->Fit(fFits[m],"RS");
      else status = fTraces[m]->Fit(fFits[m],"RNQSW");
      fStatus[m] =status->Status(); 
       if(status->IsValid()){
 	ROOT_phase[m]=fFits[m]->GetParameter(0)-range.first+max_position;
 	ROOT_time[m]=ROOT_phase[m]*4.0+timestamp[m];
        }
	ROOT_beta[m]=fFits[m]->GetParameter(2);
 	ROOT_gamma[m]=fFits[m]->GetParameter(3);
 	ROOT_chisq[m]=fFits[m]->GetChisquare();
 	//ROOT_time[m]=fFits[m]->GetParameter(0)+timestamp[m];
 	ROOT_max[m]=trace->at(max_position);
        ROOT_delta[m]=fFits[m]->GetParameter(5);
      
      //cout<<"PAASS phase "<<phase[m]<<"\t ROOT phase "<<ROOT_phase[m]<<endl;

    }
  }
 return; 
}

Double_t fitTimingClass::QDCcalc(vector <UInt_t> *dTrace, int chan, UInt_t maxpos, Double_t baseline){
 double qdc=0;
 for (int i = maxpos-8; i < (Int_t)maxpos+20; i++){
  if(((double)dTrace->at(i)+(double)dTrace->at(i+1))/2.0>baseline){
   qdc += ((double)(dTrace->at(i)+dTrace->at(i+1))-baseline*2.0)*0.5; 
   }
  else continue;
  }
 return qdc;
}

Double_t fitTimingClass::CalcBaseline(vector <UInt_t> *dTrace, UInt_t initialpos,UInt_t finalpos){
  Double_t Baseline=0;
  Int_t count=0;
  for (int i =  initialpos; i < (int)finalpos; i++){
    Baseline+=dTrace->at(i);
    count++;
  } 
  Baseline/=count;

  return Baseline;
}

#endif // #ifdef fitTimingClass_cxx

