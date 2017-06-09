///@authors D. Miller, C. R. Thornsberry
/********************************************************************/
/*	find_tau.cpp                                                    */
/*		last updated: April 19th, 2015 CRT                          */
/********************************************************************/

#include <iostream>

#include "PixieSupport.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cout << " Invalid number of arguments to " << argv[0] << std::endl;
        std::cout << "  SYNTAX: " << argv[0] << " [module] [channel]\n\n";
        return 1;
    }

    int mod = atoi(argv[1]);
    int ch = atoi(argv[2]);

    PixieInterface pif("pixie.cfg");

    pif.GetSlots();
    pif.Init();
    pif.Boot(PixieInterface::DownloadParameters | PixieInterface::ProgramFPGA |
             PixieInterface::SetDAC, true);

    TauFinder finder;
    forChannel(&pif, mod, ch, finder, 0);

    return 0;
}
