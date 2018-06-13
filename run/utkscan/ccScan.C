void ccScan(){
  const int nFiles = 1;
 // const string *filenames[nFiles] = {"Output_Pterphenyl-2squares_Cf252_gammablock_PMT_CCM_112ns.root"};
 TTree *myT;
 gROOT->Reset(); 
 TFile *f;
 //TCanvas *c1 = new TCanvas("c1");
  TCanvas *c1 = new TCanvas("c1","c1",1000,1000);
  c1->Divide(1,2);
 // TCanvas *c2 = new TCanvas("c2");
 TH1D *hist1;
 TH1D *hist2;
 double LBound[4];
 double UBound[4];
 
 for (int iF = 0; iF < nFiles; iF++){
   f = new TFile("Output_Pterphenyl-2squares_Cf252_gammablock_PMT_CCM_64ns.root","READ");
   f->GetObject("T",myT);
   c1->cd(1);
   myT->Draw("(ratio[2]+ratio[3])/2>>h1(100)","phase[2]>0&&phase[3]>0&&Pmax[2]>20000&&Pmax[2]<22000","goff");
   hist1 = (TH1D*)gDirectory->Get("h1");
   hist1->Draw();
   //gPad->WaitPrimitive();


   c1->cd(2);
   myT->Draw("(ratio[2]+ratio[3])/2>>h2(100)","phase[2]>0&&phase[3]>0&&Pmax[2]>45000&&Pmax[2]<47000","goff");
   hist2 = (TH1D*)gDirectory->Get("h2");
   hist2->Draw();
   c1->Update();
   // gPad->WaitPrimitive();
   for (int g = 0; g < 4; g++){
     cout << "Hist Lower Bound = ";
     cin >> LBound[g];
     cout << "Hist Upper Bound = ";
     cin >> UBound[g]; 
  }
 }
}
