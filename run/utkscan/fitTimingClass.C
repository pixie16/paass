#define fitTimingClass_cxx
#include "fitTimingClass.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

void fitTimingClass::Loop(Long64_t nentries, const Char_t *filename)
{
//   In a ROOT session, you can do:
//      root> .L fitTimingClass.C
//      root> fitTimingClass t
//      root> t.GetEntry(12); // Fill t data members with entry number 12
//      root> t.Show();       // Show values of entry 12
//      root> t.Show(16);     // Read and show values of entry 16
//      root> t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch
   if (fChain == 0) return;

   
TFile *outputFile;
   if(!filename)
     outputFile=new TFile("Output.root","RECREATE");
   else
     outputFile=new TFile(filename,"RECREATE");
   TTree *outTree=new TTree("T","Output Tree");
   outTree->Branch("ROOT_phase[4]",&ROOT_phase,"phase[4]/D");
   outTree->Branch("ROOT_beta[4]",&ROOT_beta,"beta[4]/D");
   outTree->Branch("ROOT_gamma[4]",&ROOT_gamma,"gamma[4]/D");
   outTree->Branch("ROOT_baseline[4]",&ROOT_baseline,"baseline[4]/D");
   outTree->Branch("ROOT_delta[4]",&ROOT_delta,"delta[4]/D");
   outTree->Branch("ROOT_ChiSq[4]",&ROOT_chisq,"ChiSq[4]/D");
   outTree->Branch("ROOT_Status[4]",&fStatus,"Stat[4]/I");
   outTree->Branch("ROOT_qdc[4]",&ROOT_qdc,"qdc[4]/D");
   outTree->Branch("ROOT_time[4]",&ROOT_time,"time[4]/D");
   outTree->Branch("ROOT_max[4]",&ROOT_max,"max[4]/D");

  

if(nentries==-1)   nentries = fChain->GetEntriesFast();

   std::cout << nentries <<" entries in the ROOT file" << std::endl;

   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
   if(jentry%1000==0)
   cout<< jentry<<endl;
   //Fit(jentry,kTRUE);
   Fit(jentry,kFALSE); //No Draw
  
   //Plot(jentry,kTRUE); //No fix beta and gamma
   ////     cout<<jentry<<"Filling... "<<endl;
   outTree->Fill();
  }

   outTree->Write();
   outputFile->Close();
   cout<<endl;
}
