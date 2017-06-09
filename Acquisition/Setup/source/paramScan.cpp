/**	A program that scans a specified parameter and determines the effect on the resolution of the most
 *	energetic peak. This is useful for a Cs137 source.
 *	A ROOT file is output with canvases showing each fit an a final TGraph showing the resulting 
 *	resolution as a function of the parameter specified.
 *
 *	@authors  Karl Smith
 *	@date 26 Jan 2015
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

// Minimum RISETIME value given by r = 2^(N-1) * (32 ns)
// where r is the RISETIME and N is the FILTER_RANGE.
#define MIN_TRIGGER_RISETIME 0.016 // us
#define MIN_ENERGY_RISETIME 0.032 // us

// Minimum FLATTOP value given by f = 2^(N-1) * (48 ns)
// where f is the FLATTOP and N is the FILTER_RANGE
// except for TRIGGER_FLATTOP which has minumum 0.
#define MIN_TRIGGER_FLATTOP 0.000 // us
#define MIN_ENERGY_FLATTOP 0.048 // us

// Minimum filter step size is given by s = (N+1) * (8 ns)
// where s is the step size and N is the FILTER_RANGE.
#define MIN_ENERGY_STEP 0.016 // us
#define MIN_TRIGGER_STEP 0.008 // us

// Restrict the minimum on TAU because attempting to set
// TAU to zero causes the user to need to reboot the crate.
#define MIN_TAU 1E-4
#define MIN_TAU_STEP 1E-3

// FAST_FILTER_RANGE can only take the value zero.
// SLOW_FILTER_RANGE may only take non-zero positive values.

class parInfo {
public:
    const char *parName;
    double startVal;
    double stopVal;
    double stepSize;
    double value;
    int numSteps;
    double initialVal; ///<Value of parameter prior to scan.
    bool goodValues;
    bool debug;

    parInfo() : parName(0), startVal(0), stopVal(0), stepSize(0), value(0),
                numSteps(0), initialVal(0), goodValues(false), debug(false) {}

    parInfo(const char *parName_, const double &startVal_,
            const double &stopVal_, const double &stepSize_) :
            parName(parName_), startVal(startVal_), stopVal(stopVal_),
            stepSize(stepSize_), value(0), numSteps(0), initialVal(0),
            goodValues(false), debug(false) {
        checkValues();
    }

    bool setValues(const char *parName_, const double &startVal_,
                   const double &stopVal_, const double &stepSize_) {
        parName = parName_;
        startVal = startVal_;
        stopVal = stopVal_;
        stepSize = stepSize_;
        return checkValues();
    }

    bool checkValues() {
        if (stopVal <= startVal)
            return (goodValues = false);

        if (strcmp(parName, "ENERGY_FLATTOP") == 0)
            updateValues(MIN_ENERGY_FLATTOP, MIN_ENERGY_STEP);
        else if (strcmp(parName, "ENERGY_RISETIME") == 0)
            updateValues(MIN_ENERGY_RISETIME, MIN_ENERGY_STEP);
        else if (strcmp(parName, "TRIGGER_FLATTOP") == 0)
            updateValues(MIN_TRIGGER_FLATTOP, MIN_TRIGGER_STEP);
        else if (strcmp(parName, "TRIGGER_RISETIME") == 0)
            updateValues(MIN_TRIGGER_RISETIME, MIN_TRIGGER_STEP);
        else if (strcmp(parName, "TAU") == 0)
            updateValues(MIN_TAU, MIN_TAU_STEP);
        else
            return (goodValues = false);

        value = startVal;

        return (goodValues = true);
    }

    void increment() { value += stepSize; }

    void reset() { value = startVal; }

private:
    void updateValues(const double &minValue_, const double &minStep_) {
        if (startVal < minValue_)
            startVal = minValue_;

        if (stepSize < minStep_)
            stepSize = minStep_;

        // Ensure that the values are evenly divisible by the minimum step size.
        int tempStart = (int) (startVal * 1000);
        int tempStop = (int) (stopVal * 1000);
        int tempStep = (int) (stepSize * 1000);
        int tempMinStep = (int) (minStep_ * 1000);

        if (debug) {
            std::cout << "start=" << tempStart << " ns, stop=" << tempStop
                      << " ns, step=";
            std::cout << tempStep << " ns, minStep=" << tempMinStep << " ns\n";
        }

        if (tempStart % tempMinStep != 0) { // Go to the next lowest step.
            tempStart -= tempStart % tempMinStep;
            startVal = tempStart / 1000.0;
        }
        if (tempStop % tempMinStep != 0) { // Go to the next highest step.
            tempStop += tempStart % tempMinStep;
            stopVal = tempStop / 1000.0;
        }
        if (tempStep % tempMinStep != 0) { // Go to the next lowest step.
            tempStep -= tempStep % tempMinStep;
            stepSize = tempStep / 1000.0;
        }

        if (debug) {
            std::cout << "start=" << tempStart << " ns, stop=" << tempStop
                      << " ns, step=";
            std::cout << tempStep << " ns, minStep=" << tempMinStep << " ns\n";
        }

        // Calculate the number of steps.
        numSteps = (tempStop - tempStart) / tempStep;
        if ((tempStop - tempStart) % tempStep != 0)
            numSteps++;

        if (debug) {
            std::cout << "steps=" << numSteps << std::endl;
            for (int i = 0; i <= numSteps; i++)
                std::cout << " " << i << ": " << startVal + i * stepSize
                          << " ns\n";
        }

        stopVal = startVal + numSteps * stepSize;
    }
};

///A program
int main(int argc, char *argv[]) {
    if (argc < 7 || (argc > 9 && argc < 11) || argc > 13) {
        printf("Usage: %s <module> <channel> <parameterName> <start> <stop> <stepSize> [runtime] [scan.root]\n",
               argv[0]);
        return EXIT_FAILURE;
    }

    //Boolean specifiying if there was a second parameter
    bool isTwoDim;

    int mod = atoi(argv[1]);
    int ch = atoi(argv[2]);

    parInfo par1(argv[3], atof(argv[4]), atof(argv[5]), atof(argv[6]));
    parInfo par2;

    const char *outputFilename = "scan.root";
    float runTime = 10;

    if (argc < 11) {
        isTwoDim = false;
        if (argc > 7)
            runTime = atoi(argv[7]);
        if (argc == 9)
            outputFilename = argv[8];
    } else {
        isTwoDim = true;
        par2.setValues(argv[7], atof(argv[8]), atof(argv[9]), atof(argv[10]));
        if (argc > 11)
            runTime = atoi(argv[11]);
        if (argc == 13)
            outputFilename = argv[12];
    }

    //par1.debug = true;
    if (!par1.checkValues())
        return EXIT_FAILURE;
    if (isTwoDim && !par2.checkValues())
        return EXIT_FAILURE;

    std::cout << "Scanning M" << mod << "C" << ch << "\n";
    std::cout << "Scanning " << par1.parName << " from " << par1.startVal
              << "->" << par1.stopVal << " with steps of " << par1.stepSize
              << "\n";
    if (isTwoDim) {
        std::cout << "Scanning " << par2.parName << " from " << par2.startVal
                  << "->" << par2.stopVal << " with steps of " << par2.stepSize
                  << "\n";
    }
    std::cout << "MCA Run time: " << runTime << "s\n";
    std::cout << "Scan output: " << outputFilename << "\n";

    std::cout << "Number of steps: " << par1.numSteps;
    if (isTwoDim) std::cout << ", " << par2.numSteps;
    std::cout << "\n";

    if (!isTwoDim)
        std::cout << "Scan time: " << (par1.numSteps * runTime) / 60.0 << "m\n";
    else
        std::cout << "Scan time: "
                  << (par1.numSteps * par2.numSteps * runTime) / 60.0 << "m\n";

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

    if (!pif.ReadSglChanPar(par1.parName, par1.initialVal, mod, ch)) {
        std::cout << "Check parameter name!\n";
        return EXIT_FAILURE;
    }
    std::cout << par1.parName << " initial value: " << par1.initialVal << "\n";
    if (isTwoDim) {
        if (!pif.ReadSglChanPar(par2.parName, par2.initialVal, mod, ch)) {
            std::cout << "Check parameter name!\n";
            return EXIT_FAILURE;
        }
        std::cout << par2.parName << " initial value: " << par2.initialVal
                  << "\n";
    }

    TFile *f = new TFile(outputFilename, "RECREATE");
    TGraphErrors *gr = new TGraphErrors();
    TGraph2DErrors *gr2d = new TGraph2DErrors();

    MCA_ROOT *mca = new MCA_ROOT(&pif, "MCA");

    double readback;
    for (int step = 0; step <= par1.numSteps; ++step) {
        //Write parameter value
        pif.WriteSglChanPar(par1.parName, par1.value, mod, ch);

        //Read back the value to see what it actually was set to.
        pif.PrintSglChanPar(par1.parName, mod, ch);
        pif.ReadSglChanPar(par1.parName, readback, mod, ch);
        pif.SaveDSPParameters();
        printf("Readback : %f\n", readback);

        //Reset par2 value
        par2.reset();

        for (int step2 = 0; step2 <= par2.numSteps; ++step2) {
            if (isTwoDim) {
                //Write parameter value
                pif.WriteSglChanPar(par2.parName, par2.value, mod, ch);

                //Read back the value to see what it actually was set to.
                pif.PrintSglChanPar(par2.parName, mod, ch);
                pif.ReadSglChanPar(par2.parName, readback, mod, ch);
                pif.SaveDSPParameters();
            }

            if (mca->IsOpen())
                mca->Run(runTime);

            TH1 *hist = mca->GetHistogram(mod, ch);
            TSpectrum *s = new TSpectrum(10);
            s->Search(hist);
            TF1 *func = new TF1("func", "gaus");

            //Find the tallest peak and initialize the fitting function
            float maxValY = 0;
            for (int peak = 0; peak < s->GetNPeaks(); ++peak) {
                if (maxValY < s->GetPositionY()[peak]) {
                    maxValY = s->GetPositionY()[peak];
                    //Estimate parameters
                    float mean = s->GetPositionX()[peak];
                    float sigma = 0.03 * mean; //Reoslutions hould be roughly 3%
                    func->SetRange(mean - 3 * sigma, mean + 3 * sigma);
                    func->SetParameter(0, s->GetPositionY()[peak]);
                    func->SetParameter(1, mean);
                    func->SetParameter(2, sigma);
                }
            }
            //Fit the peak with options:
            //	R	Use specified Range
            //	Q	Quiet output
            //	M	More Fitting to improve fit
            //	E	Error Estimation
            hist->Fit(func, "RQME");

            float res = 100 * func->GetParameter(2) / func->GetParameter(1) *
                        2 * sqrt(2 * log(2));
            float resErr = res *
                           sqrt(pow(
                                   func->GetParError(1) / func->GetParameter(1),
                                   2) +
                                pow(func->GetParError(2) /
                                    func->GetParameter(2), 2)) *
                           2 * sqrt(2 * log(2));

            if (!isTwoDim) printf("Loop: %2d %5f ", step, par1.value);
            else
                printf("Loop: %2d:%2d %5f %5f ", step, step2, par1.value,
                       par2.value);
            printf("res: %5f%% FWHM Res: %5f%%\n\n", res / 2 / sqrt(2 * log(2)),
                   res);
            if (!isTwoDim) {
                gr->SetPoint(gr->GetN(), par1.value, res);
                gr->SetPointError(gr->GetN() - 1, 0, resErr);
            } else {
                gr2d->SetPoint(gr2d->GetN(), par1.value, par2.value, res);
                gr2d->SetPointError(gr2d->GetN() - 1, 0, 0, resErr);
            }

            f->cd();

            if (!isTwoDim) {
                hist->SetName(Form("h%d", step));
                hist->SetTitle(Form("%s %f", par1.parName, par1.value));
            } else {
                hist->SetName(Form("h%d_%d", step, step2));
                hist->SetTitle(Form("%s %f %s %f", par1.parName, par1.value,
                                    par2.parName, par2.value));
            }
            hist->Write();
            par2.increment();
        } //end par2 loop
        par1.increment();
    } //end par1 loop

    //Set the titles at the end so that the TGraph2D histogram has been created.
    if (!isTwoDim) {
        gr->GetXaxis()->SetTitle(Form("%s", par1.parName));
        gr->GetYaxis()->SetTitle("FWHM Resolution [%]");
        gr->SetMinimum(0);
    } else {
        gr2d->GetXaxis()->SetTitle(Form("%s", par1.parName));
        gr2d->GetYaxis()->SetTitle(Form("%s", par2.parName));
        gr2d->GetZaxis()->SetTitle("FWHM Resolution [%]");
        gr2d->SetMinimum(0);
    }

    std::cout << par1.parName << "\t" << "sigmaRes\t" << "FwhmRes\n";
    for (int i = 0; i < gr->GetN(); i++) {
        double x, y;
        if (gr->GetPoint(i, x, y) == i)
            std::cout << x << "\t" << y / 2 / sqrt(2 * log(2)) << "%\t" << y
                      << "%\n";

    }

    delete mca;

    std::cout << "Restoring initial parameter values.\n";
    pif.WriteSglChanPar(par1.parName, par1.initialVal, mod, ch);
    pif.PrintSglChanPar(par1.parName, mod, ch);
    if (isTwoDim) {
        pif.WriteSglChanPar(par2.parName, par2.initialVal, mod, ch);
        pif.PrintSglChanPar(par2.parName, mod, ch);
    }
    pif.SaveDSPParameters();

    f->cd();
    if (!isTwoDim) gr->Write("resGraph");
    else gr2d->Write("resGraph");
    f->Write(0, TObject::kOverwrite);
    f->Close();
    delete f;

    return 0;
}
