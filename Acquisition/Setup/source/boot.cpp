///@authors D. Miller
// Simple program to boot pixie cards loading the working set file

#include <cstdlib>

#include <unistd.h>

#include "PixieInterface.h"

int main(int argc, char *argv[]) {
    PixieInterface pif("pixie.cfg");

    pif.GetSlots();
    pif.Init();
    if (!pif.Boot(PixieInterface::BootAll, true)) {
        sleep(1);
        pif.Boot(PixieInterface::BootAll, true);
    }

    return EXIT_SUCCESS;
}
