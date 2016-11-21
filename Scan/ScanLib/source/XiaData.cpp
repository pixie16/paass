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

/// Compute the trace baseline, baseline standard deviation, and find the pulse maximum.
float ChannelEvent::ComputeBaseline(){
	if(size == 0){ return -9999; }
	if(baseline > 0){ return baseline; }

	// Find the baseline.
	baseline = 0.0;
	size_t sample_size = (15 <= size ? 15:size);
	for(size_t i = 0; i < sample_size; i++){
		baseline += event->adcTrace[i];
	}
	baseline = float(baseline)/sample_size;
	
	// Calculate the standard deviation.
	stddev = 0.0;
	for(size_t i = 0; i < sample_size; i++){
		stddev += (event->adcTrace[i] - baseline)*(event->adcTrace[i] - baseline);
	}
	stddev = std::sqrt((1.0/sample_size) * stddev);
	
	// Find the maximum value and the maximum bin.
	maximum = -9999.0;
	for(size_t i = 0; i < size; i++){
		if(event->adcTrace[i]-baseline > maximum){ 
			maximum = event->adcTrace[i]-baseline;
			max_index = i;
		}
	}

	// Find the pulse maximum by fitting with a third order polynomial.
	if(event->adcTrace[max_index-1] >= event->adcTrace[max_index+1]) // Favor the left side of the pulse.
		maximum = calculateP3(max_index-2, event->adcTrace, cfdPar[0], cfdPar[1], cfdPar[2], cfdPar[3]) - baseline;
	else // Favor the right side of the pulse.
		maximum = calculateP3(max_index-1, event->adcTrace, cfdPar[0], cfdPar[1], cfdPar[2], cfdPar[3]) - baseline;

	return baseline;
}

float ChannelEvent::FindLeadingEdge(const float &thresh_/*=0.05*/){
	if(!event || ComputeBaseline() < 0){ return -9999; }
	else if(phase >= 0.0){ return phase; }

	// Check if this is a valid pulse
	if(maximum <= 0 || max_index == 0){ return -9999; }

	for(size_t index = max_index; index > 0; index--){
		if(event->adcTrace[index] <= thresh_ * maximum){ 
			// Interpolate and return the value
			// y = thresh_ * maximum
			// x = (x1 + (y-y1)/(y2-y1))
			// x1 = index, x2 = index+1
			// y1 = event->adcTrace[index], y2 = event->adcTrace[index+1]
			if(event->adcTrace[index+1] == event->adcTrace[index]){ return index+1; }
			else{ return (phase = (index + (thresh_ * maximum - event->adcTrace[index])/(event->adcTrace[index+1] - event->adcTrace[index]))); }
		}
	}
	
	return -9999;
}

float ChannelEvent::IntegratePulse(const size_t &start_/*=0*/, const size_t &stop_/*=0*/){
	if(!event || ComputeBaseline() < 0){ return -9999; }
	
	size_t stop = (stop_ == 0?size:stop_);
	
	qdc = 0.0;
	for(size_t i = start_+1; i < stop; i++){ // Integrate using trapezoidal rule.
		qdc += 0.5*(event->adcTrace[i-1] + event->adcTrace[i]) - baseline;
	}

	return qdc;
}

/// Perform CFD analysis on the waveform.
float ChannelEvent::AnalyzeXiaCFD(const float &F_/*=0.5*/, const size_t &D_/*=1*/, const size_t &L_/*=1*/){
	if(size == 0 || baseline < 0){ return -9999; }
	if(!cfdvals)
		cfdvals = new float[size];
	
	float cfdMinimum = 9999;
	size_t cfdMinIndex = 0;
	
	phase = -9999;

	// Compute the cfd waveform.
	for(size_t cfdIndex = 0; cfdIndex < size; ++cfdIndex){
		cfdvals[cfdIndex] = 0.0;
		if(cfdIndex >= L_ + D_ - 1){
			for(size_t i = 0; i < L_; i++)
				cfdvals[cfdIndex] += F_ * (event->adcTrace[cfdIndex - i]-baseline) - (event->adcTrace[cfdIndex - i - D_]-baseline);
		}
		if(cfdvals[cfdIndex] < cfdMinimum){
			cfdMinimum = cfdvals[cfdIndex];
			cfdMinIndex = cfdIndex;
		}
	}

	// Find the zero-crossing.
	if(cfdMinIndex > 0){
		// Find the zero-crossing.
		for(size_t cfdIndex = cfdMinIndex-1; cfdIndex >= 0; cfdIndex--){
			if(cfdvals[cfdIndex] >= 0.0 && cfdvals[cfdIndex+1] < 0.0){
				phase = cfdIndex - cfdvals[cfdIndex]/(cfdvals[cfdIndex+1]-cfdvals[cfdIndex]);
				break;
			}
		}
	}

	return phase;
}

