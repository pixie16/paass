#include <cmath>

#include "HelperFunctions.hpp"
#include "XiaData.hpp"

/////////////////////////////////////////////////////////////////////
// XiaData
/////////////////////////////////////////////////////////////////////

/// Default constructor.
XiaData::XiaData(){
	clear();
}

/// Constructor from a pointer to another XiaData.
XiaData::XiaData(XiaData *other_){
	adcTrace = other_->adcTrace;

	energy = other_->energy; 
	time = other_->time;
	
	for(int i = 0; i < numQdcs; i++){
		qdcValue[i] = other_->qdcValue[i];
	}

	modNum = other_->modNum;
	chanNum = other_->chanNum;
	trigTime = other_->trigTime;
	cfdTime = other_->cfdTime;
	eventTimeLo = other_->eventTimeLo;
	eventTimeHi = other_->eventTimeHi;
	eventTime = other_->eventTime;

	virtualChannel = other_->virtualChannel;
	pileupBit = other_->pileupBit;
	saturatedBit = other_->saturatedBit;
	cfdForceTrig = other_->cfdForceTrig; 
	cfdTrigSource = other_->cfdTrigSource; 
}

XiaData::~XiaData(){
}

void XiaData::reserve(const size_t &size_){
	if(size_ == 0){ return; }
	adcTrace.reserve(size_);
}

void XiaData::assign(const size_t &size_, const int &input_){
	adcTrace.assign(size_, input_);
}

void XiaData::push_back(const int &input_){
	adcTrace.push_back(input_);
}

void XiaData::clear(){
	adcTrace.clear();

	energy = 0.0; 
	time = 0.0;
	
	for(int i = 0; i < numQdcs; i++){
		qdcValue[i] = 0;
	}

	modNum = 0;
	chanNum = 0;
	trigTime = 0;
	cfdTime = 0;
	eventTimeLo = 0;
	eventTimeHi = 0;
	eventTime = 0.0;

	virtualChannel = false;
	pileupBit = false;
	saturatedBit = false;
	cfdForceTrig = false; 
	cfdTrigSource = false; 
}

/////////////////////////////////////////////////////////////////////
// ChannelEvent
/////////////////////////////////////////////////////////////////////

/// Default constructor.
ChannelEvent::ChannelEvent(){
	event = NULL;
	cfdvals = NULL;
	Clear();
}

/// Constructor from a XiaData. ChannelEvent will take ownership of the XiaData.
ChannelEvent::ChannelEvent(XiaData *event_){
	event = NULL;
	cfdvals = NULL;
	Clear();
	event = event_;
	size = event->adcTrace.size();
}

ChannelEvent::~ChannelEvent(){
	if(event){ delete event; }
	if(cfdvals){ delete[] cfdvals; }
}

void ChannelEvent::Clear(){
	phase = -9999;
	maximum = -9999;
	baseline = -9999;
	stddev = -9999;
	qdc = -9999;
	cfdCrossing = -9999;
	max_index = 0;
	cfdIndex = 0;

	hires_energy = -9999;
	hires_time = -9999;

	valid_chan = false;
	ignore = false;

	size = 0;
	if(cfdvals){ delete[] cfdvals; }
	if(event){ event->clear(); }

	event = NULL;
	cfdvals = NULL;
}