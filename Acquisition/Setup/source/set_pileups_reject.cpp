///@authors D. Miller
// set_pileups_reject.cpp
// Set proton catcher mode to pile-up rejection
//   Traces and energies only recorded for non piled-up events
// August 2010, DTM

#include <iostream>

#include <cstdlib>

#include "utilities.h"
#include "PixieInterface.h"

using std::cout;
using std::endl;

class RejectionSetter : public PixieFunction<> {
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

    RejectionSetter setter;

    if (forChannel(pif, mod, ch, setter))
        pif.SaveDSPParameters();

    return EXIT_SUCCESS;
}

bool RejectionSetter::operator()(PixieFunctionParms<> &par) {
#ifdef PIF_REVD
    if ( par.pif.SetProtonCatcherMode(par.mod, par.ch,
                      PixieInterface::PC_REJECT) ) {
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
