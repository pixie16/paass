#include "RootScanner.hpp"

#include "TSystem.h"

RootScanner::RootScanner() : ScanInterface() {
	new TApplication("scanner", 0, NULL);

	canvas_ = new TCanvas("canvas", "");
	
}
RootScanner::~RootScanner() {
	canvas_->Close();
	delete canvas_;
}

/** IdleTask is called whenever a scan is not busy doing things. This method 
 * may be used to update things which need to be updated every so often
  * (e.g. a root TCanvas).
  * \return Nothing.
  */
void RootScanner::IdleTask() {
	gSystem->ProcessEvents();
	usleep(100000);
}
