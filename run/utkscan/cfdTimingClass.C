#define cfdTimingClass_cxx
#include "cfdTimingClass.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

void cfdTimingClass::Loop(Long64_t nentries, const Char_t *filename)
{
//   In a ROOT session, you can do:
//      root> .L cfdTimingClass.C
//      root> cfdTimingClass t
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
   TTree *outTree = new TTree("T","Output Tree");
   outTree->Branch("phase[4]",&phase,"phase[4]/D");
   outTree->Branch("max[4]",&max,"max[4]/I");
   outTree->Branch("time[4]",&time,"time[4]/D");
   outTree->Branch("qdc[4]",&qdc,"qdc[4]/D");
   outTree->Branch("sbase[4]",&sbase,"sbase[4]/D");
   outTree->Branch("abase[4]",&abase,"abase[4]/D");


   if(nentries == -1){nentries = fChain->GetEntriesFast(); cout<<nentries<< " entries are being calculated" << endl;}

   Long64_t nbytes = 0, nb = 0;

   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      if (jentry%1000==0) cout << "On entry: " << jentry << endl;
      //nb = fChain->GetEntry(jentry);   nbytes += nb;
      // if (Cut(ientry) < 0) continue;
      //DigitalCFD(jentry);
      PolyCFD(jentry,0.5);
    outTree->Fill();
   }
   outTree->Write();
   outputFile->Close();
}
