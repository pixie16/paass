#define energyClass_cxx
#include "energyClass.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

void energyClass::Loop(Long64_t nentries, const Char_t *filename)
{
//   In a ROOT session, you can do:
//      root> .L energyClass.C
//      root> energyClass t
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

   TFile *outf;
   if(!filename)
     outf=new TFile("Output.root","RECREATE");
   else
     outf=new TFile(filename,"RECREATE");

   TTree *ETree = new TTree("nEnergy","Output_Tree");
   ETree->Branch("event",&event,"event/l");
   ETree->Branch("Pmax[4]",&Pmax,"Pmax[4]/D");
   ETree->Branch("Fmax[4]",&Fmax,"Fmax[4]/D");
   ETree->Branch("pCorr",&pCorr,"pCorr/D");
   ETree->Branch("distance",&distance,"distance/D");
   ETree->Branch("ToF",&ToF,"ToF/D");
   ETree->Branch("ToF_E",&ToF_E,"ToF_E/D");
   ETree->Branch("En",&En,"En/D");
   ETree->Branch("VandE",&VandE,"VandE/D");
   ETree->Branch("startqdc",&startqdc,"startqdc/D");
   ETree->Branch("stopqdc",&stopqdc,"stopqdc/D");
   ETree->Branch("ratio[4]",&ratio,"ratio[4]/D");
   ETree->Branch("ypos[2]",&ypos,"ypos[2]/D");
   ETree->Branch("xpos[2]",&xpos,"xpos[2]/D");
   ETree->Branch("Xpos",&Xpos,"Xpos/D");
   ETree->Branch("Ypos",&Ypos,"Ypos/D");
   ETree->Branch("nLeft",&nLeft,"nLeft/I");
   ETree->Branch("nRight",&nRight,"nRight/I");
   ETree->Branch("Xseg",&Xseg,"Xseg/I");
   ETree->Branch("Yseg",&Yseg,"Yseg/I");

   if (nentries<0) nentries = fChain->GetEntriesFast();

   LoadCuts();
//   SetLength(362.0);

   Long64_t nbytes = 0, nb = 0;
   cout << "Analyzing " << nentries << " events." << endl;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
//      Long64_t ientry = LoadTree(jentry);
      if (jentry%500==0) cout << "\rOn entry: "<< jentry << flush;
//      if (ientry < 0) break;
 
      CalculateEnergy(jentry);
      // if (Cut(ientry) < 0) continue;
      ETree->Fill();
  }
 cout << endl;
 ETree->Write();
 outf->Close();
}
