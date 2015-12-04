#include <iostream>
#include <algorithm> 

// PixieCore libraries
#include "ScanMain.hpp"
#include "ChannelEvent.hpp"

// Local files
#include "scope.hpp"

// Root files
#include "TApplication.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TH2F.h"
#include "TAxis.h"
#include "TFile.h"
#include "TF1.h"
#include "TProfile.h"
#include "TPaveStats.h"

#define ADC_TIME_STEP 4 // ns
#define SLEEP_WAIT 1E4 // When not in shared memory mode, length of time to wait between calls to gSystem->ProcessEvents (in us).

double PaulauskasFitFunc(double *x, double *p) {
	float diff = (x[0] - p[1])/ADC_TIME_STEP;
	if (diff < 0 ) return 0;
	return p[0] * exp(-diff * p[2]) * (1 - exp(-pow(diff * p[3],4)));
}

Oscilloscope::Oscilloscope(int mod /*= 0*/, int chan/*=0*/) :
	mod_(mod),
	chan_(chan), 
	acqRun_(true),
	singleCapture_(false),
	numAvgWaveforms_(1),
	threshLow_(0),
	threshHigh_(-1),
	fitLow_(10),
	fitHigh_(15),
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
	hist = new TH2F("hist","",256,0,1,256,0,1);
	hist->SetBit(TH1::kCanRebin);

	paulauskasFunc = new TF1("paulauskas",PaulauskasFitFunc,0,1,4);
	paulauskasFuncText = new TF1("paulauskasText","[0] * exp(-(x - [1])*[2]) * (1 - exp(-pow((x-[1])*[3],4)))",4);
	paulauskasFunc->SetParNames("amplitude","phase","beta","gamma");

	gStyle->SetPalette(51);
	
	//Display the stats: Integral
	gStyle->SetOptStat(1000000);
	//Display Fit Stats: Fit Values, Errors, and ChiSq.
	gStyle->SetOptFit(111);

}

Oscilloscope::~Oscilloscope(){
	canvas->Close();
	delete canvas;
	delete graph;
	delete hist;
	delete paulauskasFunc;

	// Call Unpacker::Close() to finish cleanup.
	Close();
}


void Oscilloscope::ResetGraph(unsigned int size) {
	if(size != x_vals.size()){
		std::cout << message_head << "Changing trace length from " << x_vals.size()*ADC_TIME_STEP << " to " << size*ADC_TIME_STEP << " ns.\n";
		x_vals.resize(size);
		for(size_t index = 0; index < x_vals.size(); index++){
			x_vals[index] = ADC_TIME_STEP * index;
		}	
		while ((unsigned int) graph->GetN() > size) graph->RemovePoint(graph->GetN());
	}
	hist->SetBins(x_vals.size(), x_vals.front(), x_vals.back() + ADC_TIME_STEP, 1, 0, 1);

	std::stringstream stream;
	stream << "M" << mod_ << "C" << chan_;
	graph->SetTitle(stream.str().c_str());
	hist->SetTitle(stream.str().c_str());

	resetGraph_ = false;
}

void Oscilloscope::Plot(std::vector<ChannelEvent*> events){

	///The limits of the vertical axis
	static float axisMin, axisMax;

	// Draw the trace

	if(events[0]->size != x_vals.size()){ // The length of the trace has changed.
		resetGraph_ = true;
	}
	if (resetGraph_) {
		ResetGraph(events[0]->size);
		axisMax = 0;
		axisMin = 1E9;
	}
	
	//For a waveform pulse we use a graph.
	if (numAvgWaveforms_ == 1) {
		int index = 0;
		for (size_t i=0;i<events[0]->size;++i) {
			graph->SetPoint(index, x_vals[i], events[0]->yvals[i]);
			index++;
		}

		if (graph->GetYaxis()->GetXmax() > axisMax) axisMax = graph->GetYaxis()->GetXmax(); 
		if (graph->GetYaxis()->GetXmin() < axisMin) axisMin = graph->GetYaxis()->GetXmin(); 
		graph->GetYaxis()->SetLimits(axisMin, axisMax);
		graph->GetYaxis()->SetRangeUser(axisMin, axisMax);

		graph->Draw("APC0");

		float lowVal = (events[0]->max_index - fitLow_) * ADC_TIME_STEP;
		float highVal = (events[0]->max_index + fitHigh_) * ADC_TIME_STEP;
		paulauskasFunc->SetRange(lowVal, highVal);
		paulauskasFunc->SetParameters(events[0]->qdc*0.5,lowVal,0.5,0.1);
		paulauskasFunc->SetParLimits(0,0,2*events[0]->qdc);
		paulauskasFunc->SetParLimits(3,0,0.5);
		graph->Fit(paulauskasFunc,"QMER");
	}
	//For multiple events with make a 2D histogram and plot the profile on top.
	else {

		//Determine the maximum and minimum values of the events.
		for (auto itr = events.begin(); itr != events.end(); ++itr) {
			ChannelEvent* evt = *itr;
			if (evt->maximum > axisMax) axisMax = 1.1 * evt->maximum;
			if (evt->stddev < axisMin) axisMin = 0.9 * evt->stddev;
		}

		//Reset the histogram
		hist->Reset();
		//Rebin the histogram
		hist->SetBins(x_vals.size(), x_vals.front(), x_vals.back() + ADC_TIME_STEP, axisMax - axisMin, axisMin, axisMax);

		//Fill the histogram
		for (auto itr = events.begin(); itr != events.end(); ++itr) {
			ChannelEvent *evt = *itr;
			for (size_t i=0;i<evt->size;++i) {
				hist->Fill(x_vals[i],evt->yvals[i]);
			}
		}

		prof = hist->ProfileX("AvgPulse");
		prof->SetLineColor(kRed);
		prof->SetMarkerColor(kRed);

		float lowVal = prof->GetBinCenter(prof->GetMaximumBin() - fitLow_);
		float highVal = prof->GetBinCenter(prof->GetMaximumBin() + fitHigh_);
		paulauskasFunc->SetRange(lowVal, highVal);
		paulauskasFunc->SetParameters(events[0]->qdc*0.5,lowVal,0.5,0.1);
		paulauskasFunc->SetParLimits(0,0,2*events[0]->qdc);
		paulauskasFunc->SetParLimits(3,0,0.5);
		prof->Fit(paulauskasFunc,"QMER");

		hist->SetStats(false);
		hist->Draw("COLZ");		
		prof->Draw("SAMES");
		canvas->Update();	
		TPaveStats* stats = (TPaveStats*) prof->GetListOfFunctions()->FindObject("stats");
		if (stats) {
			stats->SetX1NDC(0.55);
			stats->SetX2NDC(0.9);
		}

	}

	canvas->Update();

	if (saveFile_ != "") {
		TFile f(saveFile_.c_str(), "RECREATE");
		graph->Clone("trace")->Write();
		f.Close();
		saveFile_ = "";
	}

	num_displayed++;
}

