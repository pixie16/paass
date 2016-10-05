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
	
		static constexpr const int numAxes_ = 3;
		struct AxisInfo {	
			float rangeUserMin[numAxes_];
			float rangeUserMax[numAxes_];
			float limitMin[numAxes_];
			float limitMax[numAxes_];
			bool reset;
		};
		std::map< TVirtualPad*, AxisInfo > zoomInfo_;

};

#endif //ROOTSCANNER_H
