/********************************************************************
 *	MCA.cpp						    
 *	based on mca_paw.cpp
 *	KM 05 Jan 2012 
 ********************************************************************/
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>

#include <cstdlib>

#include "HisDrr.h"
#include "Exceptions.h"
#include "utilities.h"
#include "unistd.h"

// pixie includes
#include "Display.h"
#include "PixieInterface.h"
#include "Utility.h"

using namespace std;

const size_t HIS_SIZE = 16384;
const size_t ADC_SIZE = 32768;

class HistogramReader : public PixieFunction<>
{
  bool firstRun;
  HisDrr* histogram;
public:
  string basename_;
  bool operator()(PixieFunctionParms<> &par);
  HistogramReader(string basename) : basename_(basename) { firstRun = true; }
  ~HistogramReader() { delete histogram; }
};

bool HistogramReader::operator()(PixieFunctionParms<> &par)
{
    PixieInterface::word_t histo[ADC_SIZE];

    par.pif.ReadHistogram(histo, ADC_SIZE, par.mod, par.ch);

    try {
        if (firstRun) {
            string input = "mca_input.txt";
            string drr = basename_ + ".drr";
            string his = basename_ + ".his";

            Display::LeaderPrint("Creating new empty histogram");
            histogram = new HisDrr(drr, his, input);
            cout << Display::OkayStr() << endl;

            firstRun = false;
        }

        int id = (par.mod + 1) * 100 + par.ch;

        vector<unsigned int> data;
        data.resize(HIS_SIZE, 0);

        for (size_t i = 0; i < ADC_SIZE; i++) {
            unsigned v = static_cast<unsigned>(i / (ADC_SIZE / HIS_SIZE ));
            data[v] += histo[i];
        }
        histogram->setValue(id, data);

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

