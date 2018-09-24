//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue May  8 15:18:06 2018 by ROOT version 6.08/02
// from TTree timing/
// found on file: EJ299_TeflonWrappedMylar_shortbar_1100V_Sr90_16b_001_CFD.root
//////////////////////////////////////////////////////////

#ifndef pspmtClass_h
#define pspmtClass_h

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
#include <TLine.h>

// Header file for the classes stored in the TTree if any.
#include "vector"
#include <vector>
#include <algorithm>
#include <utility>
#include <iterator> 

class pspmtClass {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Double_t        left_time_qdc;
   Double_t        left_time_amp;
   Double_t        left_time_snr;
   Double_t        left_time_abase;
   Double_t        left_time_sbase;
   UChar_t         left_time_id;
   Double_t        right_time_qdc;
   Double_t        right_time_amp;
   Double_t        right_time_snr;
   Double_t        right_time_abase;
   Double_t        right_time_sbase;
   UChar_t         right_time_id;
   Int_t           left_qdc[4];
   Int_t           right_qdc[4];
   Int_t           left_max[4];
   Int_t           right_max[4];
   UInt_t          nLeft;
   UInt_t          nRight;
   Double_t        leftTimeStamp;
   Double_t        rightTimeStamp;
   vector<unsigned int> *trace_left_dynode;
   vector<unsigned int> *trace_right_dynode;

   // List of branches
   TBranch        *b_left_time;   //!
   TBranch        *b_right_time;   //!
   TBranch        *b_left_qdc;   //!
   TBranch        *b_right_qdc;   //!
   TBranch        *b_left_max;   //!
   TBranch        *b_right_max;   //!
   TBranch        *b_nLeft;   //!
   TBranch        *b_nRight;   //!
   TBranch        *b_left_timeS;   //!
   TBranch        *b_right_timeS;   //!
   TBranch        *b_trace_left_dynode;   //!
   TBranch        *b_trace_right_dynode;   //!

   //Parameters
   Bool_t k4fold;
   Double_t fFraction;
   Double_t fSamplingRate;
   Int_t fDelay;
   Int_t fQDCwin;

   Double_t GetFraction(){return fFraction;}
   Double_t GetSamplingRate(){return fSamplingRate;}
   Int_t    GetCCDelay(){return fDelay;}
   Int_t    GetQDCwin(){return fQDCwin;}

   void SetFraction(Double_t aFraction){fFraction=aFraction;}
   void SetSamplingRate(Double_t aSamplingRate){fSamplingRate = aSamplingRate;}
   void SetCCDelay(Int_t CCdelay){fDelay = CCdelay;}   
   void SetQDCwin(Int_t QDCwin){fQDCwin = QDCwin;}

   ///////////// Variable to Save
   ULong64_t event;
   Double_t phase[2];
   Double_t Pmax[2];
   Double_t Fmax[2];
   Double_t qdc[2];
   Double_t time[2];
   Double_t Pixietime[2];
   Double_t ToF;
   Double_t sbase[2];
   Double_t abase[2];
   Double_t thresh[2];
   Double_t uPoint[2];
   Double_t lPoint[2];
   Double_t lThresh[2];
   Double_t uThresh[2];
   Double_t slope[2];
   Double_t tailqdc[2];
   Double_t ratio[2];
   Double_t leadqdc[2];
   Double_t dpoint[2];
   UInt_t   size[2];
   Bool_t   k2fold;
   Double_t ypos[2];
   Double_t xpos[2];
   ////////////////////////////////

   TGraphErrors *fTraces[2];
   TF1 *fpol3[2];
   TF1 *fpol2[2];
   TF1 *fpol1[2];

   pspmtClass(TTree *tree=0);
   virtual ~pspmtClass();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop(Long64_t nentries =-1,const Char_t *filename=NULL);
   virtual void     DumpTraces(int chan = 1);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
   virtual void     Plot(Long64_t entry = -1, Bool_t kDraw = kFALSE);
   virtual void     PolyCFD(Long64_t entry = -1);
   virtual void     PolyCFDDraw(Long64_t entry = -1, Double_t frac = 0.45, Int_t Chan = 0);
   virtual void     PolyScan(Long64_t nentries=1000, Int_t chan1=0, Int_t chan2=1);
   virtual void   QDCcalc(vector <UInt_t> *dTrace, int chan, Double_t cfdpos, Double_t baseline);
   virtual Double_t   CalcBaseline(vector <UInt_t> *dTrace, UInt_t initialpos,UInt_t finalpos);
   virtual void     CalcPosition(int chan=0);

};

