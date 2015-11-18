#include <iostream>

// PixieCore libraries
#include "ScanMain.hpp"
#include "ChannelEvent.hpp"

// Local files
#include "filterer.hpp"

// Root files
#include "TApplication.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TFile.h"
#include "TAxis.h"

#define ADC_CLOCK_uSEC 4E-3 // us
#define ADC_TIME_STEP 4 // ns
#define SLEEP_WAIT 1E4 // When not in shared memory mode, length of time to wait between calls to gSystem->ProcessEvents (in us).

void Filterer::Filter(float *trace_, const size_t &length_, float *filtered1, const unsigned int &risetime_, const unsigned int &flattop_){
	float sum;
	size_t a1, a2, a3, a4;
	
	for(size_t i = 0; i < 2*risetime_ + flattop_; i++){
		filtered1[i] = 0;
	}
	
	for(size_t i = 2*risetime_ + flattop_; i < length_; i++){
		sum = 0.0;
		a1 = i - (2*risetime_ + flattop_);
		a2 = i - (risetime_ + flattop_);
		a3 = i - risetime_;
		a4 = i;
		for(size_t j = a1; j <= a2; j++){
			sum += -1*trace_[j];
		}
		for(size_t j = a3; j <= a4; j++){
			sum += trace_[j];
		}
		filtered1[i] = sum/risetime_;
	}
}

Filterer::Filterer(int mod /*= 0*/, int chan/*=0*/) :
	mod_(mod),
	chan_(chan), 
	acqRun_(true),
	singleCapture_(false),
	threshLow_(0),
	threshHigh_(-1),
	delay_(2),
	num_traces(0),
	num_displayed(0)
{
	time(&last_trace);

	trig_rise = 4;
	trig_flat = 0;
	
	energy_rise = 10;
	energy_flat = 10;
	
	// Variables for root graphics
	rootapp = new TApplication("filterer", 0, NULL);
	gSystem->Load("libTree");
	
	canvas = new TCanvas("filterer_canvas", "Filterer");
	canvas->cd();
	
	graph = new TGraph();
	f_fast = new TGraph();
	f_slow = new TGraph();
	
	f_fast->SetLineColor(2);
	f_slow->SetLineColor(3);
}

Filterer::~Filterer(){
	canvas->Close();
	delete canvas;
	delete graph;
	delete f_fast;
	delete f_slow;

	// Call Unpacker::Close() to finish cleanup.
	Close();
}

void Filterer::ResetGraph(unsigned int size) {
	if(size != x_vals.size()){
		std::cout << message_head << "Changing trace length from " << x_vals.size()*ADC_TIME_STEP << " to " << size*ADC_TIME_STEP << " ns.\n";
		delete fast_filter_y;
		delete slow_filter_y;
		fast_filter_y = new float[size];
		slow_filter_y = new float[size];
		x_vals.resize(size);
		for(size_t index = 0; index < x_vals.size(); index++){
			fast_filter_y[index] = 0.0;
			slow_filter_y[index] = 0.0;
			x_vals[index] = ADC_TIME_STEP * index;
		}	
		while ((unsigned int) graph->GetN() > size) graph->RemovePoint(graph->GetN());
	}

	std::stringstream stream;
	stream << "M" << mod_ << "C" << chan_;
	graph->SetTitle(stream.str().c_str());

	resetGraph_ = false;
}

void Filterer::Plot(ChannelEvent *event_){

	///The limits of the vertical axis
	static float axisMin, axisMax;

	// Draw the trace

	if(event_->size != x_vals.size()){ // The length of the trace has changed.
		resetGraph_ = true;
	}
	if (resetGraph_) {
		ResetGraph(event_->size);
		axisMax = 0;
		axisMin = 1E9;
	}
	
	// Run the fast filter (trigger) on the trace.
	Filter(event_->yvals, event_->size, fast_filter_y, trig_rise, trig_flat);
	
	// Run the slow filter (energy) on the trace.
	Filter(event_->yvals, event_->size, slow_filter_y, energy_rise, energy_flat);
	
	for (size_t i = 0; i < event_->size ; i++) {
		graph->SetPoint(i, x_vals[i], event_->yvals[i]);
		f_fast->SetPoint(i, x_vals[i], fast_filter_y[i]);
		f_slow->SetPoint(i, x_vals[i], slow_filter_y[i]);
	}

	if (graph->GetYaxis()->GetXmax() > axisMax) axisMax = graph->GetYaxis()->GetXmax(); 
	if (graph->GetYaxis()->GetXmin() < axisMin) axisMin = graph->GetYaxis()->GetXmin(); 
	
	if (f_fast->GetYaxis()->GetXmax() > axisMax) axisMax = f_fast->GetYaxis()->GetXmax(); 
	if (f_fast->GetYaxis()->GetXmin() < axisMin) axisMin = f_fast->GetYaxis()->GetXmin(); 
	
	if (f_slow->GetYaxis()->GetXmax() > axisMax) axisMax = f_slow->GetYaxis()->GetXmax(); 
	if (f_slow->GetYaxis()->GetXmin() < axisMin) axisMin = f_slow->GetYaxis()->GetXmin(); 
	
	graph->GetYaxis()->SetLimits(axisMin, axisMax);
	graph->GetYaxis()->SetRangeUser(axisMin, axisMax);

	graph->Draw("APL");
	f_fast->Draw("LSAME");
	f_slow->Draw("LSAME");

	canvas->Update();

	num_displayed++;

	num_traces++;
}

