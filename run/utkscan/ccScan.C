void ccScan(){
  const int nFiles = 14;
  const char *filenames[nFiles] = {
"root_files/Output_Pterphenyl-2squares_Cf252_gammablock_PMT_CCM_16ns.root",
"root_files/Output_Pterphenyl-2squares_Cf252_gammablock_PMT_CCM_24ns.root",
"root_files/Output_Pterphenyl-2squares_Cf252_gammablock_PMT_CCM_32ns.root",
"root_files/Output_Pterphenyl-2squares_Cf252_gammablock_PMT_CCM_40ns.root",
"root_files/Output_Pterphenyl-2squares_Cf252_gammablock_PMT_CCM_48ns.root",
"root_files/Output_Pterphenyl-2squares_Cf252_gammablock_PMT_CCM_56ns.root",
"root_files/Output_Pterphenyl-2squares_Cf252_gammablock_PMT_CCM_64ns.root",
"root_files/Output_Pterphenyl-2squares_Cf252_gammablock_PMT_CCM_72ns.root",
"root_files/Output_Pterphenyl-2squares_Cf252_gammablock_PMT_CCM_80ns.root",
"root_files/Output_Pterphenyl-2squares_Cf252_gammablock_PMT_CCM_88ns.root",
"root_files/Output_Pterphenyl-2squares_Cf252_gammablock_PMT_CCM_96ns.root",
"root_files/Output_Pterphenyl-2squares_Cf252_gammablock_PMT_CCM_104ns.root",
"root_files/Output_Pterphenyl-2squares_Cf252_gammablock_PMT_CCM_112ns.root",
"root_files/Output_Pterphenyl-2squares_Cf252_gammablock_PMT_CCM_120ns.root",
 };


 TTree *myT;
 TFile *f;
  TCanvas *c1 = new TCanvas("c1","c1",1000,1000);
  c1->Divide(1,2);
 TH1D *hist1;
 TH1D *hist2;
 double LBound[4];
 double UBound[4];
 TF1 *fGauss[4];
 TF1 *total1;
 TF1 *total2;
 double par[6];
 double param[6];

 FILE *FoMout;
 FoMout = fopen("CCM_FoMScan.txt","w");
 bool done = false;
 const char *plot1, *plot2;
 cout << "Use Geometric Mean? [default is Average] (y/n):";
 string method;
 while(!done){
// stringstream method;
 getline(cin, method);
 if(method == 'y'){ plot1 = "sqrt(ratio[2]*ratio[3])>>h1(100)"; plot2 = "sqrt(ratio[2]*ratio[3])>>h2(100)"; done = true;}
 else if(method == 'n'){ plot1 = "(ratio[2]+ratio[3])/2>>h1(100)"; plot2 = "(ratio[2]+ratio[3])/2>>h2(100)";done = true;}
 else {cout << "Invalid entry. Please enter \"y\" or \"n\":";}
 }
 for (int iF = 0; iF < nFiles; iF++){
   cout << "On File " << iF+1 << " of " << nFiles << "(filename: "<< filenames[iF] << ")" << endl;
   f = new TFile(filenames[iF],"READ");
   f->GetObject("T",myT);
   c1->cd(1);
   myT->Draw(plot1,"phase[2]>0&&phase[3]>0&&(Pmax[2]+Pmax[3])/2>20000&&(Pmax[2]+Pmax[3])/2<22000","goff");
//   myT->Draw("(ratio[2]+ratio[3])/2>>h1(100)","phase[2]>0&&phase[3]>0&&(Pmax[2]+Pmax[3])/2>20000&&(Pmax[2]+Pmax[3])/2<22000","goff");
   hist1 = (TH1D*)gDirectory->Get("h1");
   hist1->Draw();

   c1->cd(2);
   myT->Draw(plot2,"phase[2]>0&&phase[3]>0&&(Pmax[2]+Pmax[3])/2>45000&&(Pmax[2]+Pmax[3])/2<47000","goff");
//   myT->Draw("(ratio[2]+ratio[3])/2>>h2(100)","phase[2]>0&&phase[3]>0&&Pmax[2]>45000&&Pmax[2]<47000","goff");
   hist2 = (TH1D*)gDirectory->Get("h2");
   hist2->Draw();
   c1->Update();
   for (int g = 0; g < 4; g++){
     cout << "(peak " << g << ") Hist Lower Bound = ";
     cin >> LBound[g];
     cout << "(peak " << g << ") Hist Upper Bound = ";
     cin >> UBound[g]; 
     if (LBound[g]==-9999||UBound[g]==-9999){
      cout << "Restarting FoM Calculation" << endl;
      g = -1;
      continue;
      }
     fGauss[g]= new TF1(Form("f%d",g),"gaus");
     fGauss[g]->SetRange(LBound[g],UBound[g]);
     if(g<=1){c1->cd(1); hist1->Fit(fGauss[g],"QR+");}
     if(g>=2){c1->cd(2); hist2->Fit(fGauss[g],"QR+");}
     c1->Update();

     if (g==1){
     total1 = new TF1(Form("fT%d",g),"gaus(0)+gaus(3)");
     fGauss[0]->GetParameters(&par[0]);
     fGauss[1]->GetParameters(&par[3]); 
     total1->SetParameters(par);
     total1->SetRange(LBound[0],UBound[1]);
     total1->SetLineColor(kBlue);
     hist1->Fit(total1,"QR+");
     c1->Update();
     }
     if (g==3){
     total2 = new TF1(Form("fT%d",g),"gaus(0)+gaus(3)");
     fGauss[2]->GetParameters(&par[0]);
     fGauss[3]->GetParameters(&par[3]); 
     total2->SetParameters(par);
     total2->SetRange(LBound[2],UBound[3]);
     total2->SetLineColor(kBlue);
     hist2->Fit(total2,"QR+");
     c1->Update();
      }
  }
  total1->GetParameters(&param[0]);
  double FoM1 = abs((param[1]-param[4])/(2.35*(param[2]+param[5])));

  total2->GetParameters(&param[0]);
  double FoM2 = abs((param[1]-param[4])/(2.35*(param[2]+param[5])));
  fprintf(FoMout,"%f %f\n", FoM1,FoM2);
  cout << FoM1 << " " << FoM2 << endl;
  
  f->Close();
 }
 c1->WaitPrimitive();
 fclose(FoMout);
}
