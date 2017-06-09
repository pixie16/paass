///@authors D. Miller, C. R. Thornsberry
/********************************************************************/
/*	pmwrite.cpp                                                       */
/*		last updated: April 19th, 2015 CRT                          */
/********************************************************************/

#include <iostream>
#include <stdlib.h>

#include "PixieSupport.h"

int main(int argc, char *argv[]) {
    if (argc < 5) {
        std::cout << " Invalid number of arguments to " << argv[0] << std::endl;
        std::cout << "  SYNTAX: " << argv[0]
                  << " [module] [channel] [parameter] [value]\n\n";
        return 1;
    }

    int mod = atoi(argv[1]);
    int ch = atoi(argv[2]);
    double value = std::strtod(argv[4], NULL);

    PixieInterface pif("pixie.cfg");

    pif.GetSlots();
    pif.Init();
    pif.Boot(PixieInterface::DownloadParameters | PixieInterface::ProgramFPGA |
             PixieInterface::SetDAC, true);

    std::string temp_str(argv[3]);
    ParameterChannelWriter writer;
    if (forChannel(&pif, mod, ch, writer,
                   make_pair(temp_str, value))) { pif.SaveDSPParameters(); }

    return 0;
}
