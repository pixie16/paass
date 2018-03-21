#include <iostream>
#include <vector>
#include <TTree.h>
#include <TH2F.h>
#include "TGraph.h"
#include "TCutG.h"

void FindRes(){

TFile *file1 = new TFile("/data/NEXT/TestStand/ToF_11072017_25cm_all.root");
TTree *t = (TTree*)file1->Get("timing");

//gROOT->SetBatch(kTRUE);

t->Draw("start1.time-start2.time:start1.qdc>>h1(1000,0,700000,100,0,20)","start1.phase>0&&start2.phase>0","goff");
TH2F *H2 = (TH2F*)gDirectory->Get("h1");

int LB = H2->FindLastBinAbove(0,1);

//const int nP = 700000/5000;
std::vector <double> x, Tres, Rres;
double uTh = H2->ProjectionX()->GetBinLowEdge(LB);
double xVal = 50000;
double yVal, rmsVal;

//char cCut[100];
int nP=0;
TCutG *cutG = new TCutG("cutG",5);
   cutG->SetName("cutG");
   cutG->SetVarX("start1.qdc");
   cutG->SetVarY("start1.time-start2.time");
   cutG->SetPoint(0,uTh+2000,0);
   cutG->SetPoint(1,uTh+2000,19);

  TF1 *f1 = new TF1("f1","gaus(0)"); 

 while (xVal<500000){
  nP++;
  x.push_back(xVal);

   cutG->SetPoint(2,xVal,19);
   cutG->SetPoint(3,xVal,0);
   cutG->SetPoint(4,uTh+2000,0);
 
//  sscanf(cCut,"start1.qdc > %f && start1.phase>0 && start2.phase>0", xVal);
//  t->Draw("start1.time-start2.time:start1.qdc>>h1(1000,0,700000,100,0,20)","cutG","goff");
//  TH2F *H2 = (TH2F*)gDirectory->Get("h1");

  rmsVal = H2->ProjectionY("h_Py",0,-1,"[cutG]")->GetRMS();
  

  TH1D *h_pY = H2->ProjectionY("h_Py",0,-1,"[cutG]");
  //double maxbin = h_pY->GetMaximumBin(); 
  h_pY->Fit(f1,"QN");
  yVal = f1->GetParameter(2);

  Rres.push_back(rmsVal*2.35);  
  Tres.push_back(yVal*2.35);
  std::cout << nP << " " << xVal << " " << yVal << " " << rmsVal << endl;
  xVal += 20000;
  
  }

//gROOT->SetBatch(kFALSE);

TCanvas *c2 = new TCanvas("c2","c2",600,600);

TGraph *g1 = new TGraph(nP,&(x[0]),&(Tres[0]));
TGraph *g2 = new TGraph(nP,&(x[0]),&(Rres[0]));

g1->SetLineColor(kBlue);
g1->SetLineWidth(2);
g2->SetLineColor(kRed);
g2->SetLineWidth(2);


g1->Draw();
g2->Draw("same");

//  H2->ProjectionY("h_pX",0,-1,"[cutG]")->Draw();
}
