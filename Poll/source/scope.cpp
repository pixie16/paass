#include <iostream>

// PixieCore libraries
#include "ScanMain.hpp"
#include "ChannelEvent.hpp"

// Local files
#include "scope.hpp"

// Root files
#include "TApplication.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TFile.h"

#define ADC_TIME_STEP 4 // ns


void Oscilloscope::ResetGraph(int size) {
	if(size != x_vals.size()){
		std::cout << message_head << "Changing trace length from " << x_vals.size()*ADC_TIME_STEP << " to " << size*ADC_TIME_STEP << " ns.\n";
		x_vals.resize(size);
		for(size_t index = 0; index < x_vals.size(); index++){
			x_vals[index] = ADC_TIME_STEP * index;
		}	
		while (graph->GetN() > size) graph->RemovePoint(graph->GetN());
	}

	std::stringstream stream;
	stream << "mod = " << mod_ << ", chan = " << chan_;
	graph->SetTitle(stream.str().c_str());

	resetGraph_ = false;
}

void Oscilloscope::Plot(ChannelEvent *event_){

	///The limits of the vertical axis
	static float axisMin, axisMax;

	time_t cur_time;
	time(&cur_time);
	
	// Draw the trace
	if((int)difftime(cur_time, last_trace) >= delay_){
		event_->CorrectBaseline();
		
		if(event_->trace.size() != x_vals.size()){ // The length of the trace has changed.
			resetGraph_ = true;
		}
		if (resetGraph_) {
			ResetGraph(event_->trace.size());
			axisMax = 0;
			axisMin = 1E9;
		}
		else {
			axisMin = graph->GetYaxis()->GetXmin();
			axisMax = graph->GetYaxis()->GetXmax();
		}

		if(event_->trace.size() != x_vals.size()){ // The length of the trace has changed.
			
		}

		int index = 0;
		for(auto iterx = x_vals.begin(), itery = event_->trace.begin(); iterx != x_vals.end() && itery != event_->trace.end(); iterx++, itery++){
			graph->SetPoint(index++, *iterx, *itery);
		}
		
		if (graph->GetYaxis()->GetXmax() > axisMax) axisMax = graph->GetYaxis()->GetXmax(); 
		if (graph->GetYaxis()->GetXmin() < axisMin) axisMin = graph->GetYaxis()->GetXmin(); 
		graph->GetYaxis()->SetLimits(axisMin, axisMax);
		graph->GetYaxis()->SetRangeUser(axisMin, axisMax);

		graph->Draw("APC");
		canvas->Update();


		if (saveFile_ != "") {
			TFile f(saveFile_.c_str(), "RECREATE");
			graph->Clone("trace")->Write();
			f.Close();
			saveFile_ = "";
		}

		time(&last_trace);
		num_displayed++;
	}
	gSystem->ProcessEvents();

	num_traces++;
}

void Oscilloscope::ProcessRawEvent(){
	ChannelEvent *current_event = NULL;

	// Fill the processor event deques with events
	while(!rawEvent.empty()){
		current_event = rawEvent.front();
	
		// Pass this event to the correct processor
		if(current_event->modNum == mod_ && current_event->chanNum == chan_){  
			// This is a signal we wish to plot.
			Plot(current_event); 
		}
		// Remove this event from the raw event deque
		delete current_event;
		rawEvent.pop_front();
	}
}

Oscilloscope::Oscilloscope(int mod /*= 0*/, int chan/*=0*/) :
	mod_(mod),
	chan_(chan), 
	delay_(2),
	num_traces(0),
	num_displayed(0)
{
	time(&last_trace);
	
	// Variables for root graphics
	rootapp = new TApplication("scope", 0, NULL);
	gSystem->Load("libTree");
	
	canvas = new TCanvas("scope_canvas", "Oscilloscope");
	canvas->cd();
	
	graph = new TGraph();
	
	graph->Draw();
}

Oscilloscope::~Oscilloscope(){
	std::cout << "~Oscilloscope\n";

	// Cleanup the local variables.
	canvas->Close();
	
	rootapp->Delete();
	delete canvas;
	delete graph;
	
	// Call Unpacker::Close() to finish cleanup.
	Close();
}

bool Oscilloscope::Initialize(std::string prefix_){
	if(init){ return false; }
	
	// Print a small welcome message.
	std::cout << prefix_ << "Displaying traces for mod = " << mod_ << ", chan = " << chan_ << ".\n";
	
	return (init = true);
}

/**
 * \param[in] prefix_
 */
void Oscilloscope::ArgHelp(std::string prefix_){
	std::cout << prefix_ << "--mod [module]   | Module of signal of interest (default=0)\n";
	std::cout << prefix_ << "--chan [channel] | Channel of signal of interest (default=0)\n";
}

/** 
 *
 *	\param[in] prefix_ 
 */
void Oscilloscope::CmdHelp(std::string prefix_){
	std::cout << prefix_ << "set [module] [channel] - Set the module and channel of signal of interest (default = 0, 0).\n";
	std::cout << prefix_ << "delay [time]           - Set the delay between drawing traces (in seconds, default = 1 s).\n";
	std::cout << prefix_ << "log                    - Toggle log/linear mode on the y-axis.\n";
	std::cout << prefix_ << "save <fileName>        - Save the next trace to the specified file name..\n";
}

/**
 * \param args_
 * \param filename_
 */
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

bool Oscilloscope::CommandControl(std::string cmd_, const std::vector<std::string> &args_){
	if(cmd_ == "set"){ // Toggle debug mode
		if(args_.size() == 2){
			mod_ = atoi(args_.at(0).c_str());
			chan_ = atoi(args_.at(1).c_str());
			resetGraph_ = true;
		}
		else{
			std::cout << message_head << "Invalid number of parameters to 'set'\n";
			std::cout << message_head << " -SYNTAX- set [module] [channel]\n";
		}
	}
	else if(cmd_ == "save") {
		if (args_.size() == 1) {
			saveFile_ = args_.at(0);
		}
		else {
			std::cout << message_head << "Invalid number of parameters to 'save'\n";
			std::cout << message_head << " -SYNTAX- save <fileName>\n";
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
	else{ return false; }

	return true;
}

void Oscilloscope::IdleTask() {
	gSystem->ProcessEvents();
}

int main(int argc, char *argv[]){
	ScanMain scan_main((Unpacker*)(new Oscilloscope()));
	
	scan_main.SetMessageHeader("Scope: ");

	return scan_main.Execute(argc, argv);
}