void Filterer::ProcessRawEvent(){
	ChannelEvent *current_event = NULL;
	
	// Fill the processor event deques with events
	while(!rawEvent.empty()){
		if(kill_all){ break; }
		//If the acquistion is not running we clear the events.
		//	For the SHM mode we simply break and wait for the next data packet.
		//	For the ldf mode we sleep and check acqRun again.
		while(!acqRun_) {
			//Clean up any stored waveforms.
			if(shm_mode || kill_all){ return; }
			usleep(SLEEP_WAIT);
			gSystem->ProcessEvents();
		}
	
		//Get the first event int he FIFO.
		current_event = rawEvent.front();
		rawEvent.pop_front();

		// Pass this event to the correct processor
		if(current_event->modNum == mod_ && current_event->chanNum == chan_){  
			current_event->CorrectBaseline();
			if (current_event->maximum < threshLow_) {
				delete current_event;
				continue;
			}
			//Check threhsold.
			if (threshHigh_ > threshLow_ && current_event->maximum > threshHigh_) {
				delete current_event;
				continue;
			}

			//Process the waveform.
			current_event->FindLeadingEdge();
			current_event->FindQDC();

			time_t cur_time;
			time(&cur_time);
			while(difftime(cur_time, last_trace) < delay_) {
				usleep(SLEEP_WAIT);
				gSystem->ProcessEvents();
				time(&cur_time);
			}	

			Plot(current_event); 
			
			//If this is a single capture we stop the plotting.
			if (singleCapture_) acqRun_ = false;

			time(&last_trace);
		}
		gSystem->ProcessEvents();
	}
}

bool Filterer::Initialize(std::string prefix_){
	if(init){ return false; }

	// Print a small welcome message.
	std::cout << prefix_ << "Displaying traces for mod = " << mod_ << ", chan = " << chan_ << ".\n";

	return (init = true);
}

/**
 * \param[in] prefix_
 */
void Filterer::ArgHelp(std::string prefix_){
	std::cout << prefix_ << "--mod [module]   | Module of signal of interest (default=0)\n";
	std::cout << prefix_ << "--chan [channel] | Channel of signal of interest (default=0)\n";
}

/** 
 *
 *	\param[in] prefix_ 
 */
void Filterer::CmdHelp(std::string prefix_){
	std::cout << prefix_ << "set [mod] [chan]    - Set the module and channel of signal of interest (default = 0, 0).\n";
	std::cout << prefix_ << "single              - Perform a single capture.\n";
	std::cout << prefix_ << "thresh [low] <high> - Set the plotting window for trace maximum.\n";
	std::cout << prefix_ << "delay [time]        - Set the delay between drawing traces (in seconds, default = 1 s).\n";
	std::cout << prefix_ << "log                 - Toggle log/linear mode on the y-axis.\n";
	std::cout << prefix_ << "trise [risetime]    - Set the length of the fast filter (in μs).\n";
	std::cout << prefix_ << "tflat [flattop]     - Set the gap of the fast filter (in μs).\n";
	std::cout << prefix_ << "erise [risetime]    - Set the length of the slow filter (in μs).\n";
	std::cout << prefix_ << "eflat [flattop]     - Set the gap of the slow filter (in μs).\n";
}

/**
 * \param args_
 * \param filename_
 */
bool Filterer::SetArgs(std::deque<std::string> &args_, std::string &filename_){
	std::string current_arg;
	while(!args_.empty()){
		current_arg = args_.front();
		args_.pop_front();

		if(current_arg == "--mod"){
			if(args_.empty()){
				std::cout << " Error: Missing required argument to option '--mod'!\n";
				return false;
			}
			mod_ = atoi(args_.front().c_str());
			args_.pop_front();
		}
		else if(current_arg == "--chan"){
			if(args_.empty()){
				std::cout << " Error: Missing required argument to option '--chan'!\n";
				return false;
			}
			chan_ = atoi(args_.front().c_str());
			args_.pop_front();
		}
		else{ filename_ = current_arg; }
	}
	
	return true;
}

