///@authors D. Miller
// toggle_catcher.cpp
// Switch whether proton capture mode is on or off
// August 2010, DTM

#include <iostream>

#include <cstdlib>

#include "utilities.h"
#include "PixieInterface.h"

using std::cout;
using std::endl;

class CatcherToggler : public PixieFunction<> {
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

    CatcherToggler toggler;

    if (forChannel(pif, mod, ch, toggler))
        pif.SaveDSPParameters();

    return EXIT_SUCCESS;
}

bool CatcherToggler::operator()(PixieFunctionParms<> &par) {
#ifdef PIF_REVD
    if ( par.pif.ToggleCatcherBit(par.mod, par.ch) ) {
      par.pif.PrintSglChanPar("CHANNEL_CSRA", par.mod, par.ch);
      return true;
    } else {
      return false;
    }
#else
    cout << "Toggling proton catcher only implemented for Rev. D" << endl;
    return false;
#endif
}
