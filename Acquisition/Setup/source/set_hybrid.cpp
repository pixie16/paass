///@authors D. Miller
// set_hybrid.cpp
// Set proton catcher mode to hybrid acquisition
//   Traces recorded for piled-up events, no traces otherwise
//   Energies and timestamps always recorded
// August 2010, DTM

#include <iostream>

#include <cstdlib>

#include "utilities.h"
#include "PixieInterface.h"

using std::cout;
using std::endl;

class HybridSetter : public PixieFunction<> {
    bool operator()(PixieFunctionParms<> &par);
};

int main(int argc, char **argv) {
    if (argc != 3) {
        cout << "usage: " << argv[0] << " <module> <channel>" << endl;
        return EXIT_FAILURE;
    }

    int mod = atoi(argv[1]);
    int ch = atoi(argv[2]);

    PixieInterface pif("pixie.cfg");

    pif.GetSlots();
    pif.Init();
    pif.Boot(PixieInterface::DownloadParameters |
             PixieInterface::ProgramFPGA |
             PixieInterface::SetDAC, true);

    HybridSetter setter;

    if (forChannel(pif, mod, ch, setter))
        pif.SaveDSPParameters();

    return EXIT_SUCCESS;
}

bool HybridSetter::operator()(PixieFunctionParms<> &par) {
#ifdef PIF_REVD
    if ( par.pif.SetProtonCatcherMode(par.mod, par.ch,
                      PixieInterface::PC_HYBRID) ) {
      par.pif.PrintSglChanPar("CHANNEL_CSRA", par.mod, par.ch);
      return true;
    } else {
      return false;
    }
#else
    cout << "Proton catcher modes only implemented for Rev. D" << endl;
    return false;
#endif
}