#endif

#ifdef pspmtClass_cxx
pspmtClass::pspmtClass(TTree *tree) : fChain(0) 
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


   char fName[200];
   for (int iI=0;iI<2;iI++){
    sprintf(fName,"f3%d",iI);
    fTraces[iI] = new TGraphErrors();
    fpol3[iI] = new TF1(fName,"pol3",0,1);
    sprintf(fName,"f2%d",iI);
    fpol2[iI] = new TF1(fName,"pol2",0,1);
    sprintf(fName,"f1%d",iI);
    fpol1[iI] = new TF1(fName,"pol1",0,1);
    } 

}

pspmtClass::~pspmtClass()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();

   for (int iD=0;iD<2;iD++){
   delete fTraces[iD];
   delete fpol3[iD];
   delete fpol2[iD];
   delete fpol1[iD];
   }
}

Int_t pspmtClass::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   fChain->GetEntry(entry);
    size[0]=trace_left_dynode->size();
    size[1]=trace_right_dynode->size();
//    size[2]=trace_stop1->size();
//    size[3]=trace_stop2->size();
   return 1; 
}
Long64_t pspmtClass::LoadTree(Long64_t entry)
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

void pspmtClass::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   trace_left_dynode = 0;
   trace_right_dynode = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("left_time", &left_time_qdc, &b_left_time);
   fChain->SetBranchAddress("right_time", &right_time_qdc, &b_right_time);
   fChain->SetBranchAddress("left_qdc[4]", left_qdc, &b_left_qdc);
   fChain->SetBranchAddress("right_qdc[4]", right_qdc, &b_right_qdc);
   fChain->SetBranchAddress("left_max[4]", left_max, &b_left_max);
   fChain->SetBranchAddress("right_max[4]", right_max, &b_right_max);
   fChain->SetBranchAddress("nLeft", &nLeft, &b_nLeft);
   fChain->SetBranchAddress("nRight", &nRight, &b_nRight);
   fChain->SetBranchAddress("leftTimeStamp", &leftTimeStamp, &b_left_timeS);
   fChain->SetBranchAddress("rightTimeStamp", &rightTimeStamp, &b_right_timeS);
   fChain->SetBranchAddress("trace_left_dynode", &trace_left_dynode, &b_trace_left_dynode);
   fChain->SetBranchAddress("trace_right_dynode", &trace_right_dynode, &b_trace_right_dynode);
   SetSamplingRate(4.0);
   SetFraction(0.45);
   SetCCDelay(10);
   SetQDCwin(50);
   Notify();
}

Bool_t pspmtClass::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void pspmtClass::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t pspmtClass::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}

void pspmtClass::Plot(Long64_t entry,Bool_t kDraw){

  GetEntry(entry);

  for(Int_t j=0;j<2;j++){
    fTraces[j]->Set(0); 
  }

  if(size[0]!=0){
    for(UInt_t j=0;j<size[0];j++){
      fTraces[0]->SetPoint(j,j,trace_left_dynode->at(j));
      fTraces[0]->SetPointError(j,0,left_time_sbase);
    }
  }
  if(size[1]!=0){
    for(UInt_t j=0;j<size[1];j++){
     fTraces[1]->SetPoint(j,j,trace_right_dynode->at(j));
      fTraces[1]->SetPointError(j,0,right_time_sbase);
    }
  }
//  if(size2!=0){
//    for(UInt_t j=0;j<size2;j++){
//      fTraces[2]->SetPoint(j,j,trace_stop1->at(j));
//      fTraces[2]->SetPointError(j,0,stop1_sbase);
//    }
//  }
//  if(size3!=0){
//    for(UInt_t j=0;j<size3;j++){
//     fTraces[3]->SetPoint(j,j,trace_stop2->at(j));
//      fTraces[3]->SetPointError(j,0,stop2_sbase);
//    }
//  }
//

  if(size[0]==0&&size[1]==0) return ;
  else if(kDraw){
          TCanvas *c;
      if(!gPad){
	c=new TCanvas();
	c->Divide(1,2); 
      }
      else{
	//  c=(TCanvas*)gPad;
	c=gPad->GetCanvas();
	c->Clear();
	c->Divide(1,2);     
      }
      for(Int_t i=0;i<2;i++){
	c->cd(i+1);   
	if(fTraces[i]){
	  fTraces[i]->Draw("AL*");
//          fTraces[i]->GetXaxis()->SetRangeUser(40,60);
	}
	else
	  continue;
      }
    }
  return;
 }


