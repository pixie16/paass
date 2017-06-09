///@authors D. Miller
/********************************************************************/
/*	trace_paw.cpp						    */
/*		last updated: 10/02/09 DTM	     	       	    */
/*			       					    */
/********************************************************************/
#include <cmath>
#include <cstdio>
#include <cstdlib>

// pixie includes
#include "utilities.h"
#include "PixieInterface.h"

// cernlib includes
#include "cfortran.h"
#include "hbook.h"

const size_t PAWC_SIZE = 900000;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float h[PAWC_SIZE];
} PAWC_DEF;

#define PAWC COMMON_BLOCK(PAWC,pawc)
COMMON_BLOCK_DEF(PAWC_DEF, PAWC);

#ifdef gfortran
PAWC_DEF pawc_;
#endif

#ifdef __cplusplus
}
#endif

class TraceGrabber : public PixieFunction<> {
    bool operator()(PixieFunctionParms<> &par);
};

int main(int argc, char *argv[]) {
    HLIMIT(PAWC_SIZE);

    if (argc != 3) {
        printf("usage: %s <module> <channel>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int mod = atoi(argv[1]);
    int ch = atoi(argv[2]);

    PixieInterface pif("pixie.cfg");
    pif.GetSlots();
    pif.Init();

    usleep(200);
    pif.Boot(PixieInterface::DownloadParameters |
             PixieInterface::ProgramFPGA |
             PixieInterface::SetDAC, true);


    TraceGrabber grabber;
    forChannel(pif, mod, ch, grabber);

    char traceFile[] = "trace.dat";
    char fileOptions[] = "N";

    HRPUT(0, traceFile, fileOptions);

    return EXIT_SUCCESS;
}

bool TraceGrabber::operator()(PixieFunctionParms<> &par) {
    char traceName[] = "test trace";
    static unsigned int modRead = par.pif.GetNumberCards();

    const size_t size = PixieInterface::GetTraceLength();
    unsigned short trace[size];

    if (modRead != par.mod) {
        par.pif.AcquireTraces(par.mod);
        modRead = par.mod;
    }

    unsigned int nhis = 100 * (par.mod + 1) + par.ch;

    usleep(10);
    if (par.pif.ReadSglChanTrace(trace, size, par.mod, par.ch)) {
        HBOOK1(nhis, traceName, size, 0, size, 0);

        unsigned long sum = 0;
        unsigned long sumsq = 0;

        for (size_t i = 0; i < size; i++) {
            sum += trace[i];
            sumsq += trace[i] * trace[i];

            HF1(nhis, i, trace[i]);
        }
        printf("Trace ---- MOD/CHAN %2u / %2u AVER |-  %6.1f -| SIG %4.1f \n",
               par.mod, par.ch,
               (float) sum / size,
               (float) sqrt(size * sumsq - sum * sum) / size);

        return true;
    } else return false;
}

