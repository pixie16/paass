#include "HistScannerChanData.hpp"

#include "XiaData.hpp"

ClassImp(HistScannerChanData)

HistScannerChanData::HistScannerChanData() :
	mult{0},
	filterEn{0},
	peakAdc{0},
	traceQdc{0}
{

}

HistScannerChanData::~HistScannerChanData() {

}

void HistScannerChanData::Set(XiaData* data) {
	int mod = data->modNum;
	int chan = data->chanNum;

	if (mod >= NUMMODULES) {
		std::cout << "ERROR: Received data for unexpected module " << mod << " / " <<
NUMMODULES << "\n";
		return;
	}

	mult[mod][chan]++;
	filterEn[mod][chan] = data->energy; 

	//Do not delete as it causes segfault.
	ChannelEvent *chEvent = new ChannelEvent(data);

	chEvent->CorrectBaseline();

	peakAdc[mod][chan] = chEvent->maximum;
	traceQdc[mod][chan] = chEvent->IntegratePulse(chEvent->max_index - 10, chEvent->max_index + 15);

	hitMap_.push_back(std::make_pair(mod, chan));
}

void HistScannerChanData::Clear() {
	for(auto itr = hitMap_.begin(); itr != hitMap_.end(); ++itr) {
		int mod = itr->first;
		int chan = itr->second;

		mult[mod][chan] = 0;
		filterEn[mod][chan] = 0;
		peakAdc[mod][chan] = 0;
		traceQdc[mod][chan] = 0;
	}

	hitMap_.clear();
}
