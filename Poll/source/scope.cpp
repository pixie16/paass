#include <iostream>

// PixieCore libraries
#include "Unpacker.hpp"
#include "ChannelEvent.hpp"

// Local files
#include "scope.hpp"

void Oscilloscope::Plot(ChannelEvent *event_){
	// Does nothing for now.
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
	num_traces = 0;
}

Oscilloscope::Oscilloscope(int mod_, int chan_){
	mod = mod_;
	chan = chan_; 
	num_traces = 0;
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
