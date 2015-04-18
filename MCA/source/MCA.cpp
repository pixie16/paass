#include "MCA.h"

#include <iostream>
#include <iomanip>

#include "PixieInterface.h"
#include "Display.h"
#include "Utility.h"

/**The MCA is initialized and run for the specified duration or until a
 * stop command is received. At specific intervals the MCA output is
 * updated via MCA::StoreData().
 *
 * \parma[in] duration Amount of time to run the MCA.
 */
void MCA::Run(float duration) {
	//Reset stop flag in case it was used for termination.
	_stop = false;

	//Start the pixie histogram
	_pif->StartHistogramRun();

	float runTime = 0;

	//Loop until we reach the run duration or a stop is received.
	while (runTime < duration || !_stop) {
		sleep(2);
		//Update run time
		runTime += usGetDTime() / 1.0e6;
		std::cout << "|" << std::fixed << std::setprecision(2) << runTime  << " s |\r" << std::flush; 

		//Check if run is still ok
		if (!_pif->CheckRunStatus()) {
			std::cout << Display::ErrorStr("Run TERMINATED") << std::endl;
			break;
		}

		//Store the MCA data via the inherited method StoreData()
		for (int mod = 0;mod < _pif->GetNumberCards(); mod++) {
			for (int ch = 0; ch < _pif->GetNumberChannels(); ch++) {
				StoreData(mod, ch);
			}
		}
		//Flush the data to disk.
		Flush();
	}

	//End the run
	_pif->EndRun();

	//Display run completion information.
	runTime += usGetDTime() / 1.0e6;
	std::cout << std::endl;
	Display::LeaderPrint("Run finished");
	std::cout << Display::OkayStr() << std::endl;
	Display::LeaderPrint("Total running time:");
	std::cout << std::fixed << std::setprecision(2) << runTime << " s" << std::endl; 

	//Uset cout flags
	std::cout.unsetf(std::ios_base::floatfield);
	std::cout.precision(6);

}

void MCA::Stop() {
	_stop = true;
}
