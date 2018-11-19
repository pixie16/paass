//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Sat Nov 17 14:52:43 2018 by ROOT version 6.08/02
// from TTree T/Output Tree
// found on file: Output_Gridtest.root
//////////////////////////////////////////////////////////

#ifndef energyClass_h
#define energyClass_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include <iostream>
#include <math.h>
class energyClass {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   ULong64_t       event;
   Double_t        phase[4];
   Double_t        Pmax[4];
   Double_t        Fmax[4];
   Double_t        time[4];
   Double_t        Pixietime[4];
   Double_t        ToF;
   Double_t        ToF_E;
   Double_t        qdc[4];
   Double_t        startqdc;
   Double_t        stopqdc;
   Double_t        leadqdc[4];
   Double_t        sbase[4];
   Double_t        abase[4];
   Double_t        thresh[4];
   Double_t        uPoint[4];
   Double_t        lPoint[4];
   Double_t        uThresh[4];
   Double_t        lThresh[4];
   Double_t        tailqdc[4];
   Double_t        ratio[4];
   Double_t        slope[4];
   Double_t        dpoint[4];
   Double_t        ypos[2];
   Double_t        xpos[2];
   Int_t           nLeft;
   Int_t           nRight;
   Int_t           left_qdc[4];
   Int_t           right_qdc[4];
   Bool_t          k4fold;

   // List of branches
   TBranch        *b_event;   //!
   TBranch        *b_phase;   //!
   TBranch        *b_Pmax;   //!
   TBranch        *b_Fmax;   //!
   TBranch        *b_time;   //!
   TBranch        *b_Pixietime;   //!
   TBranch        *b_ToF;   //!
   TBranch        *b_ToF_E;   //!
   TBranch        *b_qdc;   //!
   TBranch        *b_startqdc;   //!
   TBranch        *b_stopqdc;   //!
   TBranch        *b_leadqdc;   //!
   TBranch        *b_sbase;   //!
   TBranch        *b_abase;   //!
   TBranch        *b_thresh;   //!
   TBranch        *b_uPoint;   //!
   TBranch        *b_lPoint;   //!
   TBranch        *b_uThresh;   //!
   TBranch        *b_lThresh;   //!
   TBranch        *b_tailqdc;   //!
   TBranch        *b_ratio;   //!
   TBranch        *b_slope;   //!
   TBranch        *b_dpoint;   //!
   TBranch        *b_ypos;   //!
   TBranch        *b_xpos;   //!
   TBranch        *b_nLeft;   //!
   TBranch        *b_nRight;   //!
   TBranch        *b_left_qdc;   //!
   TBranch        *b_right_qdc;   //!
   TBranch        *b_k4fold;   //!

   ////Public Data Members
   Double_t xcuts[7], ycuts[3];
   Double_t x_del = 6; // mm
   Double_t y_del = 12.7; // mm
   Double_t seg_L[4][8];
   Double_t Xpos, Ypos;
   Int_t Xseg, Yseg;
   Double_t En;

   energyClass(TTree *tree=0);
   virtual ~energyClass();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop(Long64_t nentries = -1, const Char_t *filename=NULL);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
   virtual void     LoadCuts();
   virtual void     SetLength(Double_t pLength=362);
   virtual void     CalculateEnergy(Long64_t entry = -1);

};

#endif

#ifdef energyClass_cxx
energyClass::energyClass(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("Output_Gridtest.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("Output_Gridtest.root");
      }
      f->GetObject("T",tree);

   }
   Init(tree);
}

energyClass::~energyClass()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t energyClass::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t energyClass::LoadTree(Long64_t entry)
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

