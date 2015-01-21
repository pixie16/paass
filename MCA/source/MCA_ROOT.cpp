/********************************************************************
 *	MCA_ROOT.cpp						    
 *	based on MCA.cpp
 *	KM 05 Jan 2012 
 *	KS 21 Jan 2015
 ********************************************************************/
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>

#include <cstdlib>
#include <map>

#include "Exceptions.h"
#include "utilities.h"
#include "unistd.h"

// pixie includes
#include "Display.h"
#include "PixieInterface.h"
#include "Utility.h"

#include "TFile.h"
#include "TH1D.h"

using namespace std;

const size_t HIS_SIZE = 16384;
const size_t ADC_SIZE = 32768;

class HistogramReader : public PixieFunction<>
{
	TFile* file;
	map<int,TH1D*> histograms;
public:
	string basename_;
	bool operator()(PixieFunctionParms<> &par);
	HistogramReader(string basename) : basename_(basename) { 
		file = new TFile(Form("%s.root",basename_.c_str()),"RECREATE");
	}
	~HistogramReader() { 
		file->Write();
		file->Close();
		delete file;
	}
};

bool HistogramReader::operator()(PixieFunctionParms<> &par)
{
	PixieInterface::word_t histo[ADC_SIZE];

	par.pif.ReadHistogram(histo, ADC_SIZE, par.mod, par.ch);

	try {
		int id = (par.mod + 1) * 100 + par.ch;
		auto loc = histograms.find(id);
		TH1D* histogram;
		if (loc != histograms.end())	{
			histogram = loc->second;
		}
		else {
      	Display::LeaderPrint("Creating new empty histogram");
         histogram = new TH1D(Form("h%d%02d",par.mod,par.ch),Form("Mod %d Ch %d",par.mod,par.ch),ADC_SIZE,0,ADC_SIZE);
			histograms[id] = histogram;
		
         cout << Display::OkayStr() << endl;
		}

		for (size_t i = 0; i < ADC_SIZE; i++) {
			histogram->SetBinContent(i+1,histo[i]);
      }

    } catch (IOError &err) {
        cout << "I/O Error: " << err.show() << endl;
        abort();
    } catch (GenError &err) {
        cout << "Error: " << err.show() << endl;
        abort();
    }

  return true;
}

int main(int argc, char *argv[])
{
  int totalTime;
  string basename = "MCA";
 
  if (argc >= 2) {
    totalTime = atoi(argv[1]);
    if (argc >= 3)
        basename = argv[2];
  } else {
    totalTime = 10;
  }

  PixieInterface pif("pixie.cfg");
  pif.GetSlots();

  pif.Init();

  //cxx, end any ongoing runs
  pif.EndRun();
  pif.Boot(PixieInterface::DownloadParameters |
	   PixieInterface::ProgramFPGA |
	   PixieInterface::SetDAC, true);

  pif.RemovePresetRunLength(0);
  double runTime = 0;

  pif.StartHistogramRun();

  HistogramReader reader(basename);
  usleep(100);
  while (runTime < totalTime) {
    sleep(2);
    runTime += usGetDTime() / 1.0e6;
    cout << "|" << fixed << setprecision(2) << runTime  << " s |\r" << flush; 
    if (!pif.CheckRunStatus()) {
      cout << Display::ErrorStr("Run TERMINATED") << endl;
      break;
    }
    forChannel(pif, -1, -1, reader);
  }
  runTime += usGetDTime() / 1.0e6;
  cout << endl;
  pif.EndRun();
  Display::LeaderPrint("Run finished");
  cout << Display::OkayStr() << endl;
  cout << setw(50) << setfill('.');
  cout << "Total running time: ";
  cout << fixed << setprecision(2) << runTime << " s" << endl; 
  usleep(100);

  return EXIT_SUCCESS;
}

