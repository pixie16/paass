#define traceClass_cxx
#include "traceClass.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

void traceClass::Loop(Long64_t nentries, const Char_t *filename)
{
//   In a ROOT session, you can do:
//      root> .L traceClass.C
//      root> traceClass t
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

   if (nentries<0) nentries = fChain->GetEntries();
   
  std::cout << "Looping " << nentries << " entries" << std::endl;

  char hname[200];
  const int ulimit = 65000;
  const int llimit = 5000;
  const int nTH = (ulimit-llimit)/5000;
//  std::cout << "Making " << nTH << " Histotrams" << std::endl;
  TH2F *hPulse[nTH];
  for (int iH=0; iH<nTH; iH++){
    sprintf(hname,"hP%dK",iH*5+5);
   // std::cout << hname << endl;
    hPulse[iH] = new TH2F(hname,hname,200,0,800,5000,0,ulimit+5000);
   }

   Long64_t nbytes = 0, nb = 0;

  std::vector<unsigned int> *trace;

   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      // if (Cut(ientry) < 0) continue;

  trace = trace_stop1; 
  UInt_t size = trace->size();
  if(size!=0){
   UInt_t maxval = *std::max_element(trace->begin(),trace->end());
   UInt_t minval = *std::min_element(trace->begin(),trace->end()); 
   int nY = maxval/5000;
   nY -= 1;
   if (nY>=nTH || nY<0) continue;
   for (UInt_t iB=0; iB<size; iB++){
    hPulse[nY]->Fill((Int_t)iB*4,trace->at(iB)-minval);
    }
   }
  }

 for (int iTH=0; iTH<nTH; iTH++){
  hPulse[iTH]->Write();
  }
 outputFile->Close();
}
