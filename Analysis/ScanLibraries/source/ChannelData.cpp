

#include "ChannelData.hpp"

/////////////////////////////////////////////////////////////////////
// ChannelData
/////////////////////////////////////////////////////////////////////

/// Default constructor.
ChannelData::ChannelData(){
	event = NULL;
	xvals = NULL;
	yvals = NULL;
	Clear();
}

/// Constructor from a XiaData. ChannelData will take ownership of the XiaData.
ChannelData::ChannelData(XiaData *event_){
	event = NULL;
	xvals = NULL;
	yvals = NULL;
	Clear();
	event = event_;
	size = event->GetTrace().size();
	if(size != 0){
		xvals = new float[size];
		yvals = new float[size];
	}
}

ChannelData::~ChannelData(){
	if(event){ delete event; }
	if(xvals){ delete[] xvals; }
	if(yvals){ delete[] yvals; }
}

// float ChannelData::FindLeadingEdge(const float &thresh_/*=0.05*/){
// 	if(!event || (!baseline_corrected && CorrectBaseline() < 0)){ return -9999; }
// 	else if(phase >= 0.0){ return phase; }

// 	// Check if this is a valid pulse
// 	if(maximum <= 0 || max_index == 0){ return -9999; }

// 	for(size_t index = max_index; index > 0; index--){
// 		if(yvals[index] <= thresh_ * maximum){ 
// 			// Interpolate and return the value
// 			// y = thresh_ * maximum
// 			// x = (x1 + (y-y1)/(y2-y1))
// 			// x1 = index, x2 = index+1
// 			// y1 = yvals[index], y2 = yvals[index+1]
// 			if(yvals[index+1] == yvals[index]){ return index+1; }
// 			else{ return (phase = (index + (thresh_ * maximum - yvals[index])/(yvals[index+1] - yvals[index]))); }
// 		}
// 	}
	
// 	return -9999;
// }

void ChannelData::Clear() {
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
	if(event){ event->Clear(); }
	
	event = NULL;
	xvals = NULL;
	yvals = NULL;
}
