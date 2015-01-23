#include "MCA.h"

#include <iostream>
#include <iomanip>

#include "PixieInterface.h"
#include "Display.h"
#include "Utility.h"

void MCA::Run(float duration) {

	//Start the pixie histogram
	_pif->StartHistogramRun();

	float runTime = 0;
	usleep(100);
	while (runTime < duration) {
		sleep(2);
		runTime += usGetDTime() / 1.0e6;
		std::cout << "|" << std::fixed << std::setprecision(2) << runTime  << " s |\r" << std::flush; 
		if (!_pif->CheckRunStatus()) {
			std::cout << Display::ErrorStr("Run TERMINATED") << std::endl;
			break;
		}
		for (int mod = 0;mod < _pif->GetNumberCards(); mod++) {
			for (int ch = 0; ch < _pif->GetNumberChannels(); ch++) {
				StoreData(mod, ch);
			}
		}
		Flush();
	}
	runTime += usGetDTime() / 1.0e6;
	std::cout << std::endl;
	_pif->EndRun();
	Display::LeaderPrint("Run finished");
	std::cout << Display::OkayStr() << std::endl;
	std::cout << std::setw(50) << std::setfill('.');
	std::cout << "Total running time: ";
	std::cout << std::fixed << std::setprecision(2) << runTime << " s" << std::endl; 
}