/// Perform CFD analysis on the waveform.
float ChannelEvent::AnalyzeCFD(const float &F_/*=0.5*/){
	if(size == 0 || baseline < 0){ return -9999; }

	float threshold = F_*maximum + baseline;

	phase = -9999;
	for(size_t cfdIndex = max_index; cfdIndex > 0; cfdIndex--){
		if(event->adcTrace[cfdIndex-1] < threshold && event->adcTrace[cfdIndex] >= threshold){
			float p0, p1, p2;

			// Fit the rise of the trace to a 2nd order polynomial.
			calculateP2(cfdIndex-1, event->adcTrace, p0, p1, p2);
			
			// Calculate the phase of the trace.
			phase = (-p1+std::sqrt(p1*p1 - 4*p2*(p0 - threshold)))/(2*p2);

			break;
		}
	}

	return phase;
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
	cfdCrossing = -9999;
	max_index = 0;

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

float calculateP2(const short &x0, const std::vector<int> &trace, float &p0, float &p1, float &p2){
	float x1[3], x2[3];
	for(size_t i = 0; i < 3; i++){
		x1[i] = (x0+i);
		x2[i] = std::pow(x0+i, 2);
	}

	// messy
	const int *y = &trace.data()[x0];

	float denom = 1*(x1[1]*x2[2]-x2[1]*x1[2]) - x1[0]*(1*x2[2]-x2[1]*1) + x2[0]*(1*x1[2]-x1[1]*1);

	p0 = (y[0]*(x1[1]*x2[2]-x2[1]*x1[2]) - x1[0]*(y[1]*x2[2]-x2[1]*y[2]) + x2[0]*(y[1]*x1[2]-x1[1]*y[2]))/denom;
	p1 = (1*(y[1]*x2[2]-x2[1]*y[2]) - y[0]*(1*x2[2]-x2[1]*1) + x2[0]*(1*y[2]-y[1]*1))/denom;
	p2 = (1*(x1[1]*y[2]-y[1]*x1[2]) - x1[0]*(1*y[2]-y[1]*1) + y[0]*(1*x1[2]-x1[1]*1))/denom;

	// Calculate the maximum of the polynomial.
	return (p0 - p1*p1/(4*p2));
}

float calculateP3(const short &x0, const std::vector<int> &trace, float &p0, float &p1, float &p2, float &p3){
	float x1[4], x2[4], x3[4];
	for(size_t i = 0; i < 4; i++){
		x1[i] = (x0+i);
		x2[i] = std::pow(x0+i, 2);
		x3[i] = std::pow(x0+i, 3);
	}

	// messy
	const int *y = &trace.data()[x0];

	float denom = 1*(x1[1]*(x2[2]*x3[3]-x2[3]*x3[2]) - x1[2]*(x2[1]*x3[3]-x2[3]*x3[1]) + x1[3]*(x2[1]*x3[2]-x2[2]*x3[1])) - x1[0]*(1*(x2[2]*x3[3]-x2[3]*x3[2]) - 1*(x2[1]*x3[3]-x2[3]*x3[1]) + 1*(x2[1]*x3[2]-x2[2]*x3[1])) + x2[0]*(1*(x1[2]*x3[3]-x1[3]*x3[2]) - 1*(x1[1]*x3[3]-x1[3]*x3[1]) + 1*(x1[1]*x3[2]-x1[2]*x3[1])) - x3[0]*(1*(x1[2]*x2[3]-x1[3]*x2[2]) - 1*(x1[1]*x2[3]-x1[3]*x2[1]) + 1*(x1[1]*x2[2]-x1[2]*x2[1]));

	p0 = (y[0]*(x1[1]*(x2[2]*x3[3]-x2[3]*x3[2]) - x1[2]*(x2[1]*x3[3]-x2[3]*x3[1]) + x1[3]*(x2[1]*x3[2]-x2[2]*x3[1])) - x1[0]*(y[1]*(x2[2]*x3[3]-x2[3]*x3[2]) - y[2]*(x2[1]*x3[3]-x2[3]*x3[1]) + y[3]*(x2[1]*x3[2]-x2[2]*x3[1])) + x2[0]*(y[1]*(x1[2]*x3[3]-x1[3]*x3[2]) - y[2]*(x1[1]*x3[3]-x1[3]*x3[1]) + y[3]*(x1[1]*x3[2]-x1[2]*x3[1])) - x3[0]*(y[1]*(x1[2]*x2[3]-x1[3]*x2[2]) - y[2]*(x1[1]*x2[3]-x1[3]*x2[1]) + y[3]*(x1[1]*x2[2]-x1[2]*x2[1])))/denom;
	p1 = (1*(y[1]*(x2[2]*x3[3]-x2[3]*x3[2]) - y[2]*(x2[1]*x3[3]-x2[3]*x3[1]) + y[3]*(x2[1]*x3[2]-x2[2]*x3[1])) - y[0]*(1*(x2[2]*x3[3]-x2[3]*x3[2]) - 1*(x2[1]*x3[3]-x2[3]*x3[1]) + 1*(x2[1]*x3[2]-x2[2]*x3[1])) + x2[0]*(1*(y[2]*x3[3]-y[3]*x3[2]) - 1*(y[1]*x3[3]-y[3]*x3[1]) + 1*(y[1]*x3[2]-y[2]*x3[1])) - x3[0]*(1*(y[2]*x2[3]-y[3]*x2[2]) - 1*(y[1]*x2[3]-y[3]*x2[1]) + 1*(y[1]*x2[2]-y[2]*x2[1])))/denom;
	p2 = (1*(x1[1]*(y[2]*x3[3]-y[3]*x3[2]) - x1[2]*(y[1]*x3[3]-y[3]*x3[1]) + x1[3]*(y[1]*x3[2]-y[2]*x3[1])) - x1[0]*(1*(y[2]*x3[3]-y[3]*x3[2]) - 1*(y[1]*x3[3]-y[3]*x3[1]) + 1*(y[1]*x3[2]-y[2]*x3[1])) + y[0]*(1*(x1[2]*x3[3]-x1[3]*x3[2]) - 1*(x1[1]*x3[3]-x1[3]*x3[1]) + 1*(x1[1]*x3[2]-x1[2]*x3[1])) - x3[0]*(1*(x1[2]*y[3]-x1[3]*y[2]) - 1*(x1[1]*y[3]-x1[3]*y[1]) + 1*(x1[1]*y[2]-x1[2]*y[1])))/denom;
	p3 = (1*(x1[1]*(x2[2]*y[3]-x2[3]*y[2]) - x1[2]*(x2[1]*y[3]-x2[3]*y[1]) + x1[3]*(x2[1]*y[2]-x2[2]*y[1])) - x1[0]*(1*(x2[2]*y[3]-x2[3]*y[2]) - 1*(x2[1]*y[3]-x2[3]*y[1]) + 1*(x2[1]*y[2]-x2[2]*y[1])) + x2[0]*(1*(x1[2]*y[3]-x1[3]*y[2]) - 1*(x1[1]*y[3]-x1[3]*y[1]) + 1*(x1[1]*y[2]-x1[2]*y[1])) - y[0]*(1*(x1[2]*x2[3]-x1[3]*x2[2]) - 1*(x1[1]*x2[3]-x1[3]*x2[1]) + 1*(x1[1]*x2[2]-x1[2]*x2[1])))/denom;

	// Calculate the maximum of the polynomial.
	float xmax1 = (-2*p2+std::sqrt(4*p2*p2-12*p3*p1))/(6*p3);
	float xmax2 = (-2*p2-std::sqrt(4*p2*p2-12*p3*p1))/(6*p3);

	if((2*p2+6*p3*xmax1) < 0) // The second derivative is negative (i.e. this is a maximum).
		return (p0 + p1*xmax1 + p2*xmax1*xmax1 + p3*xmax1*xmax1*xmax1);

	return (p0 + p1*xmax2 + p2*xmax2*xmax2 + p3*xmax2*xmax2*xmax2);
}
