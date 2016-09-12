#include "RootScanner.hpp"

#include "TSystem.h"

RootScanner::RootScanner() : 
	ScanInterface()
{
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
	canvas_->Update();
	usleep(100000);
}

void RootScanner::ResetZoom(TVirtualPad *pad /*= gPad*/) {
	AxisInfo* padZoomInfo = &zoomInfo_[pad];
	for (int i=0;i<2;i++) {
		padZoomInfo->axisVals[i][0] = 1E9;
		padZoomInfo->axisVals[i][1] = -1E9;
		padZoomInfo->userZoomVals[i][0] = 1E9;
		padZoomInfo->userZoomVals[i][1] = -1E9;
	}
	padZoomInfo->reset = true;
}

/**The following routine permits a plot to auto zoom unless the user has set 
 * the zoom. If the user has zoomed the axis extents are stored, but the user
 * zoom is set.
 */
void RootScanner::UpdateZoom(TAxis *axisX, TAxis *axisY, TVirtualPad *pad /*=gPad*/) {
	auto itr = zoomInfo_.find(pad);
	if (itr == zoomInfo_.end()) ResetZoom(pad);
	AxisInfo* padZoomInfo = &zoomInfo_[pad];

	//If zoom has been reset we continue, otherwise we get current axis limits.
	if (padZoomInfo->reset) padZoomInfo->reset = false;
	else {
		//Get the user zoom settings.
		padZoomInfo->userZoomVals[0][0] = pad->GetUxmin();
		padZoomInfo->userZoomVals[0][1] = pad->GetUxmax();
		padZoomInfo->userZoomVals[1][0] = pad->GetUymin();
		padZoomInfo->userZoomVals[1][1] = pad->GetUymax();
	}

	//Determine if the user had zoomed or unzoomed by comparing the current axis
	// limits to those taken from the canvas.
	bool userZoom[2];
	for (int i=0; i<2; i++) {
		userZoom[i] =  (padZoomInfo->userZoomVals[i][0] > padZoomInfo->axisVals[i][0] || 
			padZoomInfo->userZoomVals[i][1] < padZoomInfo->axisVals[i][1]);
	}

	//If the axis min / max are outside current stored values thens We update 
	// the values.
	if (axisX->GetXmin() < padZoomInfo->axisVals[0][0]) 
		padZoomInfo->axisVals[0][0] = axisX->GetXmin(); 
	if (axisX->GetXmax() > padZoomInfo->axisVals[0][1]) 
		padZoomInfo->axisVals[0][1] = axisX->GetXmax(); 
	if (axisY->GetXmin() < padZoomInfo->axisVals[1][0]) 
		padZoomInfo->axisVals[1][0] = axisY->GetXmin(); 
	if (axisY->GetXmax() > padZoomInfo->axisVals[1][1]) 
		padZoomInfo->axisVals[1][1] = axisY->GetXmax(); 

	//Set the limits of the axis to the determined min / max.
	axisX->SetLimits(padZoomInfo->axisVals[0][0], padZoomInfo->axisVals[0][1]);
	axisY->SetLimits(padZoomInfo->axisVals[1][0], padZoomInfo->axisVals[1][1]);

	//If the user didn't zoom we store the current axis limits in the userZoom 
	// values.
	for (int i = 0; i < 2; i++) {
		if (!userZoom[i]) {
			for (int j = 0; j < 2; j++) padZoomInfo->userZoomVals[i][j] = padZoomInfo->axisVals[i][j];
		}
	}

	//Set the range of the axis to the determined userZoom values.
	axisX->SetRangeUser(padZoomInfo->userZoomVals[0][0], padZoomInfo->userZoomVals[0][1]);
	axisY->SetRangeUser(padZoomInfo->userZoomVals[1][0], padZoomInfo->userZoomVals[1][1]);

}

