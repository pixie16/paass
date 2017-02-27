#include "HistScannerChanData.hpp"

#include "XiaData.hpp"

ClassImp(HistScannerChanData)

HistScannerChanData::HistScannerChanData() :
	mult{0},
	filterEn{0},
	peakAdc{0},
	traceQdc{0},
	baseline{0},
	timeStampNs{0},
	cfdBin{0},
	timeCfdNs{0}
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

	timeStampNs[mod][chan] = data->time * 8; // 8 NS / SYSTEM Clock

	//Do not delete as it causes segfault.
	ChannelEvent *chEvent = new ChannelEvent(data);

	baseline[mod][chan] = chEvent->CorrectBaseline();

	peakAdc[mod][chan] = chEvent->maximum;
	traceQdc[mod][chan] = chEvent->IntegratePulse(chEvent->max_index - 10, chEvent->max_index + 15);

	cfdBin[mod][chan] = chEvent->AnalyzeCFD(); //BIN
	timeCfdNs[mod][chan] = timeStampNs[mod][chan] + cfdBin[mod][chan] * 4; // 4 NS / ADC Clock

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
		baseline[mod][chan] = 0;
		timeStampNs[mod][chan] = 0;
		cfdBin[mod][chan] = 0;
		timeCfdNs[mod][chan] = 0;
	}

	hitMap_.clear();
}
