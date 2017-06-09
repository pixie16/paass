///@authors D. Miller, C. R. Thornsberry
/********************************************************************/
/*	adjust_offsets.cpp                                              */
/*		last updated: April 19th, 2015 CRT                          */
/********************************************************************/

#include <iostream>

#include "PixieSupport.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << " Invalid number of arguments to " << argv[0] << std::endl;
        std::cout << "  SYNTAX: " << argv[0] << " [module]\n\n";
        return 1;
    }

    int mod = atoi(argv[1]);

    PixieInterface pif("pixie.cfg");

    pif.GetSlots();
    pif.Init();
    pif.Boot(PixieInterface::DownloadParameters | PixieInterface::ProgramFPGA |
             PixieInterface::SetDAC, true);

    OffsetAdjuster adjuster;
    if (forModule(&pif, mod, adjuster, 0)) { pif.SaveDSPParameters(); }

    return 0;
}
