///@authors C. R. Thornsberry
// toggl.cpp
// Switch any CHANNEL_CSRA bit
// April 14th, 2015, CRT

#include <iostream>
#include <string>
#include <string.h>

#include "PixieSupport.h"

int main(int argc, char **argv) {
    BitFlipper flipper;

    if (argc < 4) {
        std::cout << " Invalid number of arguments to " << argv[0] << std::endl;
        std::cout << "  SYNTAX: " << argv[0] << " [mod] [chan] [bit]\n\n";
        flipper.Help();
        return 1;
    }

    int mod = atoi(argv[1]);
    int ch = atoi(argv[2]);

    PixieInterface pif("pixie.cfg");

    pif.GetSlots();
    pif.Init();
    pif.Boot(PixieInterface::DownloadParameters | PixieInterface::ProgramFPGA |
             PixieInterface::SetDAC, true);

    flipper.SetBit(argv[3]);

    std::string dum_str = "CHANNEL_CSRA";
    if (forChannel(&pif, mod, ch, flipper, dum_str)) {
        pif.SaveDSPParameters();
    }

    return 0;
}
