//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Wed Mar 21 14:46:51 2018 by ROOT version 6.08/02
// from TTree timing/
// found on file: HM_PMT_EJ200_5cm_Co60_Col_1000V_LoGain.root
//////////////////////////////////////////////////////////

#ifndef traceClass_h
#define traceClass_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "vector"

class traceClass {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Double_t        start1_qdc;
   Double_t        start1_time;
   Double_t        start1_snr;
   Double_t        start1_wtime;
   Double_t        start1_phase;
   Double_t        start1_abase;
   Double_t        start1_sbase;
   UChar_t         start1_id;
   Double_t        stop1_qdc;
   Double_t        stop1_time;
   Double_t        stop1_snr;
   Double_t        stop1_wtime;
   Double_t        stop1_phase;
   Double_t        stop1_abase;
   Double_t        stop1_sbase;
   UChar_t         stop1_id;
   Double_t        start2_qdc;
   Double_t        start2_time;
   Double_t        start2_snr;
   Double_t        start2_wtime;
   Double_t        start2_phase;
   Double_t        start2_abase;
   Double_t        start2_sbase;
   UChar_t         start2_id;
   Double_t        stop2_qdc;
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

   traceClass(TTree *tree=0);
   virtual ~traceClass();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef traceClass_cxx
traceClass::traceClass(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("HM_PMT_EJ200_5cm_Co60_Col_1000V_LoGain.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("HM_PMT_EJ200_5cm_Co60_Col_1000V_LoGain.root");
      }
      f->GetObject("timing",tree);

   }
   Init(tree);
}

traceClass::~traceClass()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t traceClass::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t traceClass::LoadTree(Long64_t entry)
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

void traceClass::Init(TTree *tree)
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

Bool_t traceClass::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void traceClass::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t traceClass::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef traceClass_cxx
