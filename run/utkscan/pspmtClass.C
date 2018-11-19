#define pspmtClass_cxx
#include "pspmtClass.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TEntryList.h>
#include <TMath.h>

//void printArray(int arr[], int size);
//void quicksort(int arr[], int l, int r);
//quicksort code taken from github @ https://gist.github.com/Erniuu/f5d38f1e6b892c70dbac
// Function to swap two pointers
void swap(Double_t *a, Double_t *b){
    Double_t temp = *a;
    *a = *b;
    *b = temp;
}
// Function to print an array of integers
void printArray(Double_t arr[], Int_t size)
{
    for (int i = 0; i < size; i++) printf("%f ", arr[i]);
    printf("\n");
}
// Function to run quicksort on an array of integers
// l is the leftmost starting index, which begins at 0
// r is the rightmost starting index, which begins at array length - 1
void quicksort(Double_t arr[], Int_t l, Int_t r)
{
    // Base case: No need to sort arrays of length <= 1
    if (l >= r) return;
    // Choose pivot to be the last element in the subarray
    Double_t pivot = arr[r];
    // Index indicating the "split" between elements smaller than pivot and 
    // elements greater than pivot
    int cnt = l;
    // Traverse through array from l to r
    for (int i = l; i <= r; i++){
        // If an element less than or equal to the pivot is found...
        if (arr[i] <= pivot){
            // Then swap arr[cnt] and arr[i] so that the smaller element arr[i] 
            // is to the left of all elements greater than pivot
            swap(&arr[cnt], &arr[i]);
            // Make sure to increment cnt so we can keep track of what to swap
            // arr[i] with
            cnt++;
        }
    }
    // NOTE: cnt is currently at one plus the pivot's index 
    // (Hence, the cnt-2 when recursively sorting the left side of pivot)
    quicksort(arr, l, cnt-2); // Recursively sort the left side of pivot
    quicksort(arr, cnt, r);   // Recursively sort the right side of pivot
}

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
   outTree->Branch("ToF_E",&ToF_E,"ToF_E/D");
   outTree->Branch("qdc[4]",&qdc,"qdc[4]/D");
   outTree->Branch("startqdc",&startqdc,"startqdc/D");
   outTree->Branch("stopqdc",&stopqdc,"stopqdc/D");
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

   xypos->Reset();

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
      if (qdc[0]>0&&qdc[1]>0) startqdc = (qdc[0]+qdc[1])/2.0;
      else startqdc = -9999;
      if (qdc[2]>0&&qdc[3]>0) stopqdc = (qdc[2]+qdc[3])/2.0;
      else stopqdc = -9999;
    outTree->Fill();

    if (nLeft==4 && nRight==4) {
    Xpos = (xpos[1]+xpos[0])/2.0;
    Ypos = (ypos[1]+ypos[0])/2.0;
    xypos->Fill(Xpos,Ypos);
    }

   }
   cout<<endl;
   outTree->Write();

   xypos->Write();
   xypos->ProjectionY()->Write();
   xypos->ProjectionX()->Write();

   //outputFile->Close();

}


void pspmtClass::Grid(){
  if (xypos->GetEntries()<10000){ 
   std::cout << "Position histogram only has " << xypos->GetEntries() << " entries" << endl; 
   return;
   }
  TSpectrum *X = new TSpectrum(8);
  TSpectrum *Y = new TSpectrum(4);
  TCanvas *c1 = new TCanvas();
  c1->Divide(1,2);
  c1->cd(1);
  Int_t nX = X->Search(xypos->ProjectionX(),0.1,"nobackground",0.2);
  c1->cd(2);
  Int_t nY = Y->Search(xypos->ProjectionY(),0.2,"nobackground",0.2);
  if (nX!=8 or nY!=4) {std::cout << "Too Few peaks found. X: " << nX << ", Y: "<< nY << endl; return;}
  Double_t *xpeaks, *ypeaks;
  xpeaks = X->GetPositionX();
  ypeaks = Y->GetPositionX();

//  std::cout << ypeaks[0] << " " << ypeaks[1] << " " << ypeaks[2] << " " << ypeaks[3] << endl;

// Run a test case of quicksort
// Test case taken from http://geeksquiz.com/quick-sort/
//    int arr[] = {10, 7, 8, 9, 1, 5};
//    int n = sizeof(arr) / sizeof(arr[0]);
    quicksort(xpeaks, 0, nX-1);
    quicksort(ypeaks, 0, nY-1);
    printf("Sorted arrays: \n");
    printArray(xpeaks, nX);
    printArray(ypeaks, nY); 
 
  FILE *fout; fout = fopen("Grid.txt","w");
  for (int i=0;i<nX-1;i++){
//   fprintf(fout,"%f ",xpeaks[i]);
   fprintf(fout,"%f ",(xpeaks[i]+xpeaks[i+1])/2.0);
//   if (i<7) xmins[i] = (xpeaks[i]+xpeaks[i+1])/2.0;
   }
  fprintf(fout,"\n");
  for (int i=0;i<nY-1;i++){
//   fprintf(fout,"%f ",ypeaks[i]);
   fprintf(fout,"%f ",(ypeaks[i]+ypeaks[i+1])/2.0);
///   if (i<3) ymins[i] = (ymins[i]+ymins[i+1])/2.0;
   }
  fprintf(fout,"\n");
  fclose(fout);
  return;
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


