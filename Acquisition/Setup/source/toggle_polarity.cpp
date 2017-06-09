///@authors D. Miller
// toggle_polarity.cpp
// flips the polarity setting for a Pixie-16 channel
// August 2010, DTM

#include <cstdio>
#include <cstdlib>

#include "utilities.h"

#include "PixieInterface.h"

class PolarityToggler : public PixieFunction<> {
public:
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
    pif.Boot(PixieInterface::DownloadParameters |
             PixieInterface::ProgramFPGA |
             PixieInterface::SetDAC, true);

    PolarityToggler toggler;

    if (forChannel(pif, mod, ch, toggler))
        pif.SaveDSPParameters();

    return EXIT_SUCCESS;
}

bool PolarityToggler::operator()(PixieFunctionParms<> &par) {
    if (par.pif.TogglePolarity(par.mod, par.ch)) {
        par.pif.PrintSglChanPar("CHANNEL_CSRA", par.mod, par.ch);
        return true;
    } else {
        return false;
    }
}
