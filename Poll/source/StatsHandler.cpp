#include <fstream>
#include <iostream>
#include <numeric>
#include <vector>

#include "Utility.h"
#include "StatsHandler.hpp"

using namespace std;

const string StatsHandler::dumpFile = "/tmp/rates.txt";

StatsHandler::StatsHandler(size_t nCards) :
    nEventsDelta(nCards), nEventsTotal(nCards),
    dataDelta(nCards, 0), dataTotal(nCards, 0),
    calcEventRate(nCards), calcDataRate(nCards, 0),
    timeElapsed(0.), totalTime(0.), dumpTime(2.),
    numCards(nCards)
{
    // do nothing else
}

StatsHandler::~StatsHandler()
{
    // do nothing
}

void StatsHandler::AddEvent(unsigned int mod, unsigned int ch, size_t size)
{
    if (!inRange(mod, 0U, numCards)) {
	cout << "Bad module " << mod << endl;
	return;
    }
    if (!inRange(mod, 0U, (unsigned)NUMBER_OF_CHANNELS)) {
	cout << "Bad channel " << ch << endl;
	return;
    }
    nEventsDelta.at(mod).at(ch)++;
    nEventsTotal.at(mod).at(ch)++;
    dataDelta.at(mod) += size;
    dataTotal.at(mod) += size;
}

void StatsHandler::AddTime(double dtime) 
{
    timeElapsed += dtime;
    totalTime   += dtime;
   
    if (timeElapsed >= dumpTime) {
	Dump();
	timeElapsed = 0;
	for (size_t i=0; i < numCards; i++) {
	    nEventsDelta.at(i).assign(0);
	}
	dataDelta.assign(numCards, 0);
    }
}

void StatsHandler::Dump(void)
{
    ofstream out(dumpFile.c_str(), ios::trunc);

    out.precision(2);
    out.setf(ios::fixed, ios::floatfield);

    double dataRate = accumulate(dataDelta.begin(), dataDelta.end(), 0);
    dataRate /= timeElapsed;

    out << "totalTime = " << totalTime << '\n';
    out << "deltaTime = " << timeElapsed 
	<< ", dataRate = " << dataRate << '\n';
    out << "mod\tch\trate\ttotal\n";
    for (unsigned int i=0; i < numCards; i++) {
	calcDataRate.at(i) = (size_t)(dataDelta.at(i) / timeElapsed);
	for (unsigned int j=0; j < NUMBER_OF_CHANNELS; j++) {	 
	    calcEventRate.at(i).at(j) = nEventsDelta.at(i).at(j) / timeElapsed;
	    out << i << '\t' << j << '\t' 
		<< calcEventRate.at(i).at(j) << '\t'
		<< nEventsTotal.at(i).at(j) << '\n';
	}//Update the status bar
    }
    out.close();
}

double StatsHandler::GetDataRate(size_t mod)
{
    return calcDataRate.at(mod);
}
double StatsHandler::GetTotalDataRate()
{
	double rate = 0;
	for (unsigned int i=0; i < numCards; i++) rate += calcDataRate.at(i);
	return rate;
}

double StatsHandler::GetEventRate(size_t mod)
{
    return accumulate( calcEventRate.at(mod).begin(),
		       calcEventRate.at(mod).end(), 0. );
}
double StatsHandler::GetTotalTime() {
	return totalTime;
}
void StatsHandler::Clear() {
	totalTime = 0;
	timeElapsed = 0;

	for (size_t i=0; i < numCards; i++) {
	    nEventsDelta.at(i).assign(0);
	}
	dataDelta.assign(numCards, 0);
}
