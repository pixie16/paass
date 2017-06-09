///@authors D. Miller
/********************************************************************/
/*	rate.cpp   						    */
/*		last updated: 10/02/09 DTM	     	       	    */
/*			       					    */
/********************************************************************/

#include <iostream>

#include <cstdio>
#include <cstdlib>

#include "utilities.h"
#include "PixieInterface.h"

class StatsReader : public PixieFunction<> {
    bool operator()(PixieFunctionParms<> &par);
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: %s <module> <channel>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int mod = atoi(argv[1]);
    int ch = atoi(argv[2]);

    PixieInterface pif("pixie.cfg");
    pif.GetSlots();
    pif.Init();
    pif.Boot(0, true);

    printf(" %2s %2s  %10s  %10s  %10s  %10s  %10s\n",
           "M", "C", "Input", "Output", "Live_t", "Proc_ev", "RealTime");

    StatsReader reader;
    forChannel(pif, mod, ch, reader);

    return EXIT_SUCCESS;
}

bool StatsReader::operator()(PixieFunctionParms<> &par) {
    static unsigned int modDataRead = par.pif.GetNumberCards();

    if (modDataRead != par.mod)
        par.pif.GetStatistics(par.mod);

    printf(" %2u %2u  %10.1f  %10.1f  %10.0f  %10.0f  %10.1f \n",
           par.mod, par.ch,
           par.pif.GetInputCountRate(par.mod, par.ch),
           par.pif.GetOutputCountRate(par.mod, par.ch),
           par.pif.GetLiveTime(par.mod, par.ch),
           par.pif.GetProcessedEvents(par.mod),
           par.pif.GetRealTime(par.mod));

    modDataRead = par.mod;

    return true;
}
