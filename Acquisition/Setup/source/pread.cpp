///@authors D. Miller, C. R. Thornsberry
/********************************************************************/
/*	pread.cpp                                                       */
/*		last updated: April 19th, 2015 CRT                          */
/********************************************************************/

#include <iostream>

#include "PixieSupport.h"

int main(int argc, char *argv[]) {
    if (argc < 4) {
        std::cout << " Invalid number of arguments to " << argv[0] << std::endl;
        std::cout << "  SYNTAX: " << argv[0]
                  << " [module] [channel] [parameter]\n\n";
        return 1;
    }

    int mod = atoi(argv[1]);
    int ch = atoi(argv[2]);

    PixieInterface pif("pixie.cfg");

    pif.GetSlots();
    pif.Init();
    pif.Boot(PixieInterface::DownloadParameters | PixieInterface::ProgramFPGA |
             PixieInterface::SetDAC, true);

    std::string temp_str(argv[3]);
    ParameterChannelReader reader;
    forChannel(&pif, mod, ch, reader, temp_str);

    return 0;
}
