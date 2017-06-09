///@authors D. Miller
/********************************************************************/
/*	trace.cpp 						    */
/*		last updated: 10/02/09 DTM	     	       	    */
/*			       					    */
/********************************************************************/
#include <iostream>

#include <cstdio>
#include <cstdlib>
#include <cmath>

// pixie includes
#include "utilities.h"
#include "PixieInterface.h"
#include "unistd.h"

using std::cout;
using std::endl;

class TraceGrabber : public PixieFunction<> {
    bool operator()(PixieFunctionParms<> &par);
};

int main(int argc, char **argv) {
    int mod = -1, ch = -1;

    PixieInterface pif("pixie.cfg");

    pif.GetSlots();
    pif.Init();

    if (argc > 1) {
        mod = atoi(argv[1]);
        if (argc > 2) {
            ch = atoi(argv[2]);
        }
    }

    usleep(200);

    pif.Boot(PixieInterface::DownloadParameters |
             PixieInterface::ProgramFPGA |
             PixieInterface::SetDAC, true);

    TraceGrabber grabber;
    forChannel(pif, mod, ch, grabber);

    return EXIT_SUCCESS;
}

bool TraceGrabber::operator()(PixieFunctionParms<> &par) {
    static unsigned int modRead = par.pif.GetNumberCards();

    const size_t size = PixieInterface::GetTraceLength();
    unsigned short *trace = new unsigned short[size];

    if (modRead != par.mod) {
        par.pif.AcquireTraces(par.mod);
        modRead = par.mod;
    }

    usleep(10);
    if (par.pif.ReadSglChanTrace(trace, size, par.mod, par.ch)) {
        unsigned long sum = 0;
        unsigned long sumsq = 0;

        for (size_t i = 0; i < size; i++) {
            sum += trace[i];
            sumsq += trace[i] * trace[i];
        }
        printf("Trace ---- MOD/CHAN %2u / %2u AVER |-  %6.1f -| SIG %4.1f \n",
               par.mod, par.ch,
               (float) sum / size,
               (float) sqrt(size * sumsq - sum * sum) / size);

        delete[] trace;
        return true;
    }
    delete[] trace;
    return false;
}
