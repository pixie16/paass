/** Program to copy all parameters from one pixie channel or module
 * to another. 
 *
 * David Miller, May 2010
 */

#include <iostream>

#include <cstdlib>

#include <pixie16/pixie16.h>

#include "Display.h"
#include "PixieInterface.h"

using namespace std;
using namespace Display;

int main(int argc, char **argv) {
    PixieInterface pif("pixie.cfg");

    if (argc < 3 || argc > 5) {
        cout << "Usage: " << argv[0] << "<source mod> <dest mod>\n"
             << "     : " << argv[0] << "<source mod> <source ch> <dest mod>\n"
             << "     : " << argv[0]
             << "<source mod> <sourch ch> <dest mdo> <dest ch>"
             << endl;
        return EXIT_SUCCESS;
    }
    pif.GetSlots();
    pif.Init();
    pif.Boot(PixieInterface::DownloadParameters |
             PixieInterface::ProgramFPGA |
             PixieInterface::SetDAC, true);

    LeaderPrint("Copying parameters");

    unsigned short bitMask = 0x1fff; // parameters to copy (i.e. all)
    unsigned short sourceMod = atoi(argv[1]);
    unsigned short sourceChan;
    unsigned short destMod;
    unsigned int totChannels = pif.GetNumberChannels() * pif.GetNumberCards();
    unsigned short *destMask = new unsigned short[totChannels];
    bool success = true;

    for (unsigned int i = 0; i < totChannels; i++)
        destMask[i] = 0;

    if (argc == 3) {
        destMod = atoi(argv[2]);
    } else {
        sourceChan = atoi(argv[2]);
        destMod = atoi(argv[3]);
    }
    if (argc == 3) {
        // copy channel by channel between two modules
        for (unsigned int i = 0; i < pif.GetNumberChannels(); i++) {
            destMask[pif.GetNumberChannels() * destMod + i] = 1;
            if (Pixie16CopyDSPParameters(bitMask, sourceMod, i, destMask) < 0)
                success = false;
            // and reset the destMask bit for the next iteration
            destMask[pif.GetNumberChannels() * destMod + i] = 0;
        }
    } else {
        if (argc == 5) {
            destMask[pif.GetNumberChannels() * destMod + atoi(argv[4])] = 1;
        } else if (argc == 4) {
            for (unsigned int i = 0; i < pif.GetNumberChannels(); i++) {
                destMask[pif.GetNumberChannels() * destMod + i] = 1;
            }
        } else {
            // this shouldn't happen
            cout << ErrorStr() << endl;
            delete[] destMask;
            return EXIT_FAILURE;
        }
        if (Pixie16CopyDSPParameters(bitMask, sourceMod, sourceChan, destMask) <
            0)
            success = false;
    }

    if (success) {
        cout << OkayStr() << endl;
        pif.SaveDSPParameters();
    } else cout << ErrorStr() << endl;

    delete[] destMask;
    return EXIT_SUCCESS;
}