void pspmtClass::PolyCFDDraw(Long64_t entry, Double_t frac, Int_t Chan){

// GetEntry(entry);
   Plot(entry, kFALSE);

   Double_t T_max[2] =   {left_time_amp,right_time_amp};
   Double_t T_qdc[2] =   {left_time_qdc,right_time_qdc};
   Double_t T_time[2] =  {leftTimeStamp,rightTimeStamp};
   Double_t T_sbase[2] = {left_time_sbase,right_time_sbase};
   Double_t T_abase[2] = {left_time_abase,right_time_abase};


//  TCanvas *c1 = new TCanvas();   
//  c1->Divide(2,2);
  TLine *l1;// = new TLine();
  TLine *l2;// = new TLine();
   std::pair <UInt_t,UInt_t> points;
  Double_t base;

  int m = Chan;  
  vector <UInt_t> *trace;
  vector <UInt_t>::iterator it;
  UInt_t max_position=0;
  switch(m){
  case 0:
    if(trace_left_dynode->size()!=0){
      it=max_element(trace_left_dynode->begin(),trace_left_dynode->end());
      max_position=distance(trace_left_dynode->begin(),it);
      trace = trace_left_dynode;
    }
    break;
  case 1:
    if(trace_right_dynode->size()!=0){
      it=max_element(trace_right_dynode->begin(),trace_right_dynode->end());
      max_position=distance(trace_right_dynode->begin(),it);
      trace = trace_right_dynode;
    }
    break;
//  case 2:
//    if(trace_stop1->size()!=0){
//      it=max_element(trace_stop1->begin(),trace_stop1->end());
//      max_position=distance(trace_stop1->begin(),it);
//      trace = trace_stop1;
//    }
//    break;
//  case 3:
//    if(trace_stop2->size()!=0){
//      it=max_element(trace_stop2->begin(),trace_stop2->end());
//      max_position=distance(trace_stop2->begin(),it);
//      trace = trace_stop2;
//    }
//    break;
  default:
    break;
  }

 if(fTraces[m]->GetN()>0){
   fTraces[m]->SetMarkerStyle(20);
   fTraces[m]->SetMarkerSize(0.7);
   fTraces[m]->Draw("AP");
   fTraces[m]->GetListOfFunctions()->Clear();
   l1 = new TLine();
   l2 = new TLine();
   std::pair <Double_t,Double_t> range((max_position-2),(max_position+2));   /// Set range for finding the absolute maximum around the peak
   fpol3[m]->SetRange(range.first,range.second);
   fpol3[m]->SetLineColor(kGreen);
   fTraces[m]->Fit(fpol3[m],"RQSW+");    // Fit 3rd order poly
   Fmax[m] = fpol3[m]->GetMaximum(range.first,range.second); //extract fit maximum
   Pmax[m] = trace->at(max_position);  //extract pixie maximum

   if (max_position>15) base= CalcBaseline(trace,0,max_position-20);
   else base = T_abase[m];

   // if(m==1)   thresh[m] = (Fmax[m]-base)*0.5+base; // used for Threshold scan as control (NEED TO REMOVE THIS FOR NORMAL POLYCFD) 
   // else thresh[m] = (Fmax[m]-base)*frac+base;   // calculate threshold based on maximum from fit 
   thresh[m] = (Fmax[m]-base)*frac+base;


   // find two points around threshold
   for (UInt_t ip = max_position; ip>1; ip--){
    if ((trace->at(ip)>=thresh[m])&&(trace->at(ip-1)<thresh[m])){
     points.first = ip-1; points.second =ip;}   //set pixie points around thresh
     lThresh[m] = trace->at(ip-1);
     uThresh[m] = trace->at(ip);

    }
//   fpol2[m]->SetRange(points.first,points.second+1);
//   fTraces[m]->Fit(fpol2[m],"RNQSW");
//   phase[m] = fpol2[m]->GetX(thresh[m],points.first,points.second+1); 
   fpol1[m]->SetRange(points.first,points.second);
   fpol1[m]->SetLineColor(kMagenta);
   fTraces[m]->Fit(fpol1[m],"RQSW+");  // fit 1st order poly
   phase[m] = fpol1[m]->GetX(thresh[m],points.first,points.second);  // Get high resolution phase from 1st order poly
   QDCcalc(trace,m,phase[m],base);
   fTraces[m]->GetXaxis()->SetRangeUser(30,100);
   l1->SetLineColor(kRed);
   l1->DrawLine(phase[m],base,phase[m],Fmax[m]); 
   l2->SetLineColor(kBlue);
   l2->DrawLine(phase[m]+fDelay,base,phase[m]+fDelay,Fmax[m]);
   }
   else cout << "No Trace to Plot. Try different entry or channel." << endl;
  
 return;


}

