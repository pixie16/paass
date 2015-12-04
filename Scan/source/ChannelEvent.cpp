#include <cmath>

#include "ChannelEvent.hpp"

ChannelEvent::ChannelEvent(){
	xvals = NULL;
	yvals = NULL;
	Clear();
}

ChannelEvent::~ChannelEvent(){
	if(xvals){ delete[] xvals; }
	if(yvals){ delete[] yvals; }
}

void ChannelEvent::reserve(const size_t &size_){
	if(size != 0){ return; }
	size = size_;
	xvals = new float[size];
	yvals = new float[size];
	trace.reserve(size);
}

void ChannelEvent::assign(const size_t &size_, const int &input_){
	trace.assign(size_, input_);
}

void ChannelEvent::push_back(const int &input_){
	trace.push_back(input_);
}

float ChannelEvent::CorrectBaseline(){
	if(size == 0){ return -9999; }
	else if(baseline_corrected){ return maximum; }

	// Find the baseline
	baseline = 0.0;
	size_t sample_size = (10 <= size ? 10:size);
	for(size_t i = 0; i < sample_size; i++){
		baseline += (float)trace[i];
	}
	baseline = baseline/sample_size;
	
	// Calculate the standard deviation
	stddev = 0.0;
	for(size_t i = 0; i < sample_size; i++){
		stddev += ((float)trace[i] - baseline)*((float)trace[i] - baseline);
	}
	stddev = std::sqrt((1.0/sample_size) * stddev);
	
	// Find the maximum value, the maximum bin, and correct the baseline
	maximum = -9999.0;
	for(size_t i = 0; i < trace.size(); i++){
		xvals[i] = i;
		yvals[i] = trace[i]-baseline;
		if(yvals[i] > maximum){ 
			maximum = yvals[i];
			max_index = i;
		}
	}
	
	baseline_corrected = true;
	
	return maximum;
}

float ChannelEvent::FindLeadingEdge(const float &thresh_/*=0.05*/){
	if(!baseline_corrected && CorrectBaseline() < 0){ return -9999; }
	else if(phase != 0.0){ return phase; }

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
	if(!baseline_corrected && CorrectBaseline() < 0){ return -9999; }
	
	size_t stop = (stop_ == 0?size:stop_);
	
	qdc = 0.0;
	for(size_t i = start_+1; i < stop; i++){ // Integrate using trapezoidal rule.
		qdc += 0.5*(yvals[i-1] + yvals[i]);
	}

	return qdc;
}

float ChannelEvent::FindQDC(const size_t &start_/*=0*/, const size_t &stop_/*=0*/){
	if(qdc != 0.0){ return qdc; }
	
	qdc = IntegratePulse(start_, stop_);

	return qdc;
}

void ChannelEvent::Clear(){
	if(xvals){ delete[] xvals; }
	if(yvals){ delete[] yvals; }
	trace.clear();
	size = 0;

	energy = 0.0; 
	time = 0.0;
	
	hires_energy = 0.0;
	hires_time = 0.0;
	valid_chan = false;
	
	for(int i = 0; i < numQdcs; i++){
		qdcValue[i] = 0;
	}

	modNum = 0;
	chanNum = 0;
	trigTime = 0;
	cfdTime = 0;
	eventTimeLo = 0;
	eventTimeHi = 0;

	phase = 0.0;
	maximum = 0.0;
	baseline = 0.0;
	stddev = 0.0;
	qdc = 0.0;
	max_index = 0;

	virtualChannel = false;
	pileupBit = false;
	saturatedBit = false;
	baseline_corrected = false;
	ignore = false;
}
