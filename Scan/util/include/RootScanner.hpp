#ifndef ROOTSCANNER_H
#define ROOTSCANNER_H

#include <map>

#include "TCanvas.h"
#include "TApplication.h"

#include "ScanInterface.hpp"

class RootScanner : public ScanInterface {
	public:
		RootScanner();
		~RootScanner();
		TCanvas *GetCanvas() {return canvas_;};
		void IdleTask();
		void UpdateZoom(TVirtualPad* pad = gPad);
		void ResetZoom(TVirtualPad *pad = gPad);
	
	private:
		TCanvas *canvas_;
	
		struct AxisInfo {	
			float rangeUserMin[2];
			float rangeUserMax[2];
			float limitMin[2];
			float limitMax[2];
			bool reset;
		};
		std::map< TVirtualPad*, AxisInfo > zoomInfo_;

};

#endif //ROOTSCANNER_H
