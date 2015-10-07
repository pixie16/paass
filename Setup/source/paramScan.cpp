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
	if (argc < 7 || (argc > 9 && argc < 11) || argc > 13 ) {
		printf("Usage: %s <module> <channel> <parameter name> <start> <stop> <stepSize> [runtime] [scan.root]\n",argv[0]);
		return EXIT_FAILURE;
	}

	//Boolean specifiying if there was a second parameter
	bool isTwoDim;

	struct parInfo {
		const char* parName;
		const double startVal;
		const double stopVal;
		const double stepSize;
		double value;
		int maxNumSteps;
		double initialVal; ///<Value of parameter prior to scan.
	};
	int mod = atoi(argv[1]);
	int ch = atoi(argv[2]);
	parInfo *par1 = new parInfo {argv[3],atof(argv[4]),atof(argv[5]), atof(argv[6])};
	const char* outputFilename = "scan.root";
	float runTime = 10;
	parInfo *par2;
	if (argc < 11) {
		isTwoDim = false;
		par2 = new parInfo{"",1,1,1};
		if (argc > 7)
			runTime = atoi(argv[7]);
		if (argc == 9)
			outputFilename = argv[8];
	}
	else {
		isTwoDim = true;
		par2 = new parInfo{argv[7],atof(argv[8]),atof(argv[9]), atof(argv[10])};
		if (argc > 11)
		runTime = atoi(argv[11]);
		if (argc == 13)
		outputFilename = argv[12];
	}

	std::cout << "Scanning M" << mod << "C" << ch << "\n";
	std::cout << "Scanning " << par1->parName << " from " << par1->startVal << "->" << par1->stopVal << " with steps of " << par1->stepSize << "\n";
	if (isTwoDim) {
		std::cout << "Scanning " << par2->parName << " from " << par2->startVal << "->" << par2->stopVal << " with steps of " << par2->stepSize << "\n";
	}
	std::cout << "MCA Run time: " << runTime << "s\n";
	std::cout << "Scan output: " << outputFilename << "\n";

	par1->maxNumSteps = fabs((par1->stopVal - par1->startVal) / par1->stepSize) + 1.5;
	par2->maxNumSteps = fabs((par2->stopVal - par2->startVal) / par2->stepSize) + 1.5;
	if (!isTwoDim) par2->maxNumSteps = 0;

	std::cout << "Max number of steps: " << par1->maxNumSteps;
	if (isTwoDim) std::cout << ", " << par2->maxNumSteps;
	std::cout << "\n";
	
	if (!isTwoDim) std::cout << "Maximum scan time: " << (par1->maxNumSteps * runTime) / 60.0 << "m\n";
	else std::cout << "Maximum scan time: " << (par1->maxNumSteps * par2->maxNumSteps * runTime) / 60.0 << "m\n";

	//Set par1 inital scan value
	par1->value = par1->startVal - par1->stepSize;


	std::cout << "\n";
	PixieInterface pif("pixie.cfg");
	pif.GetSlots();

	pif.Init();

	//cxx, end any ongoing runs
	pif.EndRun();
	pif.Boot(PixieInterface::DownloadParameters |
			PixieInterface::ProgramFPGA |
			PixieInterface::SetDAC, true);

	pif.RemovePresetRunLength(0);

	if (!pif.ReadSglChanPar(par1->parName, &par1->initialVal, mod, ch)) {
		std::cout << "Check parameter name!\n";
		return EXIT_FAILURE;
	}
	std::cout << par1->parName << " initial value: " << par1->initialVal << "\n";
	if (isTwoDim) {
		if (!pif.ReadSglChanPar(par2->parName, &par2->initialVal, mod, ch)) {
			std::cout << "Check parameter name!\n";
			return EXIT_FAILURE;
		}
		std::cout << par2->parName << " initial value: " << par2->initialVal << "\n";
	}

	TFile *f = new TFile(outputFilename,"RECREATE");
	TGraphErrors *gr = new TGraphErrors();
	TGraph2DErrors *gr2d = new TGraph2DErrors();

	MCA_ROOT *mca = new MCA_ROOT(&pif,"MCA");

	for (int step = 0; step <= par1->maxNumSteps; ++step) {
		double readback = par1->value;

		//Keep iterating until the value changes
		while (readback == par1->value) {
			par1->value += par1->stepSize;
			//If the value is larger than the stop value we exit
			if (par1->value > par1->stopVal) break;

			//Write parameter value
			pif.WriteSglChanPar(par1->parName,par1->value,mod,ch);
			//Read back the value to see what it actually was set to.
			pif.PrintSglChanPar(par1->parName, mod, ch);
			pif.ReadSglChanPar(par1->parName, &readback, mod, ch);
			pif.SaveDSPParameters();
			printf("Readback : %f\n",readback);
		}
		if (par1->value > par1->stopVal || readback > par1->stopVal) break;
		par1->value = readback;

		//Reset par2 value
		par2->value = par2->startVal - par2->stepSize;

		for (int step2 = 0; step2 <= par2->maxNumSteps; ++step2) {
			if (isTwoDim) {
				readback = par2->value;

				//Keep iterating until the value changes
				while (readback == par2->value) {
					par2->value += par2->stepSize;
					//If the value is larger than the stop value we exit
					if (par2->value > par2->stopVal) break;

					//Write parameter value
					pif.WriteSglChanPar(par2->parName,par2->value,mod,ch);
					//Read back the value to see what it actually was set to.
					pif.PrintSglChanPar(par2->parName, mod, ch);
					pif.ReadSglChanPar(par2->parName, &readback, mod, ch);
					pif.SaveDSPParameters();
				}
				if (par2->value > par2->stopVal || readback > par2->stopVal) break;
				par2->value = readback;
			}

			if (mca->IsOpen()) 
				mca->Run(runTime);

			TH1* hist = mca->GetHistogram(mod,ch);
			TSpectrum *s = new TSpectrum(10);
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

			float res = 100 * func->GetParameter(2) / func->GetParameter(1) * 
				2 * sqrt(2 * log(2));
			float resErr = res * 
				sqrt(pow(func->GetParError(1)/func->GetParameter(1),2) + 
					pow(func->GetParError(2)/func->GetParameter(2),2)) *
				2 * sqrt(2 * log(2));

			if (!isTwoDim) printf("Loop: %2d %5f ",step,par1->value);
			else printf("Loop: %2d:%2d %5f %5f ",step,step2,par1->value,par2->value);
			printf("res: %5f%% FWHM Res: %5f%%\n\n",res / 2 / sqrt(2*log(2)), res );
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
		} //end par2 loop
	} //end par1 loop

	//Set the titles at the end so that the TGraph2D histogram has been created.
	if (!isTwoDim) {
		gr->GetXaxis()->SetTitle(Form("%s",par1->parName));
		gr->GetYaxis()->SetTitle("FWHM Resolution [%]");
		gr->SetMinimum(0);
	}
	else {
		gr2d->GetXaxis()->SetTitle(Form("%s",par1->parName));
		gr2d->GetYaxis()->SetTitle(Form("%s",par2->parName));
		gr2d->GetZaxis()->SetTitle("FWHM Resolution [%]");
		gr2d->SetMinimum(0);
	}

	std::cout << par1->parName << "\t" << "sigmaRes\t" << "FwhmRes\n";
	for (int i=0;i<gr->GetN();i++) {
		double x,y;
		if (gr->GetPoint(i,x,y) == i)
			std::cout << x << "\t" << y / 2 / sqrt(2 * log(2)) << "%\t" << y << "%\n";
	
	}

	delete mca;

	std::cout << "Restoring initial parameter values.\n";
	pif.WriteSglChanPar(par1->parName, par1->initialVal, mod, ch);
	pif.PrintSglChanPar(par1->parName, mod, ch);
	if (isTwoDim) {
		pif.WriteSglChanPar(par2->parName, par2->initialVal, mod, ch);
		pif.PrintSglChanPar(par2->parName, mod, ch);
	}
	pif.SaveDSPParameters();

	f->cd();
	if (!isTwoDim) gr->Write("resGraph");
	else gr2d->Write("resGraph");
	f->Write(0,TObject::kOverwrite);
	f->Close();
	delete f;

}