void pspmtClass::PolyCFD(Long64_t entry){
   k4fold = false;
// GetEntry(entry);
   Plot(entry, kFALSE);

   Double_t T_max[2] =   {left_time_amp,right_time_amp};
   Double_t T_qdc[2] =   {left_time_qdc,right_time_qdc};
   Double_t T_time[2] =  {leftTimeStamp,rightTimeStamp};
   Double_t T_sbase[2] = {left_time_sbase,right_time_sbase};
   Double_t T_abase[2] = {left_time_abase,right_time_abase};

   int nTraces = 0;
   std::pair <UInt_t,UInt_t> points;
  Double_t base;
 for (int m=0;m<2;m++){
  
  vector <UInt_t> *trace;
  vector <UInt_t>::iterator it;
  UInt_t max_position=0;
  switch(m){
  case 0:
    if(trace_left_dynode->size()!=0){
      it=max_element(trace_left_dynode->begin(),trace_left_dynode->end());
      max_position=distance(trace_left_dynode->begin(),it);
      trace = trace_left_dynode;
    }
    break;
  case 1:
    if(trace_right_dynode->size()!=0){
      it=max_element(trace_right_dynode->begin(),trace_right_dynode->end());
      max_position=distance(trace_right_dynode->begin(),it);
      trace = trace_right_dynode;
    }
    break;
//  case 2:
//    if(trace_stop1->size()!=0){
//      it=max_element(trace_stop1->begin(),trace_stop1->end());
//      max_position=distance(trace_stop1->begin(),it);
//      trace = trace_stop1;
//    }
//    break;
//  case 3:
//    if(trace_stop2->size()!=0){
//      it=max_element(trace_stop2->begin(),trace_stop2->end());
//      max_position=distance(trace_stop2->begin(),it);
//      trace = trace_stop2;
//    }
//    break;
  default:
    break;
  }

 if(fTraces[m]->GetN()>0){
//   if (m==1) {SetCCDelay(50);SetQDCwin(250);}
//   else {SetCCDelay(10);SetQDCwin(50);}
 
   nTraces++;
   std::pair <Double_t,Double_t> range((max_position-2),(max_position+2));   /// Set range for finding the absolute maximum around the peak
   fpol3[m]->SetRange(range.first,range.second);
   fTraces[m]->Fit(fpol3[m],"RNQSW");    // Fit 3rd order poly
   Fmax[m] = fpol3[m]->GetMaximum(range.first,range.second); //extract fit maximum
   Pmax[m] = trace->at(max_position);  //extract pixie maximum

   if (max_position>20) base= CalcBaseline(trace,0,max_position-19);
   else base = T_abase[m];

//   if(m==2)   thresh[m] = (Fmax[m]-base)*0.45+base; // used for Threshold scan as control (NEED TO REMOVE THIS FOR NORMAL POLYCFD) 
//   else thresh[m] = (Fmax[m]-base)*frac+base;   // calculate threshold based on maximum from fit 
   thresh[m] = (Fmax[m]-base)*fFraction+base;


   // find two points around threshold
   for (UInt_t ip = max_position; ip>1; ip--){
    if ((trace->at(ip)>=thresh[m])&&(trace->at(ip-1)<thresh[m])){
     points.first = ip-1; points.second =ip;}   //set pixie points around thresh
     lThresh[m] = trace->at(ip-1);
     uThresh[m] = trace->at(ip);
    }
//   fpol2[m]->SetRange(points.first,points.second+1);
//   fTraces[m]->Fit(fpol2[m],"RNQSW");
//   phase[m] = fpol2[m]->GetX(thresh[m],points.first,points.second+1); 
   fpol1[m]->SetRange(points.first,points.second);
   fTraces[m]->Fit(fpol1[m],"RNQSW");  // fit 1st order poly
   phase[m] = fpol1[m]->GetX(thresh[m],points.first,points.second);  // Get high resolution phase from 1st order poly
   time[m] = T_time[m]+fSamplingRate*phase[m];  // Calculate high res time from phase and latching time

   Pixietime[m] = T_time[m];
   sbase[m] = T_sbase[m];
   lPoint[m] = points.first;
   uPoint[m] = points.second;
  // slope[m] = fpol1[m]->GetParameter(1)/4.0;  //find slope of the 1st order poly fit
   CalcPosition(m);
   if (max_position > 20 && max_position < size[m]/2 && phase[m] > 20 && phase[m]<size[m]/2){ 
      dpoint[m] = trace->at(max_position+11);
      abase[m] = base;
      QDCcalc(trace,m,phase[m],base);
      ratio[m] = tailqdc[m]/qdc[m];
    }else{
          qdc[m] = -9999;
          tailqdc[m] = -9999;
	  leadqdc[m] = -9999;
          ratio[m] = -9999;
          abase[m] = -9999;
          dpoint[m] = -9999;
         }
   }else{
    phase[m]=-9999;
    time[m]=-9999;
    abase[m] = -9999;
    sbase[m] = -9999;
    lPoint[m] = -9999;
    uPoint[m] = -9999;
    slope[m] = -9999;
    Pmax[m] = -9999;
    Fmax[m] = -9999;
    qdc[m] = -9999;
    tailqdc[m] = -9999;
    leadqdc[m] = -9999;
    dpoint[m] = -9999;
    }
 }
 UInt_t *p; p = std::find (size, size+4, 0);
 if (p == size+4) k4fold = true;

 return;
}

