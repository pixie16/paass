#include <iostream>
#include <TProfile.h>
#include <TCanvas.h>
#include <stdio.h>

using namespace std;

void NormalizePulses(){

const int nFiles = 4;
const char *filenames[4] = {
//"Co60_235_319_25kTraces.root",
//"Co60_235_319_40kTraces.root",
//"Co60_235_319_55kTraces.root"
"Co60_EJ276_10k.root",
"Co60_EJ276_20k.root",
"Co60_EJ276_30k.root",
"Co60_EJ276_40k.root"
};

TFile *f;
TProfile *h2_px[nFiles];
TProfile *h3_px[nFiles];
TH2F *hP;

 for (int iF = 0; iF < nFiles; iF++){
  f = new TFile(filenames[iF],"READ");
  char hName[50];
  sprintf(hName,"hP0_%d_px",iF);

  hP = (TH2F*)f->Get("hP0");
  h2_px[iF] = hP->ProfileX(hName);
  h2_px[iF]->SetDirectory(0);
  h2_px[iF]->SetMarkerColor(iF+1);
  h2_px[iF]->SetMarkerStyle(20);
 
  sprintf(hName,"hP1_%d_px",iF);
  hP = (TH2F*)f->Get("hP1");
  h3_px[iF] = hP->ProfileX(hName);
  h3_px[iF]->SetDirectory(0);
  h3_px[iF]->SetMarkerColor(iF+1);
  h3_px[iF]->SetMarkerStyle(20);

  f->Close();

 // std::cout << "Closed " << hName[iF] << endl;
 }


 TCanvas *c1 = new TCanvas();
 h2_px[nFiles-1]->DrawNormalized();
  for (int iP = 1; iP < nFiles; iP++){
  h2_px[nFiles-1-iP]->DrawNormalized("same");
  }

 TCanvas *c2 = new TCanvas();
 h3_px[nFiles-1]->DrawNormalized();
  for (int iP = 1; iP < nFiles; iP++){
  h3_px[nFiles-1-iP]->DrawNormalized("same");
  }
 std::cout << "Done Normalizing" << endl;
}
