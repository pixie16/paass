///@authors D. Miller
/********************************************************************/
/*	mca_paw.cpp						    */
/*		last updated: 08/23/10 DTM	     	       	    */
/*			       					    */
/********************************************************************/
#include <iostream>

#include <cstdlib>

#include "utilities.h"

// pixie includes
#include "Display.h"
#include "PixieInterface.h"
#include "Utility.h"

// cernlib includes
#include "cfortran.h"
#include "hbook.h"

const size_t PAWC_SIZE = 1200000;
const size_t HIS_SIZE = 32768;

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    float PAW[PAWC_SIZE];
} PAWC_DEF;
#define PAWC COMMON_BLOCK(PAWC,pawc)
COMMON_BLOCK_DEF(PAWC_DEF, PAWC);

#ifdef gfortran
PAWC_DEF pawc_;
#endif

#ifdef __cplusplus
}
#endif

class HistogramReader : public PixieFunction<> {
public:
    bool operator()(PixieFunctionParms<> &par);
};

using std::cout;
using std::endl;
using std::flush;

int main(int argc, char *argv[]) {
    int time;

    if (argc >= 2) {
        time = atoi(argv[1]);
    } else {
        time = 10;
    }

    PixieInterface pif("pixie.cfg");
    pif.GetSlots();

    HLIMIT(PAWC_SIZE);

    pif.Init();

    //cxx, end any ongoing runs
    pif.EndRun();
    pif.Boot(PixieInterface::DownloadParameters |
             PixieInterface::ProgramFPGA |
             PixieInterface::SetDAC, true);

    pif.RemovePresetRunLength(0);
    pif.StartHistogramRun();
    usGetDTime();

    usleep(100);
    for (int sec = 0; sec < time; sec++) {
        sleep(1);
        cout << "|" << sec << " s |\r" << flush;
        if (!pif.CheckRunStatus()) {
            cout << Display::ErrorStr("Run TERMINATED") << endl;
            break;
        }
    }
    cout << endl;
    pif.EndRun();
    Display::LeaderPrint("Run finished");
    cout << usGetDTime() / 1e6 << " s " << endl;

    usleep(100);

    HistogramReader reader;
    forChannel(pif, -1, -1, reader);

    char hisFile[] = "mca.dat";
    char fileOptions[] = "N";
    HRPUT(0, hisFile, fileOptions);

    return EXIT_SUCCESS;
}

bool HistogramReader::operator()(PixieFunctionParms<> &par) {
    PixieInterface::word_t histo[HIS_SIZE];
    char hisName[] = "test spectrum";

    par.pif.ReadHistogram(histo, HIS_SIZE, par.mod, par.ch);

    unsigned short nhis = 100 * (par.mod + 1) + par.ch;
    HBOOK1(nhis, hisName, HIS_SIZE, 0, HIS_SIZE, 0);
    for (size_t i = 0; i < HIS_SIZE; i++) {
        HF1(nhis, i, histo[i]);
    }

    return true;
}
