#include <iostream>
#include <TProfile.h>
#include <TCanvas.h>
#include <stdio.h>

using namespace std;

void NormalizePulses(){

const int nFiles = 1;
const int nH = 12;
const char *filenames[nFiles] = {
//"Co60_235_319_25kTraces.root",
//"Co60_235_319_40kTraces.root",
//"Co60_235_319_55kTraces.root"
"Output.root"
};

TFile *f;
TProfile *h_pfx[nH];
TH2F *hP;

 for (int iF = 0; iF < nFiles; iF++){
  for (int iH=0; iH<nH; iH++){
  f = new TFile(filenames[iF],"READ");
  char hName[50];
  char pfxName[50];
  sprintf(hName,"hP%dK",iH*5+5);

  sprintf(pfxName,"hP%dK_pfx",iH*5+5);

  hP = (TH2F*)f->Get(hname);
  h_pfx[iF] = hP->ProfileX(pfxName);
  h_pfx[iF]->SetDirectory(0);
  h_pfx[iF]->SetMarkerColor(iH+1);
  h_pfx[iF]->SetMarkerStyle(20);

  }
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
