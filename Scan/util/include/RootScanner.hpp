#ifndef ROOTSCANNER_H
#define ROOTSCANNER_H

#include "ScanInterface.hpp"

#include "TCanvas.h"
#include "TApplication.h"

class RootScanner : public ScanInterface {
	public:
		RootScanner();
		~RootScanner();
		TCanvas *GetCanvas() {return canvas_;};
		void IdleTask();
	
	private:
		TCanvas *canvas_;

};

#endif //ROOTSCANNER_H
