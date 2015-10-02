#include <iostream>

// PixieCore libraries
#include "Unpacker.hpp"
#include "ChannelEvent.hpp"

// Local files
#include "scope.hpp"

// Root files
#include "TApplication.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"

#define ADC_TIME_STEP 4 // ns

void Oscilloscope::Plot(ChannelEvent *event_){
	time_t cur_time;
	time(&cur_time);
	
	// Draw the trace
	if((int)difftime(cur_time, last_trace) >= delay){
		if(event_->trace.size() != x_vals.size()){ // The length of the trace has changed.
			std::cout << "Plot: Changing trace length from " << x_vals.size() << " to " << event_->trace.size() << std::endl;
			x_vals.clear();
			x_vals.assign(event_->trace.size(), 0);
			for(size_t index = 0; index < x_vals.size(); index++){
				x_vals[index] = ADC_TIME_STEP * index;
			}
			
			// Update the root TGraph
			delete graph;
			graph = new TGraph(event_->trace.size());
			
			std::stringstream stream;
			stream << "mod = " << mod << ", chan = " << chan;
			graph->SetTitle(stream.str().c_str());
			graph->GetXaxis()->SetTitle("Time (ns)");
			graph->GetYaxis()->SetTitle("ADC Channel (a.u.)");
		}
		
		int index = 0;
		std::vector<int>::iterator iterx, itery;
		for(iterx = x_vals.begin(), itery = event_->trace.begin(); iterx != x_vals.end() && itery != event_->trace.end(); iterx++, itery++){
			graph->SetPoint(index++, *iterx, *itery);
		}
		
		graph->Draw("APC");
		canvas->Update();
		
		time(&last_trace);
		num_displayed++;
	}
	
	num_traces++;
}

void Oscilloscope::ProcessRawEvent(){
	ChannelEvent *current_event = NULL;

	// Fill the processor event deques with events
	while(!rawEvent.empty()){
		current_event = rawEvent.front();
	
		// Pass this event to the correct processor
		if(current_event->modNum == mod && current_event->chanNum == chan){ Plot(current_event); } // This is a signal we wish to plot.
		
		// Remove this event from the raw event deque
		delete current_event;
		rawEvent.pop_front();
	}
}

Oscilloscope::Oscilloscope(){
	mod = 0;
	chan = 0;
	delay = 1;
	num_traces = 0;
	num_displayed = 0;
	time(&last_trace);
	
	// Variables for root graphics
	rootapp = new TApplication("scope", 0, NULL);
	gSystem->Load("libTree");
	
	canvas = new TCanvas("scope_canvas", "Oscilloscope");
	canvas->cd();
	
	graph = new TGraph();
}

Oscilloscope::Oscilloscope(int mod_, int chan_){
	mod = mod_;
	chan = chan_; 
	delay = 1;
	num_traces = 0;
	num_displayed = 0;
	time(&last_trace);

	// Variables for root graphics
	rootapp = new TApplication("scope", 0, NULL);
	gSystem->Load("libTree");
	
	canvas = new TCanvas("scope_canvas", "Oscilloscope");
	canvas->cd();
	
	graph = new TGraph();
}

Oscilloscope::~Oscilloscope(){
	canvas->Close();
	delete canvas;
	delete graph;
}

bool Oscilloscope::Initialize(std::string prefix_){
	if(init){ return false; }
	
	// Print a small welcome message.
	std::cout << prefix_ << "Displaying traces for mod = " << mod << ", chan = " << chan << ".\n";
	
	return (init = true);
}

/// Print a help dialogue.
void Oscilloscope::Help(std::string prefix_){
	std::cout << prefix_ << "--mod [module]   | Module of signal of interest (default=0)\n";
	std::cout << prefix_ << "--chan [channel] | Channel of signal of interest (default=0)\n";
}

/// Scan input arguments and set class variables.
bool Oscilloscope::SetArgs(std::deque<std::string> &args_, std::string &filename_){
	std::string current_arg;
	while(!args_.empty()){
		current_arg = args_.front();
		args_.pop_front();
		
		if(current_arg == "--mod"){
			if(args_.empty()){
				std::cout << " Error: Missing required argument to option '--mod'!\n";
				return false;
			}
			mod = atoi(args_.front().c_str());
			args_.pop_front();
		}
		else if(current_arg == "--chan"){
			if(args_.empty()){
				std::cout << " Error: Missing required argument to option '--chan'!\n";
				return false;
			}
			chan = atoi(args_.front().c_str());
			args_.pop_front();
		}
		else{ filename_ = current_arg; }
	}
	
	return true;
}
