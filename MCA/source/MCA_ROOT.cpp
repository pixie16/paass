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

#include <stdio.h>

using namespace std;

const size_t HIS_SIZE = 16384;
const size_t ADC_SIZE = 32768;

class HistogramReader : public PixieFunction<>
{
	private:
		TFile* file;
		map<int,TH1D*> histograms;
		///A class to handle redirecting stderr
		/**The class redirects stderr to a text file and also saves output in case the user
		 * would like to print it. Upon destruction stderr is restored to stdout.
		 */
		class cerr_redirect {
			private:
			char buf[BUFSIZ];
			
			public:
			cerr_redirect(const char* logFile) 
			{
				freopen(logFile,"a",stderr);
				setbuf(stderr, buf);
			};

			/**stderr is pointed to a duplicate of stdout, buffering is then set to no buffering.
 			*/
			~cerr_redirect( ) {
				dup2(fileno(stdout),fileno(stderr));
				setvbuf(stderr,NULL,_IONBF,BUFSIZ);
			};
			void Print() {
				fprintf(stdout,"%s",buf);
				fflush(stdout);
			}

		};
	public:
		HistogramReader(string basename, PixieInterface *pif);
		~HistogramReader();
		bool operator()(PixieFunctionParms<> &par);
		///Flush the histogram output to the ROOT file.
		void Flush();
};

/**Constructor which build a ROOT file with histograms for MCA.
 */
HistogramReader::HistogramReader(string basename,PixieInterface *pif) { 
	Display::LeaderPrint("Creating new empty histogram");

	cerr_redirect *redirect = new cerr_redirect("Pixie16msg.txt");
	file = new TFile(Form("%s.root",basename.c_str()),"RECREATE");
	if (Display::StatusPrint(!(file->IsOpen() && file->IsWritable()))) {
		redirect->Print();
		exit(EXIT_FAILURE);
	}
	delete redirect;
	
	for (int card=0;card < pif->GetNumberCards();card++) {
		for (int ch=0;ch < pif->GetNumberChannels();ch++) {
			int id = (card + 1) * 100 + ch;
			histograms[id] = new TH1D(Form("h%d%02d",card,ch),Form("Mod %d Ch %d",card,ch),ADC_SIZE,0,ADC_SIZE);
		}
	}
	Flush();

}
HistogramReader::~HistogramReader() { 
	Flush();
	file->Close();
	delete file;
}
bool HistogramReader::operator()(PixieFunctionParms<> &par)
{
	PixieInterface::word_t histo[ADC_SIZE];

	par.pif.ReadHistogram(histo, ADC_SIZE, par.mod, par.ch);

	int id = (par.mod + 1) * 100 + par.ch;
	auto loc = histograms.find(id);
	TH1D* histogram;
	if (loc == histograms.end())	{
		return false;
	}
	histogram = loc->second;

	for (size_t i = 0; i < ADC_SIZE; i++) {
		histogram->SetBinContent(i+1,histo[i]);
	}

	file->Write(0,TObject::kOverwrite);

  return true;
}
void HistogramReader::Flush()
{
	file->Write(0,TObject::kOverwrite);
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

  HistogramReader reader(basename,&pif);

  pif.StartHistogramRun();

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
		reader.Flush();
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

