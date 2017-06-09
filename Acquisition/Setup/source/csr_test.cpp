///@authors D. Miller, C. Thornsberry
/********************************************************************/
/*	csr_test.cpp						                            */
/*		last updated: April 17th, 2015 (CRT)                        */
/********************************************************************/

#include <iostream>

#include "PixieSupport.h"

int main(int argc, char *argv[]) {
    BitFlipper flipper;

    if (argc < 2) {
        std::cout << " Invalid number of arguments to " << argv[0] << std::endl;
        std::cout << "  SYNTAX: " << argv[0] << " [int]\n\n";
        return 1;
    }

    flipper.CSRAtest((unsigned int) atoi(argv[1]));

    return 0;
}  

