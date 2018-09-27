#define pspmtClass_cxx
#include "pspmtClass.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TEntryList.h>

void pspmtClass::Loop(Long64_t nentries, const Char_t *filename)
{
//   In a ROOT session, you can do:
//      root> .L pspmtClass.C
//      root> TTree *T = PSPMT; pspmtClass t(T)
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
   outTree->Branch("Pixietime[4]",&Pixietime,"Pixietime[4]/D");
   outTree->Branch("ToF",&ToF,"ToF/D");
   outTree->Branch("qdc[4]",&qdc,"qdc[4]/D");
   outTree->Branch("leadqdc[4]",&leadqdc,"leadqdc[4]/D");
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
   outTree->Branch("ypos[2]",&ypos,"ypos[2]/D");
   outTree->Branch("xpos[2]",&xpos,"xpos[2]/D");
   outTree->Branch("nLeft",&nLeft,"nLeft/I");
   outTree->Branch("nRight",&nRight,"nRight/I");
   outTree->Branch("left_qdc[4]",&left_qdc,"left_qdc[4]/I");
   outTree->Branch("right_qdc[4]",&right_qdc,"right_qdc[4]/I");
   outTree->Branch("k4fold",&k4fold,"k4fold/O");

//   outTree->Branch("points",&points);
   
   if(nentries == -1){nentries = fChain->GetEntriesFast(); cout<<nentries<< " entries are being calculated" << endl;}

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      if (jentry%100==0) cout << "\rOn entry: "<< jentry << flush;
//      nb = fChain->GetEntry(jentry);   nbytes += nb;
      // if (Cut(ientry) < 0) continue;
      PolyCFD(jentry);
      event=jentry;
    outTree->Fill();
   }
   cout<<endl;
   outTree->Write();
   outputFile->Close();
}
void pspmtClass::PolyScan(Long64_t nentries, Int_t chan1, Int_t chan2){

  double hstep = 0.01;
  const int nsteps = 9/hstep;
  TH2F *h2[nsteps];
  TH1F *h1[nsteps];
  if (fChain == 0) return;
  
  if(nentries == -1){nentries = fChain->GetEntriesFast(); cout<<nentries<< " entries are being calculated" << endl;}

  TFile *oF = new TFile("PolyCFDScan.root","RECREATE");

  FILE *fOut;
  fOut = fopen("PolyCFDResScan.txt","w");
  double frc;
  char hname[200];
  TF1 *f1 = new TF1("f1","gaus",0,1);
  double fMean, fRes;

  std::cout << "Running Scan" << endl;
  for (int iFrc=1; iFrc<90; iFrc++){   //cfd fraction scan
   frc = (double)iFrc*hstep;
   sprintf(hname,"h2_frac%d",iFrc);
   h2[iFrc-1] = new TH2F(hname,hname,1000,20000,500000,1000,-40,40);
   sprintf(hname,"h1_frac%d",iFrc);
   h1[iFrc-1] = new TH1F(hname,hname,1000,-30,30);
   for (int jentry=0; jentry<nentries; jentry++){
   SetFraction(frc); 
    PolyCFD(jentry);   //extract timing for given cfd fraction
    // if (phase[chan1]>20 && phase[chan2]>20 && (qdc[2]+qdc[3])/2>50000){
    if (phase[chan1]>20 && phase[chan2]>20 && (qdc[chan1]+qdc[chan2])/2>50000){
    h1[iFrc-1]->Fill(time[chan1]-time[chan2]);
    h2[iFrc-1]->Fill((qdc[chan1]+qdc[chan2])/2.0,time[chan1]-time[chan2]);    //The QDC channel needs to be whichever channel is fixed to keep consistent QDC calculation w/o dependence on CFD phase
     }
//    cout << "Chan1 time phase: " << time[chan1] << " " << phase[chan1] << "\nChan2 time phase: " << time[chan2] << " " << phase[chan2] << endl;
    }

    double mean = h1[iFrc-1]->GetMean(1);
    f1->SetRange(mean-1.5,mean+1.5);
    h1[iFrc-1]->Fit(f1,"RQNSW");
    fMean = f1->GetParameter(1);
    fRes = (f1->GetParameter(2))*2.35;
   
    cout << iFrc << " " << frc <<  " " << fMean << " " << fRes << endl;
    fprintf(fOut,"%d %f %f %f \n",iFrc,frc,fMean,fRes);
    h1[iFrc-1]->Write();
    h2[iFrc-1]->Write();
   }
  fclose(fOut);
  oF->Close();
}

void pspmtClass::DumpTraces(int chan){

TFile *listfile = new TFile("EntryListFile.root","READ");

TEntryList *nlist = (TEntryList*)listfile->Get("neutronlist");
TEntryList *glist = (TEntryList*)listfile->Get("gammalist");

int nN = nlist->GetN();
int gN = glist->GetN();
cout << "neutrons: " << nN << ", gammas: " << gN << endl;
int totN = nN + gN;

FILE *f1 = fopen("neutronTraces.dat","w");
FILE *f2 = fopen("gammaTraces.dat","w");
Long64_t entry;

for (int i=0; i<totN; i++){
 if (i<nN){
  entry = nlist->GetEntry(i);
  GetEntry(entry);
  for (int k=0; k<(int)size[chan]; k++){
   double val = (trace_right_dynode->at(k)-6600.)/65355.0;
   fprintf(f1,"%e, %f, ",(double)k*4e-9,val);
   }
  if (i>=1000) i=nN;
  }
  else if(i>=nN&&i<totN){
  entry = glist->GetEntry(i-nN);
  GetEntry(entry);
  for (int k=0; k<(int)size[chan]; k++){
   double val = (trace_right_dynode->at(k)-6600.)/65355.0;
   fprintf(f2,"%e, %f, ",(double)k*4e-9,val);
   }
   if (i>=2000) break;
  }
 }
 fclose(f1);
 fclose(f2);
 listfile->Close();
}