void energyClass::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("event", &event, &b_event);
   fChain->SetBranchAddress("phase[4]", phase, &b_phase);
   fChain->SetBranchAddress("Pmax[4]", Pmax, &b_Pmax);
   fChain->SetBranchAddress("Fmax[4]", Fmax, &b_Fmax);
   fChain->SetBranchAddress("time[4]", time, &b_time);
   fChain->SetBranchAddress("Pixietime[4]", Pixietime, &b_Pixietime);
   fChain->SetBranchAddress("ToF", &ToF, &b_ToF);
   fChain->SetBranchAddress("ToF_E", &ToF_E, &b_ToF_E);
   fChain->SetBranchAddress("qdc[4]", qdc, &b_qdc);
   fChain->SetBranchAddress("startqdc", &startqdc, &b_startqdc);
   fChain->SetBranchAddress("stopqdc", &stopqdc, &b_stopqdc);
   fChain->SetBranchAddress("leadqdc[4]", leadqdc, &b_leadqdc);
   fChain->SetBranchAddress("sbase[4]", sbase, &b_sbase);
   fChain->SetBranchAddress("abase[4]", abase, &b_abase);
   fChain->SetBranchAddress("thresh[4]", thresh, &b_thresh);
   fChain->SetBranchAddress("uPoint[4]", uPoint, &b_uPoint);
   fChain->SetBranchAddress("lPoint[4]", lPoint, &b_lPoint);
   fChain->SetBranchAddress("uThresh[4]", uThresh, &b_uThresh);
   fChain->SetBranchAddress("lThresh[4]", lThresh, &b_lThresh);
   fChain->SetBranchAddress("tailqdc[4]", tailqdc, &b_tailqdc);
   fChain->SetBranchAddress("ratio[4]", ratio, &b_ratio);
   fChain->SetBranchAddress("slope[4]", slope, &b_slope);
   fChain->SetBranchAddress("dpoint[4]", dpoint, &b_dpoint);
   fChain->SetBranchAddress("ypos[2]", ypos, &b_ypos);
   fChain->SetBranchAddress("xpos[2]", xpos, &b_xpos);
   fChain->SetBranchAddress("nLeft", &nLeft, &b_nLeft);
   fChain->SetBranchAddress("nRight", &nRight, &b_nRight);
   fChain->SetBranchAddress("left_qdc[4]", left_qdc, &b_left_qdc);
   fChain->SetBranchAddress("right_qdc[4]", right_qdc, &b_right_qdc);
   fChain->SetBranchAddress("k4fold", &k4fold, &b_k4fold);
   Notify();
}

Bool_t energyClass::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void energyClass::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t energyClass::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}

void energyClass::SetLength(Double_t pLength){

 for(int k=0;k<4;k++){
  for(int j=0;j<8;j++){
   seg_L[k][j]= sqrt( pow((pLength+x_del*(double)(7.5-j)),2)+pow((y_del*(double)(k-1.5)),2));
   std::cout << seg_L[k][j] << " " ;
   }
  std::cout << endl;
  }

}

void energyClass::CalculateEnergy(Long64_t entry){
  GetEntry(entry);
//  SetLength(362);
  Xpos = (xpos[0]+xpos[1])/2.0;
  Ypos = (ypos[0]+ypos[1])/2.0;
  
       if(Xpos < xcuts[0])                    Xseg = 0;
  else if(Xpos > xcuts[0] && Xpos < xcuts[1]) Xseg = 1;
  else if(Xpos > xcuts[1] && Xpos < xcuts[2]) Xseg = 2;
  else if(Xpos > xcuts[2] && Xpos < xcuts[3]) Xseg = 3;
  else if(Xpos > xcuts[3] && Xpos < xcuts[4]) Xseg = 4;
  else if(Xpos > xcuts[4] && Xpos < xcuts[5]) Xseg = 5;
  else if(Xpos > xcuts[5] && Xpos < xcuts[6]) Xseg = 6;
  else if(Xpos > xcuts[6])                    Xseg = 7;   

       if(Ypos < ycuts[0])                    Yseg = 0;
  else if(Ypos > ycuts[0] && Ypos < ycuts[1]) Yseg = 1;
  else if(Ypos > ycuts[1] && Ypos < ycuts[2]) Yseg = 2;
  else if(Ypos > ycuts[2])                    Yseg = 3;   

 if( ToF>0) En = 0.5*(939)*pow((seg_L[Yseg][Xseg]/(ToF*1e-6)/3E8),2);
 else En = -9999;
 
 if (En > 1000) En = -9999;
// std::cout << "Neutron Energy: " << En << " MeV" << endl;
 return;
}

void energyClass::LoadCuts(){

 FILE *fin; fin = fopen("Grid.txt","r");
 for (int i=0; i<7; i++) fscanf(fin,"%lf", &xcuts[i]);
 for (int i=0; i<3; i++) fscanf(fin,"%lf", &ycuts[i]);

 fclose(fin);

// std::cout << ycuts[0] << " " << ycuts[1] << " " << ycuts[2] << endl;
 return;
}

#endif // #ifdef energyClass_cxx