void Oscilloscope::ClearEvents() {
	while (!events_.empty()) {
		delete events_.back();
		events_.pop_back();
	}
}

void Oscilloscope::ProcessRawEvent(){
	ChannelEvent *current_event = NULL;
	//static int numWaveforms = 0;
	//static std::vector<std::pair<int, float>> waveform;

	
	// Fill the processor event deques with events
	while(!rawEvent.empty()){
		if(kill_all){ break; }
		//If the acquistion is not running we clear the events.
		//	For the SHM mode we simply break and wait for the next data packet.
		//	For the ldf mode we sleep and check acqRun again.
		while(!acqRun_) {
			//Clean up any stored waveforms.
			ClearEvents();
			if(shm_mode || kill_all){ return; }
			usleep(SLEEP_WAIT);
			gSystem->ProcessEvents();
		}
	
		//Get the first event int he FIFO.
		current_event = rawEvent.front();
		rawEvent.pop_front();

		// Pass this event to the correct processor
		if(current_event->modNum == mod_ && current_event->chanNum == chan_){  
			num_traces++;
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

			//Store the waveform in the stack of waveforms to be displayed.
			events_.push_back(current_event);

			//When we have the correct number of waveforms we plot them.
			if (events_.size() >= numAvgWaveforms_) {
				time_t cur_time;
				time(&cur_time);
				while(difftime(cur_time, last_trace) < delay_) {
					//If in shm mode and the plotting time has not alloted the events are cleared and this function is aborted.
					if (shm_mode) {
						ClearEvents();
						return;
					}
					else {
						usleep(SLEEP_WAIT);
						gSystem->ProcessEvents();
						time(&cur_time);
					}
				}	

				Plot(events_); 
				//If this is a single capture we stop the plotting.
				if (singleCapture_) acqRun_ = false;

				time(&last_trace);

				//Clean up the events plotted.
				ClearEvents();
			}
		}
		gSystem->ProcessEvents();
	}
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
	std::cout << prefix_ << "set <module> <channel> - Set the module and channel of signal of interest (default = 0, 0).\n";
	std::cout << prefix_ << "stop                   - Stop the acquistion.\n";
	std::cout << prefix_ << "run                    - Run the acquistion.\n";
	std::cout << prefix_ << "single                 - Perform a single capture.\n";
	std::cout << prefix_ << "thresh <low> [high]    - Set the plotting window for trace maximum.\n";
	std::cout << prefix_ << "fit <low> <high>       - Turn on fitting of waveform.\n";
	std::cout << prefix_ << "avg <numWaveforms>     - Set the number of waveforms to average.\n";
	std::cout << prefix_ << "save <fileName>        - Save the next trace to the specified file name..\n";
	std::cout << prefix_ << "delay [time]           - Set the delay between drawing traces (in seconds, default = 1 s).\n";
	std::cout << prefix_ << "log                    - Toggle log/linear mode on the y-axis.\n";
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
			//Set the module and channel.
			mod_ = atoi(args_.at(0).c_str());
			chan_ = atoi(args_.at(1).c_str());

			//Store previous run status.
			bool runStatus = acqRun_;
			//Stop the run to force the buffer to be cleared.
			acqRun_ = false;
			//Wait until the event buffer has been cleared.
			while(!events_.empty()) usleep(SLEEP_WAIT);
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
	else if (cmd_ == "fit") {
		if (args_.size() == 2) {
			fitLow_ = atoi(args_.at(0).c_str());
			fitHigh_ = atoi(args_.at(1).c_str());
		}
		else {
			std::cout << message_head << "Invalid number of parameters to 'fit'\n";
			std::cout << message_head << " -SYNTAX- fit <low> <high>\n";
		}
	}
	else if (cmd_ == "avg") {
		if (args_.size() == 1) {
			numAvgWaveforms_ =  atoi(args_.at(0).c_str());
		}
		else {
			std::cout << message_head << "Invalid number of parameters to 'avg'\n";
			std::cout << message_head << " -SYNTAX- avg <numWavefroms>\n";
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

/// Scan has stopped data acquisition.
void Oscilloscope::StopAcquisition(){
	acqRun_ = false;
}

/// Scan has started data acquisition.
void Oscilloscope::StartAcquisition(){
	acqRun_ = true;	
}

void Oscilloscope::IdleTask() {
	gSystem->ProcessEvents();
}

int main(int argc, char *argv[]){
	ScanMain scan_main((Unpacker*)(new Oscilloscope()));
	
	scan_main.SetMessageHeader("Scope: ");

	return scan_main.Execute(argc, argv);
}
