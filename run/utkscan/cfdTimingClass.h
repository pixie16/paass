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
#include <TLine.h>

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

   //// Public Class Members //////
   UInt_t size[4];
   vector < vector <Double_t> > filter;
   bool entryBool;
   Double_t fFraction;
   Double_t fSampling;
   Int_t fCCdelay;
   int RC1, RC2;
   ////////////////////////////////

   Double_t GetFraction(){return fFraction;}
   Double_t GetSamplingRate(){return fSampling;}
   Int_t    GetCCDelay(){return fCCdelay;}
   void     GetRC(){cout << RC1 << " " << RC2 << endl;}

   void SetFraction(Double_t aFraction){fFraction=aFraction;}
   void SetSamplingRate(Double_t aSamplingRate){fSampling = aSamplingRate;}
   void SetCCDelay(Int_t aCCdelay){fCCdelay = aCCdelay;}   
   void SetRC(Int_t aRC1, Int_t aRC2){RC1 = aRC1; RC2 = aRC2;}
   ///////////// Variable to Save
   ULong64_t event;
   Double_t phase[4];
   Double_t Pmax[4];
   Double_t Fmax[4];
   Double_t qdc[4];
   Double_t time[4];
   Double_t ToF;
   Double_t sbase[4];
   Double_t abase[4];
   Double_t thresh[4];
   Double_t uPoint[4];
   Double_t lPoint[4];
   Double_t lThresh[4];
   Double_t uThresh[4];
   Double_t slope[4];
   Double_t tailqdc[4];
   Double_t ratio[4];
   Double_t leadqdc[4];
   Double_t dpoint[4];
   Double_t Zcross[4];
   ////////////////////////////////


   TGraphErrors *fTraces[4];
   TGraphErrors *fFilters[4];
   TF1 *fpol3[4][2];
   TF1 *fpol2[4];
   TF1 *fpol1[4];

   cfdTimingClass(TTree *tree=0);
   virtual ~cfdTimingClass();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop(Long64_t nentries =-1,const Char_t *filename=NULL);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
   virtual void     Plot(Long64_t entry = -1, Bool_t kDraw = kFALSE);
   virtual void     DigitalCFD(Long64_t entry = -1, Double_t fraction=0.4, Int_t delay=2, Bool_t kDraw = kFALSE);
   virtual void     PolyCFD(Long64_t entry = -1, Double_t frac = 0.45);
   virtual void     PolyCFDDraw(Long64_t entry = -1, Double_t frac = 0.45, Int_t Chan = 0);
   virtual void     PolyScan(Long64_t nentries=1000, Int_t chan1=2, Int_t chan2=3);
   virtual void     QDCcalc(vector <UInt_t> *dTrace, int chan, Double_t cfdpos, Double_t baseline);
//   virtual Double_t   CalcQDC(vector <UInt_t> *dTrace, Double_t cfdpos, Double_t baseline);
//   virtual Double_t   CalcTRAPQDC(vector <UInt_t> *dTrace, Double_t cfdpos, Double_t baseline);
   virtual Double_t   CalcBaseline(vector <UInt_t> *dTrace, UInt_t initialpos,UInt_t finalpos);
//   virtual Double_t   CalcLeadQDC(vector <UInt_t> *dTrace, Double_t cfdpos, Double_t baseline,UInt_t maxpos);
//   virtual Double_t   CalcTailQDC(vector <UInt_t> *dTrace, Double_t cfdpos, Double_t baseline);
//   virtual Double_t   CalcTRAPTailQDC(vector <UInt_t> *dTrace, Double_t cfdpos, Double_t baseline);
   virtual Double_t   psdZeroCrossing(vector <Double_t> *dTrace, Int_t maxpos, int chan);
   virtual void    ZcrossFilter();
   virtual void    InitEntry(Long64_t Ientry=1);
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


   char fName[200];
   for (int iI=0;iI<4;iI++){
    sprintf(fName,"f3%d",iI);
    fTraces[iI] = new TGraphErrors();
    fFilters[iI] = new TGraphErrors();
    fpol3[iI][0] = new TF1(fName,"pol3",0,1);
    fpol3[iI][1] = new TF1(fName,"pol3",0,1);
    sprintf(fName,"f2%d",iI);
    fpol2[iI] = new TF1(fName,"pol2",0,1);
    sprintf(fName,"f1%d",iI);
    fpol1[iI] = new TF1(fName,"pol1",0,1);
    } 

}

