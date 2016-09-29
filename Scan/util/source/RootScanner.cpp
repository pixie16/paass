#include "RootScanner.hpp"

#include <limits>

#include "TSystem.h"
#include "TList.h"
#include "TObject.h"
#include "TAxis.h"
#include "TH1.h"
#include "TGraph.h"

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
	usleep(100000);
}

void RootScanner::ResetZoom(TVirtualPad *pad /*= gPad*/) {
	AxisInfo* padZoomInfo = &zoomInfo_[pad];
	for (int i=0;i<2;i++) {
		padZoomInfo->limitMin[i] = std::numeric_limits<float>::max();
		padZoomInfo->limitMax[i] = std::numeric_limits<float>::min();
		padZoomInfo->rangeUserMin[i] = std::numeric_limits<float>::max();
		padZoomInfo->rangeUserMax[i] = std::numeric_limits<float>::min();
	}
	padZoomInfo->reset = true;
}

//Update the zoom levels on a pad.
void RootScanner::UpdateZoom(TVirtualPad *pad /*= gPad*/) {
	//Get zoom info fo this pad.
	auto itr = zoomInfo_.find(pad);
	if (itr == zoomInfo_.end()) ResetZoom(pad);
	AxisInfo* padZoomInfo = &zoomInfo_[pad];

	//If zoom has been reset we continue, otherwise we get current axis limits.
	if (padZoomInfo->reset) padZoomInfo->reset = false;
	else {
		//Get the user zoom settings.
		padZoomInfo->rangeUserMin[0] = pad->GetUxmin();
		padZoomInfo->rangeUserMax[0] = pad->GetUxmax();
		padZoomInfo->rangeUserMin[1] = pad->GetUymin();
		padZoomInfo->rangeUserMax[1] = pad->GetUymax();
	}

	//Determine if the user had zoomed or unzoomed by comparing the current axis
	// limits to those taken from the canvas.
	bool userZoom[2];
	for (int i=0; i<2; i++) {
		userZoom[i] =  (padZoomInfo->rangeUserMin[i] > padZoomInfo->limitMin[i] || 
			padZoomInfo->rangeUserMax[i] < padZoomInfo->limitMax[i]);
	}	

	//Get the list of items on the pad.
	TList *list = gPad->GetListOfPrimitives();

	bool limitChange = false;

	//Loop over the objects in the list to determine pad limits.
	for( TObject *obj = list->First(); obj; obj = list->After(obj)) {
		TAxis *xAxis, *yAxis; //Pointers to the axes.
		//Check if the object is a histogram
		if ( TH1* hist = dynamic_cast<TH1*>(obj) ) {
			xAxis = hist->GetXaxis();
			yAxis = hist->GetYaxis();
			if (hist->GetBinContent(hist->GetMaximumBin()) * 1.1 > padZoomInfo->limitMax[1]) {
				padZoomInfo->limitMax[1] = 1.1 * hist->GetBinContent(hist->GetMaximumBin());
			}
		}
		//Check if the object is a graph
		else if ( TGraph* graph = dynamic_cast<TGraph*>(obj) ) {
			xAxis = graph->GetXaxis();
			yAxis = graph->GetYaxis();
		}
		//Not an object we care about so we continue.
		else continue;

		//If the axis min / max are outside current stored values thens We update 
		// the values.
		if (xAxis->GetXmin() < padZoomInfo->limitMin[0]) {
			padZoomInfo->limitMin[0] = xAxis->GetXmin(); 
			limitChange = true;
		}
		if (xAxis->GetXmax() > padZoomInfo->limitMax[0]) {
			padZoomInfo->limitMax[0] = xAxis->GetXmax(); 
			limitChange = true;
		}
		if (yAxis->GetXmin() < padZoomInfo->limitMin[1]) {
			padZoomInfo->limitMin[1] = yAxis->GetXmin(); 
			limitChange = true;
		}
		if (yAxis->GetXmax() > padZoomInfo->limitMax[1]) {
			padZoomInfo->limitMax[1] = yAxis->GetXmax(); 
			limitChange = true;
		}

	}

	//If the user didn't zoom we store the current axis limits in the userZoom 
	// values.
	for (int axis = 0; axis < 2; axis++) {
		if (!userZoom[axis]) {
			padZoomInfo->rangeUserMin[axis] = padZoomInfo->limitMin[axis];
			padZoomInfo->rangeUserMax[axis] = padZoomInfo->limitMax[axis];
		}
	}

	//Loop over the objects again and set the proper limits for each item.
	for( TObject *obj = list->First(); obj; obj = list->After(obj)) {
		TAxis *xAxis, *yAxis; //Pointers to the axes.
		//Check if the object is a histogram
		if ( TH1* hist = dynamic_cast<TH1*>(obj) ) {
			xAxis = hist->GetXaxis();
			yAxis = hist->GetYaxis();

			if (limitChange) {
				//Set the axes limits
				xAxis->SetLimits(padZoomInfo->limitMin[0], padZoomInfo->limitMax[0]);
				yAxis->SetLimits(padZoomInfo->limitMin[1], padZoomInfo->limitMax[1]);
				//Set the histogram maximum
				hist->SetMaximum(padZoomInfo->limitMax[1]);	
			}

		}
		//Check if the object is a graph
		else if ( TGraph* graph = dynamic_cast<TGraph*>(obj) ) {
			xAxis = graph->GetXaxis();
			yAxis = graph->GetYaxis();
		}
		else continue;

		//Set the range of the axis to the determined userZoom values.
		xAxis->SetRangeUser(padZoomInfo->rangeUserMin[0], padZoomInfo->rangeUserMax[0]);
		yAxis->SetRangeUser(padZoomInfo->rangeUserMin[1], padZoomInfo->rangeUserMax[1]);
	}
		
	if (limitChange) {
		pad->Modified();
	}

}

