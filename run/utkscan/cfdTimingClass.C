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
   outTree->Branch("event",&event,"event/l");
   outTree->Branch("phase[4]",&phase,"phase[4]/D");
   outTree->Branch("Pmax[4]",&Pmax,"Pmax[4]/D");
   outTree->Branch("Fmax[4]",&Fmax,"Fmax[4]/D");
   outTree->Branch("time[4]",&time,"time[4]/D");
   outTree->Branch("ToF",&ToF,"ToF/D");
   outTree->Branch("Xpos",&x_pos,"Xpos/D");
   outTree->Branch("Ypos",&y_pos,"Ypos/D");
   outTree->Branch("qdc[4]",&qdc,"qdc[4]/D");
   outTree->Branch("sbase[4]",&sbase,"sbase[4]/D");
   outTree->Branch("abase[4]",&abase,"abase[4]/D");
   outTree->Branch("thresh[4]",&thresh,"thresh[4]/D");
   outTree->Branch("uPoint[4]",&uPoint,"uPoint[4]/D");
   outTree->Branch("lPoint[4]",&lPoint,"lPoint[4]/D");
   outTree->Branch("uThresh[4]",&uThresh,"uThresh[4]/D");
   outTree->Branch("lThresh[4]",&lThresh,"lThresh[4]/D");
   outTree->Branch("tailqdc[4]",&tailqdc,"tailqdc[4]/D");
   outTree->Branch("ratio[4]",&ratio,"ratio[4]/D");
   outTree->Branch("slope[4]",&slope,"slope[4]/D");
   outTree->Branch("dpoint[4]",&dpoint,"dpoint[4]/D");
   outTree->Branch("Zcross[4]",&Zcross,"Zcross[4]/D");

//   outTree->Branch("points",&points);
   
   if(nentries == -1){nentries = fChain->GetEntriesFast(); cout<<nentries<< " entries are being calculated" << endl;}

   Long64_t nbytes = 0, nb = 0;

   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      event = jentry;
      if (ientry < 0) break;
      if (jentry%10000==0) cout << "\rOn entry: "<< jentry << flush;
      //nb = fChain->GetEntry(jentry);   nbytes += nb;
      // if (Cut(ientry) < 0) continue;
      //DigitalCFD(jentry);
      InitEntry(jentry);
      ZcrossFilter();   //Apply ZeroCross filter with time constants (RC1,RC2)
      Plot(jentry, kFALSE);  //Fill graphs for polyCFD analysis
      PolyCFD(jentry,0.45);    //Find CFD timing for jth entry with given fraction

      if (phase[0]>0&&phase[1]>0&&phase[2]>0&&phase[3]>0) ToF = (time[2]+time[3])/2.0-(time[0]+time[1])/2.0;  //Calculate ToF if 4 signals exist
      else ToF = -9999;

    outTree->Fill();
   }
   cout<<endl;
   outTree->Write();
   outputFile->Close();
}

void cfdTimingClass::PolyScan(Long64_t nentries, Int_t chan1, Int_t chan2){

  double hstep = 0.01;
  const int nsteps = 9/hstep;
  TH2F *h2[nsteps];
  TH1F *h1[nsteps];
  if (fChain == 0) return;
  
  if(nentries == -1){nentries = fChain->GetEntriesFast(); cout<<nentries<< " entries are being calculated" << endl;}

  TFile *oF = new TFile("PolyCFDScan.root","RECREATE");

  double frc;
  char hname[200];
  TF1 *f1 = new TF1("f1","gaus",0,1);
  double fMean, fRes;

  std::cout << "Running Scan" << endl;
  for (int iFrc=1; iFrc<90; iFrc++){   //cfd fraction scan
   frc = (double)iFrc*hstep;
   sprintf(hname,"h2_frac%d",iFrc);
   h2[iFrc-1] = new TH2F(hname,hname,1000,15000,300000,1000,-30,30);
   sprintf(hname,"h1_frac%d",iFrc);
   h1[iFrc-1] = new TH1F(hname,hname,1000,-30,30);
   for (int jentry=0; jentry<nentries; jentry++){
    
    PolyCFD(jentry,frc);   //extract timing for given cfd fraction
    if (phase[chan1]>0 && phase[chan2]>0){
    h1[iFrc-1]->Fill(time[chan1]-time[chan2]);
    h2[iFrc-1]->Fill(qdc[chan1],time[chan1]-time[chan2]);    //The QDC channel needs to be whichever channel is fixed to keep consistent QDC calculation w/o dependence on CFD phase
     }
//    cout << "Chan1 time phase: " << time[chan1] << " " << phase[chan1] << "\nChan2 time phase: " << time[chan2] << " " << phase[chan2] << endl;
    }

    double mean = h1[iFrc-1]->GetMean(1);
    f1->SetRange(mean-1.5,mean+1.5);
    h1[iFrc-1]->Fit(f1,"RQNSW");
    fMean = f1->GetParameter(1);
    fRes = (f1->GetParameter(2))*2.35;
   
    cout << iFrc << " " << frc <<  " " << fMean << " " << fRes << endl;
    h1[iFrc-1]->Write();
    h2[iFrc-1]->Write();
   }

  oF->Close();
}