cfdTimingClass::~cfdTimingClass()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();

   for (int iD=0;iD<4;iD++){
   delete fTraces[iD];
   delete fFilters[iD];
   delete fpol3[iD][0];
   delete fpol3[iD][1];
   delete fpol2[iD];
   delete fpol1[iD];
   }
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
   SetSamplingRate(4.0);
   SetCCDelay(60);
   SetRC(10,47);
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

void cfdTimingClass::InitEntry(Long64_t Ientry){
  GetEntry(Ientry);

  size[0]=trace_start1->size();
  size[1]=trace_start2->size();
  size[2]=trace_stop1->size();
  size[3]=trace_stop2->size();
  filter.clear();
  entryBool = true;
//  Double_t T_max[4] = {start1_amp,start2_amp,stop1_amp,stop2_amp};
//  Double_t T_qdc[4] = {start1_qdc,start2_qdc,stop1_qdc,stop2_qdc};
//  Double_t T_time[4] = {StartTimeStamp[0],StartTimeStamp[1],StopTimeStamp[0],StopTimeStamp[1]};
//  Double_t T_sbase[4] = {start1_sbase,start2_sbase,stop1_sbase,stop2_sbase};
//  Double_t T_abase[4] = {start1_abase,start2_abase,stop1_abase,stop2_abase};

}

void cfdTimingClass::ZcrossFilter(){
// Double_t T_abase[4] = {start1_abase,start2_abase,stop1_abase,stop2_abase};
// std::cout << start1_abase<< " " <<start2_abase<< " " <<stop1_abase<< " " <<stop2_abase;
 std::vector <unsigned int> *dTrace;
 std::vector <double> tempvec;
// TGraph *g1 = new TGraph();
// TGraph *g2 = new TGraph();
  double a1 = (RC1)/(RC1+fSampling);
  double a2 = (fSampling)/(RC2+fSampling);
  double co1 = a1*(1.0-a2)*(1.0-a2);
  double co2 = -1.0*a1*(1.0-a2);
  double co3 = 0.0;
  double co4 = 2.0*a2+a1;
  double co5 = -1.0*(a2*a2+2.0*a1*a2);
  double co6 = a1*a2*a2;

//  cout<<a1<<" "<<a2<<" "<<co1<<" "<<co2<<" "<<co3<<" "<<co4<<" "<<co5<<" "<<co6<<endl;

//  double a = (fSamplingRate)/(fSamplingRate+RC1);
//  double co1 = a*a*(1.0-a);
//  double co2 = -2.0*a*a;
//  double co3 = a*a*(1.0-a);
//  double co4 = 3.0*a;
//  double co5 = -3.0*a*a;
//  double co6 = a*a*a;

//  cout<<a<<" "<<co1<<" "<<co2<<" "<<co3<<" "<<co4<<" "<<co5<<" "<<co6<<endl;
 double baseline=0;

 for(int m=0; m<4; m++){
//  if (filter[m].size()!=0) filter[m].clear();
  double y_n=0.0, y_n1=0.0, y_n2=0.0, y_n3=0.0;
  switch(m){
      case 0:
        dTrace = trace_start1;
        break;
      case 1:
        dTrace = trace_start2;
        break;
      case 2:
        dTrace = trace_stop1;
        break;
      case 3:
        dTrace = trace_stop2;
        break;
      default:
        break;
      }
    if(size[m]>100) baseline = CalcBaseline(dTrace,5,30);
  for(Int_t i=0;i<(Int_t)size[m];i++){ 
    switch(i){
     case 0:
      y_n = co1*(dTrace->at(i)-baseline);
//      y_n = co1*abs(dTrace->at(i)-baseline);
      break;
     case 1:
      y_n = co1*(dTrace->at(i)-baseline)+co2*(dTrace->at(i-1)-baseline)+co4*y_n1;
//      y_n = co1*abs(dTrace->at(i)-baseline)+co2*abs(dTrace->at(i-1)-baseline)+co3*y_n1;
      break;
     case 2:
      y_n = co1*(dTrace->at(i)-baseline)+co2*(dTrace->at(i-1)-baseline)+co3*(dTrace->at(i-2)-baseline)+co4*y_n1+co5*y_n2;
//      y_n = co1*abs(dTrace->at(i)-baseline)+co2*abs(dTrace->at(i-1)-baseline)+co3*y_n1+co4*y_n2;
      break;
     default:
      y_n = co1*(dTrace->at(i)-baseline)+co2*(dTrace->at(i-1)-baseline)+co3*(dTrace->at(i-2)-baseline)+co4*y_n1+co5*y_n2+co6*y_n3;
//      y_n = co1*abs(dTrace->at(i)-baseline)+co2*abs(dTrace->at(i-1)-baseline)+co3*y_n1+co4*y_n2+co5*y_n3;
      break;
    }
//    std::cout << dTrace->at(i) << " " << baseline << " " << y_n << " " << y_n1 << " " << y_n2 << " " << y_n3 << endl;
    tempvec.push_back(y_n);    
//    if (m==2) {g1->SetPoint(i,i,y_n); g2->SetPoint(i,i,dTrace->at(i)-baseline);}
    y_n3 = y_n2;
    y_n2 = y_n1;
    y_n1 = y_n;
  }
  filter.push_back(tempvec);
  tempvec.clear();
 }
// g1->Draw("AL*");
// g2->Draw("same");
}

