#ifndef HISTSCANNERCHANDATA_H
#define HISTSCANNERCHANDATA_H

struct HistScannerChanData {
	int mod;
	int chan;
	float filterEn;
	float peakAdc;
	float traceQdc;
};

#endif //HISTSCANNERCHANDATA_H
