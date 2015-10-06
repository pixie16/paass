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
#include "TH2F.h"

#define ADC_TIME_STEP 4 // ns

void Oscilloscope::UpdateGraph(int size_){
	if(size_ != graph->GetN()){
		std::cout << message_head << "Changing trace length from " << graph->GetN()*ADC_TIME_STEP << " to " << size_*ADC_TIME_STEP << " ns.\n";

		x_vals.clear();
		x_vals.assign(size_, 0);
		for(size_t index = 0; index < x_vals.size(); index++){
			x_vals[index] = ADC_TIME_STEP * index;
		}	

		// Update the root TGraph
		delete graph;
		graph = new TGraph(size_);
	}

	std::stringstream stream;
	stream << "mod = " << mod << ", chan = " << chan;
	his->SetTitle(stream.str().c_str());
	
	need_graph_update = false;
}

void Oscilloscope::UpdateFrame(ChannelEvent *event_){
	his->SetBins(1, 0, graph->GetN()*ADC_TIME_STEP, 1, 0.9*event_->baseline, 1.1*(event_->baseline+event_->maximum));
	his->Draw();
	old_maximum = event_->maximum;
}

void Oscilloscope::Plot(ChannelEvent *event_){
	time_t cur_time;
	time(&cur_time);
	
	// Draw the trace
	if((int)difftime(cur_time, last_trace) >= delay){
		event_->CorrectBaseline();
	
		if(need_graph_update || event_->trace.size() != x_vals.size()){ // The length of the trace has changed.
			UpdateGraph(event_->trace.size());
		}
		
		int index = 0;
		std::vector<int>::iterator iterx, itery;
		for(iterx = x_vals.begin(), itery = event_->trace.begin(); iterx != x_vals.end() && itery != event_->trace.end(); iterx++, itery++){
			graph->SetPoint(index++, *iterx, *itery);
		}
		
		if(event_->maximum > old_maximum){
			UpdateFrame(event_);
		}
		
		graph->Draw("PCSAME");
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
	delay = 2;
	num_traces = 0;
	num_displayed = 0;
	time(&last_trace);
	
	old_maximum = -9999;
	
	// Variables for root graphics
	rootapp = new TApplication("scope", 0, NULL);
	gSystem->Load("libTree");
	
	canvas = new TCanvas("scope_canvas", "Oscilloscope");
	canvas->cd();
	
	graph = new TGraph();
	
	// Setup the default histogram frame.
	his = new TH2F("his", "", 1, 0, 1, 1, 0, 1);
	his->SetStats(false);
	his->GetYaxis()->SetTitleOffset(1.4);
	his->GetXaxis()->SetTitle("Time (ns)");
	his->GetYaxis()->SetTitle("ADC Channel (a.u.)");
	
	need_graph_update = false;
}

Oscilloscope::Oscilloscope(int mod_, int chan_){
	mod = mod_;
	chan = chan_; 
	delay = 2;
	num_traces = 0;
	num_displayed = 0;
	time(&last_trace);
	
	old_maximum = -9999;

	// Variables for root graphics
	rootapp = new TApplication("scope", 0, NULL);
	gSystem->Load("libTree");
	
	canvas = new TCanvas("scope_canvas", "Oscilloscope");
	canvas->cd();
	
	graph = new TGraph();
	
	// Setup the default histogram frame.
	his = new TH2F("his", "", 1, 0, 1, 1, 0, 1);
	his->SetStats(false);
	his->GetYaxis()->SetTitleOffset(1.4);
	his->GetXaxis()->SetTitle("Time (ns)");
	his->GetYaxis()->SetTitle("ADC Channel (a.u.)");
	
	need_graph_update = false;
}

Oscilloscope::~Oscilloscope(){
	canvas->Close();
	delete canvas;
	delete graph;
	delete his;
}

bool Oscilloscope::Initialize(std::string prefix_){
	if(init){ return false; }
	
	// Print a small welcome message.
	std::cout << prefix_ << "Displaying traces for mod = " << mod << ", chan = " << chan << ".\n";
	
	return (init = true);
}

/// Print a command line help dialogue for recognized command line arguments.
void Oscilloscope::ArgHelp(std::string prefix_){
	std::cout << prefix_ << "--mod [module]   | Module of signal of interest (default=0)\n";
	std::cout << prefix_ << "--chan [channel] | Channel of signal of interest (default=0)\n";
}

/// Print an in-terminal help dialogue for recognized commands.
void Oscilloscope::CmdHelp(std::string prefix_){
	std::cout << prefix_ << "set [module] [channel] - Set the module and channel of signal of interest (default = 0, 0).\n";
	std::cout << prefix_ << "delay [time]           - Set the delay between drawing traces (in seconds, default = 1 s).\n";
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

bool Oscilloscope::CommandControl(std::string cmd_, const std::vector<std::string> &args_){
	if(cmd_ == "set"){ // Toggle debug mode
		if(args_.size() >= 2){
			mod = atoi(args_.at(0).c_str());
			chan = atoi(args_.at(1).c_str());
			need_graph_update = true;
			old_maximum = -9999;
		}
		else{
			std::cout << message_head << "Invalid number of parameters to 'set'\n";
			std::cout << message_head << " -SYNTAX- set [module] [channel]\n";
		}
	}
	else if(cmd_ == "delay"){
		if(args_.size() >= 2){ delay = atoi(args_.at(0).c_str()); }
		else{
			std::cout << message_head << "Invalid number of parameters to 'delay'\n";
			std::cout << message_head << " -SYNTAX- delay [time]\n";
		}
	}
	else{ return false; }

	return true;
}