bool Filterer::CommandControl(std::string cmd_, const std::vector<std::string> &args_){
	if(cmd_ == "set"){ // Toggle debug mode
		if(args_.size() == 2){
			//Set the module and channel.
			mod_ = atoi(args_.at(0).c_str());
			chan_ = atoi(args_.at(1).c_str());

			//Store previous run status.
			bool runStatus = acqRun_;
			//Stop the run to force the buffer to be cleared.
			acqRun_ = false;
			//Resotre the previous run status.
			acqRun_ = runStatus;
	
			resetGraph_ = true;
		}
		else{
			std::cout << message_head << "Invalid number of parameters to 'set'\n";
			std::cout << message_head << " -SYNTAX- set [module] [channel]\n";
		}
	}
	else if(cmd_ == "single") {
		singleCapture_ = !singleCapture_;
	}
	else if (cmd_ == "thresh") {
		if (args_.size() == 1) {
			threshLow_ = atoi(args_.at(0).c_str());
			threshHigh_ = -1;
		}
		else if (args_.size() == 2) {
			threshLow_ = atoi(args_.at(0).c_str());
			threshHigh_ = atoi(args_.at(1).c_str());
		}
		else {
			std::cout << message_head << "Invalid number of parameters to 'thresh'\n";
			std::cout << message_head << " -SYNTAX- thresh <lowerThresh> [upperThresh]\n";
		}
	}
	else if(cmd_ == "delay"){
		if(args_.size() == 1){ delay_ = atoi(args_.at(0).c_str()); }
		else{
			std::cout << message_head << "Invalid number of parameters to 'delay'\n";
			std::cout << message_head << " -SYNTAX- delay [time]\n";
		}
	}
	else if(cmd_ == "log"){
		if(canvas->GetLogy()){ 
			canvas->SetLogy(0);
			std::cout << message_head << "y-axis set to linear.\n"; 
		}
		else{ 
			canvas->SetLogy(1); 
			std::cout << message_head << "y-axis set to log.\n"; 
		}
	}
	else if(cmd_ == "trise"){ // Set the TRIGGER_RISETIME.
		if(args_.size() == 1){
			float temp = atof(args_.at(0).c_str());
			trig_rise = (int)(temp/ADC_CLOCK_uSEC);
			std::cout << message_head << "Set TRIGGER_RISETIME to " << trig_rise << " ADC ticks (" << trig_rise*ADC_CLOCK_uSEC << " μs).\n";
		}
		else{ std::cout << message_head << "TRIGGER_RISETIME = " << trig_rise*ADC_CLOCK_uSEC << " μs.\n"; }
	}
	else if(cmd_ == "tflat"){ // Set the TRIGGER_FLATTOP.
		if(args_.size() == 1){
			float temp = atof(args_.at(0).c_str());
			trig_flat = (int)(temp/ADC_CLOCK_uSEC);
			std::cout << message_head << "Set TRIGGER_FLATTOP to " << trig_flat << " ADC ticks (" << trig_flat*ADC_CLOCK_uSEC << " μs).\n";
		}
		else{ std::cout << message_head << "TRIGGER_FLATTOP = " << trig_flat*ADC_CLOCK_uSEC << " μs.\n"; }
	}
	else if(cmd_ == "erise"){ // Set the ENERGY_RISETIME.
		if(args_.size() == 1){
			float temp = atof(args_.at(0).c_str());
			energy_rise = (int)(temp/ADC_CLOCK_uSEC);
			std::cout << message_head << "Set ENERGY_RISETIME to " << energy_rise << " ADC ticks (" << energy_rise*ADC_CLOCK_uSEC << " μs).\n";
		}
		else{ std::cout << message_head << "ENERGY_RISETIME = " << energy_rise*ADC_CLOCK_uSEC << " μs.\n"; }
	}
	else if(cmd_ == "eflat"){ // Set the ENERGY_FLATTOP.
		if(args_.size() == 1){
			float temp = atof(args_.at(0).c_str());
			energy_flat = (int)(temp/ADC_CLOCK_uSEC);
			std::cout << message_head << "Set ENERGY_FLATTOP to " << energy_flat << " ADC ticks (" << energy_flat*ADC_CLOCK_uSEC << " μs).\n";
		}
		else{ std::cout << message_head << "ENERGY_FLATTOP = " << energy_flat*ADC_CLOCK_uSEC << " μs.\n"; }
	}
	else{ return false; }

	return true;
}

/// Scan has stopped data acquisition.
void Filterer::StopAcquisition(){
	acqRun_ = false;
}

/// Scan has started data acquisition.
void Filterer::StartAcquisition(){
	acqRun_ = true;	
}

void Filterer::IdleTask() {
	gSystem->ProcessEvents();
}

int main(int argc, char *argv[]){
	ScanMain scan_main((Unpacker*)(new Filterer()));
	
	scan_main.SetMessageHeader("Filterer: ");

	return scan_main.Execute(argc, argv);
}