void pspmtClass::QDCcalc(vector <UInt_t> *dTrace, int chan, Double_t cfdpos, Double_t baseline){
 qdc[chan]=0;
 tailqdc[chan]=0;
 leadqdc[chan]=0;
 int cfdposL = floor(cfdpos);
 int cfdposR = ceil(cfdpos);
 double tL = cfdpos-(double)cfdposL;
 double tR = (double)cfdposR-cfdpos;
 double partialT = (double)dTrace->at(cfdposR+fDelay)*tL+(double)dTrace->at(cfdposL+fDelay)*tR;
 tailqdc[chan] += (partialT+(double)dTrace->at(cfdposR+fDelay)-baseline*2.0)*tR*0.5;
// if (size[chan]-cfdposR<50){
 for (int i = cfdposR-5; i < cfdposR+fQDCwin; i++){
  if(((double)dTrace->at(i)+(double)dTrace->at(i+1))/2.0>baseline){
   qdc[chan] += ((double)(dTrace->at(i)+dTrace->at(i+1))-baseline*2.0)*0.5; 
   if (i>=cfdposR+fDelay) tailqdc[chan] += ((double)(dTrace->at(i)+dTrace->at(i+1))-baseline*2.0)*0.5;
   if (i<cfdposR+3) leadqdc[chan] += ((double)(dTrace->at(i)+dTrace->at(i+1))-baseline*2.0)*0.5;
   }
  else continue;
  }
// }
}

Double_t pspmtClass::CalcBaseline(vector <UInt_t> *dTrace, UInt_t initialpos,UInt_t finalpos){
  Double_t Baseline=0;
  Int_t count=0;
  for (int i =  initialpos; i < (int)finalpos; i++){
    Baseline+=dTrace->at(i);
    count++;
  } 
  Baseline/=count;

  return Baseline;
}

void pspmtClass::CalcPosition(int chan){
  
  switch(chan){
  case(0):
  ypos[chan] = (Double_t)(left_qdc[1]+left_qdc[2]-left_qdc[0]-left_qdc[3])/(left_qdc[1]+left_qdc[2]+left_qdc[0]+left_qdc[3]);
  xpos[chan] = (Double_t)(left_qdc[3]+left_qdc[2]-left_qdc[0]-left_qdc[1])/(left_qdc[1]+left_qdc[2]+left_qdc[0]+left_qdc[3]);
  break;
  case(1):
  ypos[chan] = (Double_t)(right_qdc[1]+right_qdc[2]-right_qdc[0]-right_qdc[3])/(right_qdc[1]+right_qdc[2]+right_qdc[0]+right_qdc[3]);
  xpos[chan] = (Double_t)(right_qdc[3]+right_qdc[2]-right_qdc[0]-right_qdc[1])/(right_qdc[1]+right_qdc[2]+right_qdc[0]+right_qdc[3]);
  break; 
  default:
  break;
   }
}


#endif // #ifdef pspmtClass_cxx
