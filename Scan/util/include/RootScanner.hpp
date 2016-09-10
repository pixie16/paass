#ifndef ROOTSCANNER_H
#define ROOTSCANNER_H

#include <map>

#include "TCanvas.h"
#include "TApplication.h"
#include "TAxis.h"

#include "ScanInterface.hpp"

class RootScanner : public ScanInterface {
	public:
		RootScanner();
		~RootScanner();
		TCanvas *GetCanvas() {return canvas_;};
		void IdleTask();
		void UpdateZoom(TAxis *axisX, TAxis *axisY, TVirtualPad* pad = gPad);
		void ResetZoom(TVirtualPad *pad = gPad);
	
	private:
		TCanvas *canvas_;
	
		struct AxisInfo {	
			float userZoomVals[2][2];
			float axisVals[2][2];
			bool reset;
		};
		std::map< TVirtualPad*, AxisInfo > zoomInfo_;

};

#endif //ROOTSCANNER_H
