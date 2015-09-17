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
	for(unsigned int i = 0; i < numCards; i++){
		nEventsDelta[i] = new unsigned int[NUM_CHAN_PER_MOD];
		nEventsTotal[i] = new unsigned int[NUM_CHAN_PER_MOD];
		calcEventRate[i] = new double[NUM_CHAN_PER_MOD];
	}

	for(unsigned int i = 0; i < numCards; i++){
		for(unsigned int j = 0; j < NUM_CHAN_PER_MOD; j++){
			nEventsDelta[i][j] = 0;
			nEventsTotal[i][j] = 0;
			calcEventRate[i][j] = 0.0;
		}
	}

	// Define all the 1d arrays
	dataDelta = new size_t[numCards];
	dataTotal = new size_t[numCards];
	calcDataRate = new size_t[numCards];

	timeElapsed = 0.0;
	totalTime = 0.0;
	dumpTime = 2.0; // Minimum of 2 seconds between updates
	
	is_able_to_send = true;

	client = new Client();
	if(!client->Init("127.0.0.1", 5556)){
		is_able_to_send = false;
	}
}

StatsHandler::~StatsHandler(){
	client->SendMessage((char *)"$KILL_SOCKET", 13);
	client->Close();
	
	// De-allocate the 2d arrays
	for(unsigned int i = 0; i < numCards; i++){
		delete[] nEventsDelta[i];
		delete[] nEventsTotal[i];
		delete[] calcEventRate[i];
	}
	delete[] nEventsDelta;
	delete[] nEventsTotal;
	delete[] calcEventRate;
	
	// De-allocate the 1d arrays
	delete[] dataDelta;
	delete[] dataTotal;
	delete[] calcDataRate;
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

void StatsHandler::AddTime(double dtime) {
	timeElapsed += dtime;
	totalTime   += dtime;
   
	if (timeElapsed >= dumpTime) {
		Dump();
		timeElapsed = 0;
		for(size_t i = 0; i < numCards; i++){
			for(size_t j = 0; j < NUM_CHAN_PER_MOD; j++){
				nEventsDelta[i][j] = 0;
			}
			dataDelta[i] = 0;
		}
	}
}

void StatsHandler::Dump(void){
	if(!is_able_to_send){ return; }

	double dataRate = 0.0;
	for(unsigned int i = 0; i < numCards; i++){
		dataRate += dataDelta[i];
	}
	dataRate /= timeElapsed;

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
	size_t msg_size = 20 + 2*numCards*16*8;
	char *message = new char[msg_size];
	char *ptr = message;
	
	// Construct the rate message
	memcpy(ptr, &numCards, 4); ptr += 4;
	memcpy(ptr, &totalTime, 8); ptr += 8;
	memcpy(ptr, &dataRate, 8); ptr += 8;
	for (unsigned int i=0; i < numCards; i++) {
		calcDataRate[i] = (size_t)(dataDelta[i] / timeElapsed);
		for (unsigned int j=0; j < NUM_CHAN_PER_MOD; j++) {	 
			calcEventRate[i][j] = nEventsDelta[i][j] / timeElapsed;
			memcpy(ptr, &calcEventRate[i][j], 8); ptr += 8;
			memcpy(ptr, &nEventsTotal[i][j], 4); ptr += 4;
		} //Update the status bar
	}
	
	client->SendMessage(message, msg_size);
	
	delete[] message;
}

double StatsHandler::GetDataRate(size_t mod){
	return calcDataRate[mod];
}

double StatsHandler::GetTotalDataRate(){
	double rate = 0;
	for (unsigned int i=0; i < numCards; i++) rate += calcDataRate[i];
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

void StatsHandler::Clear(){
	totalTime = 0;
	timeElapsed = 0;

	for(size_t i=0; i < numCards; i++){
		for(size_t j = 0; j < NUM_CHAN_PER_MOD; j++){
			nEventsDelta[i][j] = 0;
		}
		dataDelta[i] = 0;
		calcDataRate[i] = 0;
	}
}