void cfdTimingClass::Plot(Long64_t entry,Bool_t kDraw){

//  if (!entryBool) InitEntry(entry);
  
//  const UInt_t size0=trace_start1->size();
//  const UInt_t size1=trace_start2->size();
//  const UInt_t size2=trace_stop1->size();
//  const UInt_t size3=trace_stop2->size();

  /* cout<<"Size "<<size0<<endl;  */
  /* cout<<"Size "<<size1<<endl;  */
  /* cout<<"Size "<<size2<<endl;  */
  /* cout<<"Size "<<size3<<endl;  */

  for(Int_t j=0;j<4;j++){
    fTraces[j]->Set(0); 
    fFilters[j]->Set(0);
  }

  if(size[0]!=0){
    for(UInt_t j=0;j<size[0];j++){
      fTraces[0]->SetPoint(j,j,trace_start1->at(j));
      fTraces[0]->SetPointError(j,0,start1_sbase);
      fFilters[0]->SetPoint(j,j,filter[0].at(j));
    }
  }
  if(size[1]!=0){
    for(UInt_t j=0;j<size[1];j++){
      fTraces[1]->SetPoint(j,j,trace_start2->at(j));
      fTraces[1]->SetPointError(j,0,start2_sbase);
      fFilters[1]->SetPoint(j,j,filter[1].at(j));
    }
  }
  if(size[2]!=0){
    for(UInt_t j=0;j<size[2];j++){
      fTraces[2]->SetPoint(j,j,trace_stop1->at(j));
      fTraces[2]->SetPointError(j,0,stop1_sbase);
      fFilters[2]->SetPoint(j,j,filter[2].at(j));
    }
  }
  if(size[3]!=0){
    for(UInt_t j=0;j<size[3];j++){
      fTraces[3]->SetPoint(j,j,trace_stop2->at(j));
      fTraces[3]->SetPointError(j,0,stop2_sbase);
      fFilters[3]->SetPoint(j,j,filter[3].at(j));
    }
  }


  if(size[0]==0&&size[1]==0&&size[2]==0&&size[3]==0) return;
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
//          fTraces[i]->GetXaxis()->SetRangeUser(40,60);
	}
	else
	  continue;
      }
    }
  return;
 }


