/// @authors K. Miernik, K. Smith, C. R. Thornsberry
#include "MCA.h"

#include <iostream>
#include <iomanip>
#include <unistd.h>

#include "PixieInterface.h"
#include "Display.h"
#include "Utility.h"

///Default constructor
MCA::MCA(PixieInterface *pif) : _pif(pif) {
    time(&start_time);
}

///Return the length of time the MCA has been running.
double MCA::GetRunTime() {
    time(&stop_time);
    return difftime(stop_time, start_time);
}

/**The MCA is initialized and run for the specified duration or until a
 * stop command is received. At specific intervals the MCA output is
 * updated via MCA::StoreData(). Will continue until external bool (stop)
 * is set to false. If this pointer is set to NULL, will continue uninterrupted.
 *
 * \param[in] duration Amount of time to run the MCA.
 * \param[in] stop External boolean flag for stop run command.
 */
void MCA::Run(float duration, bool *stop) {
    //Start the pixie histogram
    _pif->StartHistogramRun();

    time(&start_time);

    //Loop until we reach the run duration or a stop is received.
    while (true) {
        if (stop != NULL && *stop) { break; }
        else if (duration > 0.0 && (difftime(stop_time, start_time) >=
                                    duration)) { break; } // Adds support for infinite MCA runs

        sleep(2);

        //Update run time
        std::cout << "|" << std::fixed << std::setprecision(2) << GetRunTime()
                  << " s |\r" << std::flush;

        //Check if run is still ok
        if (!_pif->CheckRunStatus()) {
            std::cout << Display::ErrorStr("Run TERMINATED") << std::endl;
            break;
        }

        //Store the MCA data via the inherited method StoreData()
        for (int mod = 0; mod < _pif->GetNumberCards(); mod++) {
            for (unsigned int ch = 0; ch < _pif->GetNumberChannels(); ch++) {
                StoreData(mod, ch);
            }
        }

        //Flush the data to disk.
        Flush();

        //Update the timer.
        time(&stop_time);
    }

    //End the run
    _pif->EndRun();

    //Display run completion information.
    std::cout << std::endl;
    Display::LeaderPrint("Run finished");
    std::cout << Display::OkayStr() << std::endl;
    Display::LeaderPrint("Total running time:");
    std::cout << std::fixed << std::setprecision(2) << GetRunTime() << " s"
              << std::endl;

    //Uset cout flags
    std::cout.unsetf(std::ios_base::floatfield);
    std::cout.precision(6);
}

bool MCA::Step() {
    if (!_pif || !_pif->CheckRunStatus()) { return false; }

    //Store the MCA data via the inherited method StoreData()
    for (int mod = 0; mod < _pif->GetNumberCards(); mod++) {
        for (unsigned int ch = 0; ch < _pif->GetNumberChannels(); ch++) {
            StoreData(mod, ch);
        }
    }

    //Flush the data to disk.
    Flush();

    time(&stop_time);

    return true;
}
