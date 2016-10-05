#ifndef HISTSCANNERCHANDATA_H
#define HISTSCANNERCHANDATA_H

#include <vector>
#include <utility>

#include "TObject.h"

#define NUMMODULES 13
#define NUMCHANNELS 16

//forward declaration of XiaData.
class XiaData;

class HistScannerChanData : public TObject{
	public:
		HistScannerChanData();
		virtual ~HistScannerChanData();
		void Clear();
		void Set(XiaData*);

	private:
		int mult[NUMMODULES][NUMCHANNELS];
		float filterEn[NUMMODULES][NUMCHANNELS];
		float peakAdc[NUMMODULES][NUMCHANNELS];
		float traceQdc[NUMMODULES][NUMCHANNELS];
		std::vector< std::pair< int, int > > hitMap_; //!Do not include in dictionary.

	ClassDef(HistScannerChanData, 1);
};

#endif //HISTSCANNERCHANDATA_H