void cfdTimingClass::PolyCFDDraw(Long64_t entry, Double_t frac, Int_t Chan){
//   Plot(entry, kFALSE);

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
    if(trace_start1->size()!=0){
      it=max_element(trace_start1->begin(),trace_start1->end());
      max_position=distance(trace_start1->begin(),it);
      trace = trace_start1;
    }
    break;
  case 1:
    if(trace_start2->size()!=0){
      it=max_element(trace_start2->begin(),trace_start2->end());
      max_position=distance(trace_start2->begin(),it);
      trace = trace_start2;
    }
    break;
  case 2:
    if(trace_stop1->size()!=0){
      it=max_element(trace_stop1->begin(),trace_stop1->end());
      max_position=distance(trace_stop1->begin(),it);
      trace = trace_stop1;
    }
    break;
  case 3:
    if(trace_stop2->size()!=0){
      it=max_element(trace_stop2->begin(),trace_stop2->end());
      max_position=distance(trace_stop2->begin(),it);
      trace = trace_stop2;
    }
    break;
  default:
    break;
  }

 if(fTraces[m]->GetN()>0){
   fTraces[m]->SetMarkerStyle(20);
   fTraces[m]->SetMarkerSize(0.7);
   fTraces[m]->Draw("AP");
   l1 = new TLine();
   l2 = new TLine();
   std::pair <Double_t,Double_t> range((max_position-2),(max_position+2));   /// Set range for finding the absolute maximum around the peak
   fpol3[m][0]->SetRange(range.first,range.second);
   fpol3[m][0]->SetLineColor(kGreen);
   fTraces[m]->Fit(fpol3[m][0],"RQSW+");    // Fit 3rd order poly
   Fmax[m] = fpol3[m][0]->GetMaximum(range.first,range.second); //extract fit maximum
   Pmax[m] = trace->at(max_position);  //extract pixie maximum

   if (max_position>40/fSampling) base= CalcBaseline(trace,0,max_position-40/fSampling);
   else base = T_abase[m];

//   if(m==2)   thresh[m] = (Fmax[m]-base)*0.5+base; // used for Threshold scan as control (NEED TO REMOVE THIS FOR NORMAL POLYCFD) 
//   else thresh[m] = (Fmax[m]-base)*frac+base;   // calculate threshold based on maximum from fit 
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
   fTraces[m]->GetXaxis()->SetRangeUser(30,100);
   l1->SetLineColor(kRed);
   l1->DrawLine(phase[m],base,phase[m],Fmax[m]); 
   l2->SetLineColor(kBlue);
   l2->DrawLine(phase[m]+fCCdelay/fSampling,base,phase[m]+fCCdelay/fSampling,Fmax[m]);
   }
   else cout << "No Trace to Plot. Try different entry or channel." << endl;
 return;
}

