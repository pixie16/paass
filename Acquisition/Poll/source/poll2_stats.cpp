#include <iostream>
#include <vector>
#include <string.h>
#include <unistd.h>

#include "poll2_stats.h"
#include "poll2_socket.h"

StatsHandler::StatsHandler(const size_t nCards){

	numCards = nCards;

	// Define all the 2d arrays
	nEventsDelta = new unsigned int*[numCards];
	nEventsTotal = new unsigned int*[numCards]; 
	calcEventRate = new double*[numCards];
	inputCountRate = new double*[numCards];
	outputCountRate = new double*[numCards];
	for(unsigned int i = 0; i < numCards; i++){
		nEventsDelta[i] = new unsigned int[NUM_CHAN_PER_MOD];
		nEventsTotal[i] = new unsigned int[NUM_CHAN_PER_MOD];
		calcEventRate[i] = new double[NUM_CHAN_PER_MOD];
		inputCountRate[i] = new double[NUM_CHAN_PER_MOD];
		outputCountRate[i] = new double[NUM_CHAN_PER_MOD];
	}

	for(unsigned int i = 0; i < numCards; i++){
		for(unsigned int j = 0; j < NUM_CHAN_PER_MOD; j++){
			nEventsDelta[i][j] = 0;
			nEventsTotal[i][j] = 0;
			calcEventRate[i][j] = 0.0;
			inputCountRate[i][j] = 0.0;
			outputCountRate[i][j] = 0.0;
		}
	}

	// Define all the 1d arrays
	dataDelta = new size_t[numCards];
	dataTotal = new size_t[numCards];

	timeElapsed = 0.0;
	totalTime = 0.0;
	dumpTime = 3.0; // Minimum of 2 seconds between updates
	
	is_able_to_send = true;

	client = new Client();
	if(!client->Init("127.0.0.1", 5556)){
		is_able_to_send = false;
	}

	Clear();
}

StatsHandler::~StatsHandler(){
	client->SendMessage((char *)"$KILL_SOCKET", 13);
	client->Close();
	
	// De-allocate the 2d arrays
	for(unsigned int i = 0; i < numCards; i++){
		delete[] nEventsDelta[i];
		delete[] nEventsTotal[i];
		delete[] calcEventRate[i];
		delete[] inputCountRate[i];
		delete[] outputCountRate[i];
	}
	delete[] nEventsDelta;
	delete[] nEventsTotal;
	delete[] calcEventRate;
	delete[] inputCountRate;
	delete[] outputCountRate;
	
	// De-allocate the 1d arrays
	delete[] dataDelta;
	delete[] dataTotal;
}

void StatsHandler::AddEvent(unsigned int mod, unsigned int ch, size_t size, int delta_/*=1*/){
	if(mod >= numCards){
		std::cout << "Bad module " << mod << ", numCards = " << numCards << std::endl;
		return;
	}
	if(ch >= (unsigned)NUM_CHAN_PER_MOD){
		std::cout << "Bad channel " << ch << std::endl;
		return;
	}
	nEventsDelta[mod][ch] += delta_;
	nEventsTotal[mod][ch] += delta_;
	dataDelta[mod] += size;
	dataTotal[mod] += size;
}

/**
 *	\return Returns true if the dump interval is exceeded.
 */
bool StatsHandler::AddTime(double dtime) {
	timeElapsed += dtime;
	totalTime   += dtime;

	return (timeElapsed >= dumpTime);
   
}

void StatsHandler::Dump(void){
	if(!is_able_to_send){ return; }

	double dataRate = GetTotalDataRate();

	// Below is the stats packet structure (for N modules)
	// ---------------------------------------------------
	// 4 byte total number of pixie modules (N)
	// 8 byte total time of run (in seconds)
	// 8 byte total data rate (in B/s)
	// channel 0, 0 rate
	// channel 0, 0 total
	// channel 0, 1 rate
	// channel 0, 1 total
	// ...
	// channel 0, 15 rate
	// channel 0, 15 total
	// channel 1, 0 rate
	// channel 1, 0 total
	// ...
	// channel N-1, 15 rate
	// channel N-1, 15 total
	//msg_size = 20 fixed bytes + 4 words/card/ch * numCards * 16 ch * 8 bytes/ word
	size_t msg_size = sizeof(numCards) + sizeof(totalTime) + sizeof(dataRate) + numCards*16*(sizeof(inputCountRate[0][0])+sizeof(outputCountRate[0][0])+sizeof(calcEventRate[0][0])+sizeof(nEventsTotal[0][0]));
	char *message = new char[msg_size];
	char *ptr = message;
	
	// Construct the rate message
	memcpy(ptr, &numCards, 4); ptr += 4;
	memcpy(ptr, &totalTime, 8); ptr += 8;
	memcpy(ptr, &dataRate, 8); ptr += 8;
	for (unsigned int i=0; i < numCards; i++) {
		for (unsigned int j=0; j < NUM_CHAN_PER_MOD; j++) {	 
			calcEventRate[i][j] = nEventsDelta[i][j] / timeElapsed;
			if (timeElapsed<=0) 
				calcEventRate[i][j] = 0;
			memcpy(ptr, &inputCountRate[i][j], sizeof(inputCountRate[i][j])); ptr += sizeof(inputCountRate[i][j]);
			memcpy(ptr, &outputCountRate[i][j], sizeof(outputCountRate[i][j])); ptr += sizeof(outputCountRate[i][j]);
			memcpy(ptr, &calcEventRate[i][j], sizeof(calcEventRate[i][j])); ptr += sizeof(calcEventRate[i][j]);
			memcpy(ptr, &nEventsTotal[i][j], sizeof(nEventsTotal[i][j])); ptr += sizeof(nEventsTotal[i][j]);
		} //Update the status bar
	}
	
	client->SendMessage(message, msg_size);
	
	delete[] message;
}

double StatsHandler::GetDataRate(size_t mod){
	if(timeElapsed<=0) return 0;
	return dataDelta[mod] / timeElapsed;
}

double StatsHandler::GetTotalDataRate(){
	double rate = 0;
	for (unsigned int i=0; i < numCards; i++) rate += GetDataRate(i);
	return rate;
}

double StatsHandler::GetEventRate(size_t mod){
	double output = 0.0;
	for(unsigned int i = 0; i < numCards; i++){
		output += calcEventRate[mod][i];
	}
	return output;
}

double StatsHandler::GetTotalTime() {
	return totalTime;
}

void StatsHandler::ClearRates(){
	timeElapsed = 0;

	for(size_t i=0; i < numCards; i++){
		for(size_t j = 0; j < NUM_CHAN_PER_MOD; j++){
			nEventsDelta[i][j] = 0;
		}
		dataDelta[i] = 0;
	}
}

void StatsHandler::SetXiaRates(int mod, std::vector<std::pair<double, double> > *xiaRates) {
	for (int ch = 0; ch < NUM_CHAN_PER_MOD; ch++) {
		inputCountRate[mod][ch] = xiaRates->at(ch).first;
		outputCountRate[mod][ch] = xiaRates->at(ch).second;
	}
}
void StatsHandler::ClearTotals(){
	totalTime = 0;
	for(size_t i=0; i < numCards; i++){
		for(size_t j = 0; j < NUM_CHAN_PER_MOD; j++){
			nEventsTotal[i][j] = 0;
		}
	}
}

void StatsHandler::Clear(){
	ClearRates();
	ClearTotals();
}
