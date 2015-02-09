/*	A program that scans a specified parameter and determines the effect on the resolution of the most 
 *	energetic peak. This is useful for a Cs137 source.
 *	A ROOT file is output with canvases showing each fit an a final TGraph showing the resulting 
 *	resolution as a function of the parameter specified.
 *
 *	Author: Karl Smith
 *	Date 26 Jan 2015
 *
 */

#include "TFile.h"
#include "TH1F.h"
#include "TF1.h"
#include "TSpectrum.h"
#include "TGraphErrors.h"
#include "TGraph2DErrors.h"

#include "PixieInterface.h"

#include "MCA_ROOT.h"

///A program
int main(int argc, char *argv[]) {
	if (argc != 8 && argc != 12) {
		printf("Usage: %s <module> <channel> <parameter name> <numSteps> <start> <stop> <out.root>\n",argv[0]);
		return EXIT_FAILURE;
	}

	//Boolean specifiying if there was a second parameter
	bool isTwoDim;

	struct parInfo {
		const char* parName;
		const int numSteps;
		const double startVal;
		const double stopVal;
		double value;
		double stepSize;
	};
	int mod = atoi(argv[1]);
	int ch = atoi(argv[2]);
	parInfo *par1 = new parInfo {argv[3],atoi(argv[4]),atof(argv[5]), atof(argv[6])};
	const char* outputFilename;
	parInfo *par2;
	if (argc == 8) {
		isTwoDim = false;
		par2 = new parInfo{"",1,0,0};
		outputFilename = argv[7];
	}
	else {
		isTwoDim = true;
		par2 = new parInfo{argv[7],atoi(argv[8]),atof(argv[9]), atof(argv[10])};
		outputFilename = argv[11];
	}
	float runTime = 10;


	PixieInterface pif("pixie.cfg");
	pif.GetSlots();

	pif.Init();

	//cxx, end any ongoing runs
	pif.EndRun();
	pif.Boot(PixieInterface::DownloadParameters |
			PixieInterface::ProgramFPGA |
			PixieInterface::SetDAC, true);

	pif.RemovePresetRunLength(0);

	TFile *f = new TFile(outputFilename,"RECREATE");
	TGraphErrors *gr = new TGraphErrors();
	TGraph2DErrors *gr2d = new TGraph2DErrors();

	MCA_ROOT *mca = new MCA_ROOT(&pif,"MCA");

	//Set inital par1 steps
	par1->stepSize = (par1->stopVal - par1->startVal) / (par1->numSteps-1);
	par1->value = par1->startVal;
	//Set inital par2 steps
	par2->stepSize = (par2->stopVal - par2->startVal) / (par2->numSteps-1);

	for (int step = 0; step < par1->numSteps; ++step) {
		if (par1->value > par1->stopVal) break;
		//Write parameter value
		pif.WriteSglChanPar(par1->parName,par1->value,mod,ch);
		//Read back the value to see what it actually was set to.
    	pif.PrintSglChanPar(par1->parName, mod, ch);
		pif.ReadSglChanPar(par1->parName, &par1->value, mod, ch);
		pif.SaveDSPParameters();
		
		//Reset par2 value
		par2->value = par2->startVal;
		for (int step2 = 0; step2 < par2->numSteps; ++step2) {
			if (isTwoDim) {
				if (par2->value > par2->stopVal) break;
				//Write parameter value
				pif.WriteSglChanPar(par2->parName,par2->value,mod,ch);
				//Read back the value to see what it actually was set to.
				pif.PrintSglChanPar(par2->parName, mod, ch);
				pif.ReadSglChanPar(par2->parName, &par2->value, mod, ch);
				pif.SaveDSPParameters();
			}

			if (mca->IsOpen()) 
				mca->Run(runTime);

			TH1* hist = mca->GetHistogram(mod,ch);
			TSpectrum *s = new TSpectrum(2);
			s->Search(hist);
			TF1 *func = new TF1("func","gaus");

			//Find the tallest peak and initialize the fitting function
			float maxValY = 0;
			for (int peak=0;peak < s->GetNPeaks();++peak) {
				if (maxValY < s->GetPositionY()[peak]) {
					maxValY = s->GetPositionY()[peak];
					//Estimate parameters
					float mean = s->GetPositionX()[peak];
					float sigma = 0.03 * mean; //Reoslutions hould be roughly 3%
					func->SetRange(mean - 3*sigma, mean + 3 * sigma);
					func->SetParameter(0,s->GetPositionY()[peak]);
					func->SetParameter(1,mean);
					func->SetParameter(2,sigma);
				}
			}
			//Fit the peak with options:
			//	R	Use specified Range
			//	Q	Quiet output
			//	M	More Fitting to improve fit
			//	E	Error Estimation
			hist->Fit(func,"RQME");

			float res = 100 * func->GetParameter(2) / func->GetParameter(1);
			float resErr = res * sqrt(pow(func->GetParError(1)/func->GetParameter(1),2) + 
					pow(func->GetParError(2)/func->GetParameter(2),2));

			if (!isTwoDim) printf("Loop: %2d %5f ",step,par1->value);
			else printf("Loop: %2d:%2d %5f %5f ",step,step2,par1->value,par2->value);
			printf("res: %5f\n",res);
			if (!isTwoDim) {
				gr->SetPoint(gr->GetN(),par1->value,res);
				gr->SetPointError(gr->GetN()-1,0,resErr);
			}
			else {
				gr2d->SetPoint(gr2d->GetN(),par1->value,par2->value,res);
				gr2d->SetPointError(gr2d->GetN()-1,0,0,resErr);
			}

			f->cd();

			if (!isTwoDim) {
				hist->SetName(Form("h%d",step));
				hist->SetTitle(Form("%s %f",par1->parName,par1->value));
			}
			else {
				hist->SetName(Form("h%d_%d",step,step2));
				hist->SetTitle(Form("%s %f %s %f",par1->parName,par1->value,par2->parName,par2->value));
			}
			hist->Write();
			if (isTwoDim) 
				par2->value += par2->stepSize;
		} //end par2 loop
		par1->value += par1->stepSize;
	} //end par1 loop

	//Set the titles at the end so that the TGraph2D histogram has been created.
	if (!isTwoDim) {
		gr->GetXaxis()->SetTitle(Form("%s",par1->parName));
		gr->GetYaxis()->SetTitle("Resolution");
		gr->SetMinimum(0);
	}
	else {
		gr2d->GetXaxis()->SetTitle(Form("%s",par1->parName));
		gr2d->GetYaxis()->SetTitle(Form("%s",par2->parName));
		gr2d->GetZaxis()->SetTitle("Resolution");
		gr2d->SetMinimum(0);
	}

	delete mca;

	f->cd();
	if (!isTwoDim) gr->Write("resGraph");
	else gr2d->Write("resGraph");
	f->Write(0,TObject::kOverwrite);
	f->Close();
	delete f;
}