void cfdTimingClass::PolyCFD(Long64_t entry, Double_t frac){

// GetEntry(entry);
   Plot(entry, kFALSE);

//   Double_t T_max[4] = {start1_amp,start2_amp,stop1_amp,stop2_amp};
//   Double_t T_qdc[4] = {start1_qdc,start2_qdc,stop1_qdc,stop2_qdc};
   Double_t T_time[4] = {StartTimeStamp[0],StartTimeStamp[1],StopTimeStamp[0],StopTimeStamp[1]};
   Double_t T_sbase[4] = {start1_sbase,start2_sbase,stop1_sbase,stop2_sbase};
   Double_t T_abase[4] = {start1_abase,start2_abase,stop1_abase,stop2_abase};

   // UInt_t size[4];
//    size[0]=trace_start1->size();
//    size[1]=trace_start2->size();
//    size[2]=trace_stop1->size();
//    size[3]=trace_stop2->size();

//  TCanvas *c1 = new TCanvas();   
//  c1->Divide(2,2);
  std::pair <Int_t,Int_t> points;
  Double_t base;
 for (int m=0;m<4;m++){
  
  vector <UInt_t> *trace;
  vector <UInt_t>::iterator it;
  vector <double>::iterator it2;
  UInt_t max_position=0, filter_max=0;

  switch(m){
  case 0:
    if(size[m]!=0){
      it=max_element(trace_start1->begin(),trace_start1->end());
      max_position=distance(trace_start1->begin(),it);
      trace = trace_start1;
      it2=max_element((filter[m]).begin(),(filter[m]).end());
      filter_max=distance((filter[m]).begin(),it2);
    }
    break;
  case 1:
    if(size[m]!=0){
      it=max_element(trace_start2->begin(),trace_start2->end());
      max_position=distance(trace_start2->begin(),it);
      trace = trace_start2;
      it2=max_element(filter[m].begin(),filter[m].end());
      filter_max=distance(filter[m].begin(),it2);
    }
    break;
  case 2:
    if(size[m]!=0){
      it=max_element(trace_stop1->begin(),trace_stop1->end());
      max_position=distance(trace_stop1->begin(),it);
      trace = trace_stop1;
      it2=max_element(filter[m].begin(),filter[m].end());
      filter_max=distance(filter[m].begin(),it2);
    }
    break;
  case 3:
    if(size[m]!=0){
      it=max_element(trace_stop2->begin(),trace_stop2->end());
      max_position=distance(trace_stop2->begin(),it);
      trace = trace_stop2;
      it2=max_element(filter[m].begin(),filter[m].end());
      filter_max=distance(filter[m].begin(),it2);
    }
    break;
  default:
    break;
  }

 if(fTraces[m]->GetN()>0){
   std::pair <Double_t,Double_t> range((max_position-2),(max_position+2));   /// Set range for finding the absolute maximum around the peak
   fpol3[m][0]->SetRange(range.first,range.second);
   fTraces[m]->Fit(fpol3[m][0],"RNQSW");    // Fit 3rd order poly
   Fmax[m] = fpol3[m][0]->GetMaximum(range.first,range.second); //extract fit maximum
   Pmax[m] = trace->at(max_position);  //extract pixie maximum

   if(max_position>40/fSampling) base= CalcBaseline(trace,0,max_position-40/fSampling);
   else base = T_abase[m];

//   if(m==2)   thresh[m] = (Fmax[m]-base)*0.5+base; // used for Threshold scan as control (NEED TO REMOVE THIS FOR NORMAL POLYCFD) 
//   else thresh[m] = (Fmax[m]-base)*frac+base;   // calculate threshold based on maximum from fit 
   thresh[m] = (Fmax[m]-base)*frac+base;

   //// Find maximum of bipolar filtered pulse for zero crossing PSD
//   std::pair <Double_t, Double_t> f_range((filter_max-2,filter_max-2));
//   fpol3[m][1]->SetRange(f_range.first,f_range.second);
//   fFilters[m]->Fit(fpol3[m][1],"RNQSW");    // Fit 3rd order poly
//   double FilterMax = fpol3[m][1]->GetMaximum(range.first,range.second);
//   double FilterTH = FilterMax*0.2;

   // find two points around threshold
   for (Int_t ip = max_position; ip>1; ip--){
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
   time[m] = T_time[m]+fSampling*phase[m];  // Calculate high res time from phase and latching time

   sbase[m] = T_sbase[m];
   lPoint[m] = points.first;
   uPoint[m] = points.second;
  // slope[m] = fpol1[m]->GetParameter(1)/4.0;  //find slope of the 1st order poly fit

   if (max_position > 40/fSampling && max_position < 400/fSampling && phase[m] > 150/fSampling){ 
//      Double_t base= CalcBaseline(trace,0,max_position-10);
      dpoint[m] = trace->at(max_position+32/fSampling);
      abase[m] = base;
      QDCcalc(trace,m,max_position,base);
//     qdc[m] = CalcQDC(trace,phase[m],base);
//      tailqdc[m] = CalcTailQDC(trace,phase[m],base);
//      leadqdc[m] = CalcLeadQDC(trace,phase[m],base,max_position);
      ratio[m] =  tailqdc[m]/qdc[m];
//        ratio[m] = leadqdc[m]/qdc[m]; 
//      ratio[m] =  tailqdc[m]/CalcLeadQDC(trace,max_position,base);
      Zcross[m] = psdZeroCrossing(&(filter[m]),filter_max,m);
      Zcross[m] -= phase[m];
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
//    abase[m] = T_abase[m];
//    abase[m] = base;
//    sbase[m] = T_sbase[m];
//    lPoint[m] = points.first;
//    uPoint[m] = points.second;
//    slope[m] = fpol1[m]->GetParameter(1)/4.0;
//    lThresh[m] = trace->at(points.first);
//    uThresh[m] = trace->at(points.second);
 }
 //   cout << "Chan1 time phase: " << time[2] << " " << phase[2] << "\nChan2 time phase: " << time[3] << " " << phase[3] << endl;


 return;
}

void cfdTimingClass::QDCcalc(vector <UInt_t> *dTrace, int chan, Double_t cfdpos, Double_t baseline){
 qdc[chan]=0;
 tailqdc[chan]=0;
 int cfdposL = floor(cfdpos);
 int cfdposR = ceil(cfdpos);
 double tL = cfdpos-(double)cfdposL;
 double tR = cfdpos-(double)cfdposR;
 double partialT = ((double)dTrace->at(cfdposR)*tR+(double)dTrace->at(cfdposR-1))*tR;
 tailqdc[chan] += (partialT+(double)dTrace->at(cfdposR))*tR/2;
 for (int i = cfdposR-20/fSampling; (i<cfdposR+150/fSampling)||(i==size[chan]-5); i++){
  if((double)dTrace->at(i)>baseline){
   qdc[chan] += ((double)(dTrace->at(i))-baseline); 
   if (i>cfdposR+fCCdelay/fSampling) tailqdc[chan] += ((double)(dTrace->at(i))-baseline);
   }
  else continue;
 }
}

Double_t cfdTimingClass::psdZeroCrossing(vector <Double_t> *dTrace, Int_t maxpos, int chan){
  std::pair <Int_t, Int_t> Zpoints;
  Double_t y1=0, y2=0;
 for (Int_t i=maxpos+1; i<(Int_t)size[chan]-5; i++){
    if ((dTrace->at(i)>=0)&&(dTrace->at(i+1)<0)){     //find pixie points around thresh
       Zpoints.first = i; Zpoints.second =i+1;
       y1 = (Double_t)dTrace->at(i);
       y2 = (Double_t)dTrace->at(i+1);
       break;
    }else if (i == (Int_t)size[chan]-10) {Zpoints = std::make_pair(-9999,-9999); break;}
 }
 if(Zpoints.first != -9999){
//  UInt_t y1 = dTrace->at(Zpoints.first);
//  UInt_t y2 = dTrace->at(Zpoints.second);
//    std::cout << chan << " " << y1 << " " << y2 << endl; 
 Double_t tf = (0.0-y1)*(1/(y2-y1))+Zpoints.first;
 // return tf - phase[chan];
  return tf;
  }
 else{ return -9999;}
}

Double_t cfdTimingClass::CalcBaseline(vector <UInt_t> *dTrace, UInt_t initialpos,UInt_t finalpos)
{

  Double_t Baseline=0;
  Int_t count=0;
  for (int i =  initialpos; i < (int)finalpos; i++){
    Baseline+=dTrace->at(i);
    count++;
  } 

  Baseline/=count;

  return Baseline;
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

    Pmax[iT] = T_max[iT];
    qdc[iT] = T_qdc[iT];
    abase[iT] = T_abase[iT];
    sbase[iT] = T_sbase[iT];
    time[iT] = T_time[iT]+phase[iT]*fSampling;

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
#endif // #ifdef cfdTimingClass_cxx
