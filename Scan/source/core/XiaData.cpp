#include <cmath>

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
	xvals = NULL;
	yvals = NULL;
	Clear();
}

/// Constructor from a XiaData. ChannelEvent will take ownership of the XiaData.
ChannelEvent::ChannelEvent(XiaData *event_){
	event = NULL;
	xvals = NULL;
	yvals = NULL;
	Clear();
	event = event_;
	size = event->adcTrace.size();
	if(size != 0){
		xvals = new float[size];
		yvals = new float[size];
	}
}

ChannelEvent::~ChannelEvent(){
	if(event){ delete event; }
	if(xvals){ delete[] xvals; }
	if(yvals){ delete[] yvals; }
}

float ChannelEvent::CorrectBaseline(){
	if(!event || size == 0){ return -9999; }
	else if(baseline_corrected){ return maximum; }

	// Find the baseline
	baseline = 0.0;
	size_t sample_size = (10 <= size ? 10:size);
	for(size_t i = 0; i < sample_size; i++){
		baseline += (float)event->adcTrace[i];
	}
	baseline = baseline/sample_size;
	
	// Calculate the standard deviation
	stddev = 0.0;
	for(size_t i = 0; i < sample_size; i++){
		stddev += ((float)event->adcTrace[i] - baseline)*((float)event->adcTrace[i] - baseline);
	}
	stddev = std::sqrt((1.0/sample_size) * stddev);
	
	// Find the maximum value, the maximum bin, and correct the baseline
	maximum = -9999.0;
	for(size_t i = 0; i < event->adcTrace.size(); i++){
		xvals[i] = i;
		yvals[i] = event->adcTrace[i]-baseline;
		if(yvals[i] > maximum){ 
			maximum = yvals[i];
			max_index = i;
		}
	}
	
	baseline_corrected = true;
	
	return maximum;
}

float ChannelEvent::FindLeadingEdge(const float &thresh_/*=0.05*/){
	if(!event || (!baseline_corrected && CorrectBaseline() < 0)){ return -9999; }
	else if(phase >= 0.0){ return phase; }

	// Check if this is a valid pulse
	if(maximum <= 0 || max_index == 0){ return -9999; }

	for(size_t index = max_index; index > 0; index--){
		if(yvals[index] <= thresh_ * maximum){ 
			// Interpolate and return the value
			// y = thresh_ * maximum
			// x = (x1 + (y-y1)/(y2-y1))
			// x1 = index, x2 = index+1
			// y1 = yvals[index], y2 = yvals[index+1]
			if(yvals[index+1] == yvals[index]){ return index+1; }
			else{ return (phase = (index + (thresh_ * maximum - yvals[index])/(yvals[index+1] - yvals[index]))); }
		}
	}
	
	return -9999;
}

float ChannelEvent::IntegratePulse(const size_t &start_/*=0*/, const size_t &stop_/*=0*/){
	if(!event || (!baseline_corrected && CorrectBaseline() < 0)){ return -9999; }
	
	size_t stop = (stop_ == 0?size:stop_);
	
	qdc = 0.0;
	for(size_t i = start_+1; i < stop; i++){ // Integrate using trapezoidal rule.
		qdc += 0.5*(yvals[i-1] + yvals[i]);
	}

	return qdc;
}

float ChannelEvent::FindQDC(const size_t &start_/*=0*/, const size_t &stop_/*=0*/){
	if(qdc >= 0.0){ return qdc; }
	
	qdc = IntegratePulse(start_, stop_);

	return qdc;
}

void ChannelEvent::Clear(){
	phase = -9999;
	maximum = -9999;
	baseline = -9999;
	stddev = -9999;
	qdc = -9999;
	max_index = 0;

	hires_energy = -9999;
	hires_time = -9999;
	
	valid_chan = false;
	baseline_corrected = false;
	ignore = false;
	
	size = 0;
	if(xvals){ delete[] xvals; }
	if(yvals){ delete[] yvals; }
	if(event){ event->clear(); }
	
	event = NULL;
	xvals = NULL;
	yvals = NULL;
}
