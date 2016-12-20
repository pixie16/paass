#include <iostream>
#include <algorithm> 

// PixieCore libraries
#include "XiaData.hpp"

// Local files
#include "scope.hpp"

#ifdef USE_HRIBF
#include "GetArguments.hpp"
#include "Scanor.hpp"
#include "ScanorInterface.hpp"
#endif

// Root files
#include "TSystem.h"
#include "TStyle.h"
#include "TMath.h"
#include "TGraph.h"
#include "TH2F.h"
#include "TAxis.h"
#include "TFile.h"
#include "TF1.h"
#include "TLine.h"
#include "TProfile.h"
#include "TPaveStats.h"

// Define the name of the program.
#ifndef PROG_NAME
#define PROG_NAME "Scope"
#endif

#define ADC_TIME_STEP 4 // ns
#define SLEEP_WAIT 1E4 // When not in shared memory mode, length of time to wait after gSystem->ProcessEvents is called (in us).

/**The Paulauskas function is described in NIM A 737 (22), with a slight 
 * adaptation. We use a step function such that f(x < phase) = baseline.
 * In addition, we also we formulate gamma such that the gamma in the paper is
 * gamma_prime = 1 / pow(gamma, 0.25).
 *
 * The parameters are:
 * p[0] = baseline
 * p[1] = amplitude
 * p[2] = phase
 * p[3] = beta
 * p[4] = gamma
 *
 * \param[in] x X value.
 * \param[in] p Paramater values.
 *
 * \return the value of the function for the specified x value and parameters.
 */
double PaulauskasFitFunc(double *x, double *p) {
	//Compute the time difference between x and the phase corrected for clock ticks.
	float diff = (x[0] - p[2])/ADC_TIME_STEP;
	//If the difference is less than zero we return the baseline.
	if (diff < 0 ) return p[0];
	//Return the computed function.
	return p[0] + p[1] * exp(-diff * p[3]) * (1 - exp(-pow(diff * p[4],4)));
}

///////////////////////////////////////////////////////////////////////////////
// class scopeUnpacker
///////////////////////////////////////////////////////////////////////////////

/// Default constructor.
scopeUnpacker::scopeUnpacker(const unsigned int &mod/*=0*/, const unsigned int &chan/*=0*/) : Unpacker(){
	mod_ = mod;
	chan_ = chan;
	threshLow_ = 0;
	threshHigh_ = -1;	
}

/** Process all events in the event list.
  * \param[in]  addr_ Pointer to a location in memory. 
  * \return Nothing.
  */
void scopeUnpacker::ProcessRawEvent(ScanInterface *addr_/*=NULL*/){
	if(!addr_){ return; }

	XiaData *current_event = NULL;

	// Fill the processor event deques with events
	while(!rawEvent.empty()){
		if(!running)
			break;
	
		//Get the first event in the FIFO.
		current_event = rawEvent.front();
		rawEvent.pop_front();

		// Safety catches for null event or empty adcTrace.
		if(!current_event || current_event->adcTrace.empty()){
			continue;
		}

		// Pass this event to the correct processor
		int maximum = *std::max_element(current_event->adcTrace.begin(),current_event->adcTrace.end());
		if(current_event->modNum == mod_ && current_event->chanNum == chan_){  
			//Check threhsold.
			if (maximum < threshLow_) {
				delete current_event;
				continue;
			}
			if (threshHigh_ > threshLow_ && maximum > threshHigh_) {
				delete current_event;
				continue;
			}

			//Store the waveform in the stack of waveforms to be displayed.
			if(addr_->AddEvent(current_event)){
				addr_->ProcessEvents();
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// class scopeScanner
///////////////////////////////////////////////////////////////////////////////

/// Default constructor.
scopeScanner::scopeScanner(int mod /*= 0*/, int chan/*=0*/) : RootScanner() {
	need_graph_update = false;
	resetGraph_ = false;
	acqRun_ = true;
	singleCapture_ = false;
	init = false;
	running = true;
	performFit_ = false;
	performCfd_ = false;
	numEvents = 20;
	numAvgWaveforms_ = 1;
	cfdF_ = 0.5;
	cfdD_ = 1;
	cfdL_ = 1;
	fitLow_ = 10;
	fitHigh_ = 15;
	delay_ = 2;
	num_displayed = 0;
	time(&last_trace);
	
	graph = new TGraph();
	cfdLine = new TLine();
	cfdLine->SetLineColor(kRed);
	cfdPol3 = new TF1("cfdPol3", "pol3");
	cfdPol3->SetLineColor(kGreen+1);
	cfdPol2 = new TF1("cfdPol2", "pol2");
	cfdPol2->SetLineColor(kMagenta+1);

	hist = new TH2F("hist","",256,0,1,256,0,1);

	SetupFunc();

	gStyle->SetPalette(51);
	
	//Display the stats: Integral
	gStyle->SetOptStat(1000000);
	
	//Display Fit Stats: Fit Values, Errors, and ChiSq.
	gStyle->SetOptFit(111);
}

/// Destructor.
scopeScanner::~scopeScanner(){
	delete graph;
	delete cfdLine;
	delete cfdPol3;
	delete cfdPol2;
	delete hist;
	delete paulauskasFunc;
}

TF1 *scopeScanner::SetupFunc() {
	paulauskasFunc = new TF1("paulauskas",PaulauskasFitFunc,0,1,5);
	paulauskasFunc->SetParNames("voffset","amplitude","phase","beta","gamma");
	
	return paulauskasFunc;
}

void scopeScanner::ResetGraph(unsigned int size) {
	delete graph;
		
	graph = new TGraph(size);
	graph->SetMarkerStyle(kFullDotSmall);
	
	if(size != x_vals.size()){
		std::cout << msgHeader << "Changing trace length from " << x_vals.size()*ADC_TIME_STEP << " to " << size*ADC_TIME_STEP << " ns.\n";
		x_vals.resize(size);
		for(size_t index = 0; index < x_vals.size(); index++)
			x_vals[index] = ADC_TIME_STEP * index;
	}
	hist->SetBins(x_vals.size(), x_vals.front(), x_vals.back() + ADC_TIME_STEP, 1, 0, 1);

	std::stringstream stream;
	stream << "M" << ((scopeUnpacker*)core)->GetMod() << "C" << ((scopeUnpacker*)core)->GetChan();
	graph->SetTitle(stream.str().c_str());
	hist->SetTitle(stream.str().c_str());

	resetGraph_ = false;
}

void scopeScanner::Plot(){
	static float histAxis[2][2];

	if(chanEvents_.size() < numAvgWaveforms_)
		return;

	if(chanEvents_.front()->size != x_vals.size()){ // The length of the trace has changed.
		resetGraph_ = true;
	}
	if (resetGraph_) {
		ResetGraph(chanEvents_.front()->size);
		ResetZoom();
		for (int i=0;i<2;i++) {
			histAxis[i][0] = 1E9;
			histAxis[i][1] = -1E9;
		}		
	}

	//For a waveform pulse we use a graph.
	if (numAvgWaveforms_ == 1) {
		int index = 0;
		for (size_t i = 0; i < chanEvents_.front()->size; ++i) {
			graph->SetPoint(index, x_vals[i], chanEvents_.front()->event->adcTrace.at(i));
			index++;
		}

		UpdateZoom();

		graph->Draw("AP0");

		float lowVal = (chanEvents_.front()->max_index - fitLow_) * ADC_TIME_STEP;
		float highVal = (chanEvents_.front()->max_index + fitHigh_) * ADC_TIME_STEP;

		///@TODO Renable the CFD with the proper functionality.
		/*
		if(performCfd_){
			ChannelEvent *evt = chanEvents_.front();

			// Find the zero-crossing of the cfd waveform.
			float cfdCrossing = evt->AnalyzeCFD(cfdF_);
			
			// Draw the cfd crossing line.
			cfdLine->DrawLine(cfdCrossing*ADC_TIME_STEP, userZoomVals[1][0], cfdCrossing*ADC_TIME_STEP, userZoomVals[1][1]);
			
			// Draw the 3rd order polynomial.
			cfdPol3->SetParameter(0, evt->cfdPar[0]);
			cfdPol3->SetParameter(1, evt->cfdPar[1]/ADC_TIME_STEP);
			cfdPol3->SetParameter(2, evt->cfdPar[2]/std::pow(ADC_TIME_STEP, 2.0));
			cfdPol3->SetParameter(3, evt->cfdPar[3]/std::pow(ADC_TIME_STEP, 3.0));
			// Find the pulse maximum by fitting with a third order polynomial.
			if(evt->event->adcTrace[evt->max_index-1] >= evt->event->adcTrace[evt->max_index+1]) // Favor the left side of the pulse.
				cfdPol3->SetRange((evt->max_index - 2)*ADC_TIME_STEP, (evt->max_index + 1)*ADC_TIME_STEP);
			else // Favor the right side of the pulse.
				cfdPol3->SetRange((evt->max_index - 1)*ADC_TIME_STEP, (evt->max_index + 2)*ADC_TIME_STEP);
			cfdPol3->Draw("SAME");
			
			// Draw the 2nd order polynomial.
			cfdPol2->SetParameter(0, evt->cfdPar[4]);
			cfdPol2->SetParameter(1, evt->cfdPar[5]/ADC_TIME_STEP);
			cfdPol2->SetParameter(2, evt->cfdPar[6]/std::pow(ADC_TIME_STEP, 2.0));
			cfdPol2->SetRange((evt->cfdIndex - 1)*ADC_TIME_STEP, (evt->cfdIndex + 1)*ADC_TIME_STEP);
			cfdPol2->Draw("SAME");
		}
		 */

		if(performFit_){
			paulauskasFunc->SetRange(lowVal, highVal);
			paulauskasFunc->SetParameters(chanEvents_.front()->baseline, 0.5 * chanEvents_.front()->qdc, lowVal, 0.5, 0.1);
			paulauskasFunc->FixParameter(0, chanEvents_.front()->baseline);
			graph->Fit(paulauskasFunc,"QMER");
		}
	}
	else { //For multiple events with make a 2D histogram and plot the profile on top.
		//Determine the maximum and minimum values of the events.
		for (unsigned int i = 0; i < numAvgWaveforms_; i++) {
			ChannelEvent* evt = chanEvents_.at(i);
			float evtMin = *std::min_element(evt->event->adcTrace.begin(), evt->event->adcTrace.end());
			float evtMax = *std::max_element(evt->event->adcTrace.begin(), evt->event->adcTrace.end());
			evtMin -= fabs(0.1 * evtMax);
			evtMax += fabs(0.1 * evtMax);
			if (evtMin < histAxis[1][0]) histAxis[1][0] = evtMin;
			if (evtMax > histAxis[1][1]) histAxis[1][1] = evtMax;
		}

		//Reset the histogram
		hist->Reset();
		
		//Rebin the histogram
		hist->SetBins(x_vals.size(), x_vals.front(), x_vals.back() + ADC_TIME_STEP, histAxis[1][1] - histAxis[1][0], histAxis[1][0], histAxis[1][1]);

		//Fill the histogram
		for (unsigned int i = 0; i < numAvgWaveforms_; i++) {
			ChannelEvent* evt = chanEvents_.at(i);
			for (size_t i=0; i < evt->size; ++i) {
				hist->Fill(x_vals[i], evt->event->adcTrace[i]);
			}
		}

		prof = hist->ProfileX("AvgPulse");
		prof->SetLineColor(kRed);
		prof->SetMarkerColor(kRed);

		float lowVal = prof->GetBinCenter(prof->GetMaximumBin() - fitLow_);
		float highVal = prof->GetBinCenter(prof->GetMaximumBin() + fitHigh_);
		
		if(performFit_){
			paulauskasFunc->SetRange(lowVal, highVal);
			paulauskasFunc->SetParameters(chanEvents_.front()->baseline, 0.5 * chanEvents_.front()->qdc, lowVal, 0.5, 0.2);
			paulauskasFunc->FixParameter(0, chanEvents_.front()->baseline);
			prof->Fit(paulauskasFunc,"QMER");
		}

		hist->SetStats(false);
		hist->Draw("COLZ");		
		prof->Draw("SAMES");

		UpdateZoom();

		GetCanvas()->Update();	
		TPaveStats* stats = (TPaveStats*) prof->GetListOfFunctions()->FindObject("stats");
		if (stats) {
			stats->SetX1NDC(0.55);
			stats->SetX2NDC(0.9);
		}
	}
	
	// Remove the events from the deque.
	for (unsigned int i = 0; i < numAvgWaveforms_; i++) {
		delete chanEvents_.front();
		chanEvents_.pop_front();
	}

	// Update the canvas.
	GetCanvas()->Update();

	// Save the TGraph to a file.
	if (saveFile_ != "") {
		TFile f(saveFile_.c_str(), "RECREATE");
		graph->Clone("trace")->Write();
		f.Close();
		saveFile_ = "";
	}

	num_displayed++;
}

/** Initialize the map file, the config file, the processor handler, 
  * and add all of the required processors.
  * \param[in]  prefix_ String to append to the beginning of system output.
  * \return True upon successfully initializing and false otherwise.
  */
bool scopeScanner::Initialize(std::string prefix_){
	if(init){ return false; }

	// Print a small welcome message.
	std::cout << "  Displaying traces for mod = " << ((scopeUnpacker*)core)->GetMod() << ", chan = " << ((scopeUnpacker*)core)->GetChan() << ".\n";

	return (init = true);
}

/** Receive various status notifications from the scan.
  * \param[in] code_ The notification code passed from ScanInterface methods.
  * \return Nothing.
  */
void scopeScanner::Notify(const std::string &code_/*=""*/){
	if(code_ == "START_SCAN"){ 
		ClearEvents();
		acqRun_ = true; 
	}
	else if(code_ == "STOP_SCAN"){ acqRun_ = false; }
	else if(code_ == "SCAN_COMPLETE"){ std::cout << msgHeader << "Scan complete.\n"; }
	else if(code_ == "LOAD_FILE"){ std::cout << msgHeader << "File loaded.\n"; }
	else if(code_ == "REWIND_FILE"){  }
	else{ std::cout << msgHeader << "Unknown notification code '" << code_ << "'!\n"; }
}

/** Return a pointer to the Unpacker object to use for data unpacking.
  * If no object has been initialized, create a new one.
  * \return Pointer to an Unpacker object.
  */
Unpacker *scopeScanner::GetCore(){ 
	if(!core){ core = (Unpacker*)(new scopeUnpacker()); }
	return core;
}

/** Add a channel event to the deque of events to send to the processors.
  * This method should only be called from skeletonUnpacker::ProcessRawEvent().
  * \param[in]  event_ The raw XiaData to add to the channel event deque.
  * \return True if events are ready to be processed and false otherwise.
  */
bool scopeScanner::AddEvent(XiaData *event_){
	if(!event_){ return false; }

	//Get the first event int the FIFO.
	ChannelEvent *channel_event = new ChannelEvent(event_);

	//Process the waveform.
	///@TODO : Renable this with the Helper functions.
	//channel_event->ComputeBaseline();
	//channel_event->FindQDC();
	
	//Push the channel event into the deque.
	chanEvents_.push_back(channel_event);

	// Handle the individual XiaData.
	if(chanEvents_.size() >= numAvgWaveforms_)
		return true;
		
	return false;
}

/** Process all channel events read in from the rawEvent.
  * This method should only be called from skeletonUnpacker::ProcessRawEvent().
  * \return True if events were processed and false otherwise.
  */
bool scopeScanner::ProcessEvents(){
	//Check if we have delayed the plotting enough
	time_t cur_time;
	time(&cur_time);
	while(difftime(cur_time, last_trace) < delay_) {
		//If in shm mode and the plotting time has not alloted the events are cleared and this function is aborted.
		if (ShmMode()) {
			ClearEvents();
			return false;
		}
		else {
			IdleTask();
			time(&cur_time);
		}
	}	

	//When we have the correct number of waveforms we plot them.
	Plot(); 
	
	//If this is a single capture we stop the plotting.
	if (singleCapture_) running = false;

	//Update the time.
	time(&last_trace);
	
	return true;
}

void scopeScanner::ClearEvents(){
	while(!chanEvents_.empty()){
		delete chanEvents_.front();
		chanEvents_.pop_front();
	}
}

/** CmdHelp is used to allow a derived class to print a help statement about
  * its own commands. This method is called whenever the user enters 'help'
  * or 'h' into the interactive terminal (if available).
  * \param[in]  prefix_ String to append at the start of any output. Not used by default.
  * \return Nothing.
  */
void scopeScanner::CmdHelp(const std::string &prefix_/*=""*/){
	std::cout << "   set <module> <channel>  - Set the module and channel of signal of interest (default = 0, 0).\n";
	std::cout << "   stop                    - Stop the acquistion.\n";
	std::cout << "   run                     - Run the acquistion.\n";
	std::cout << "   single                  - Perform a single capture.\n";
	std::cout << "   thresh <low> [high]     - Set the plotting window for trace maximum.\n";
	std::cout << "   fit <low> <high>        - Turn on fitting of waveform. Set <low> to \"off\" to disable.\n";
	std::cout << "   cfd [F=0.5] [D=1] [L=1] - Turn on cfd analysis of waveform. Set [F] to \"off\" to disable.\n";
	std::cout << "   avg <numWaveforms>      - Set the number of waveforms to average.\n";
	std::cout << "   save <fileName>         - Save the next trace to the specified file name..\n";
	std::cout << "   delay [time]            - Set the delay between drawing traces (in seconds, default = 1 s).\n";
	std::cout << "   log                     - Toggle log/linear mode on the y-axis.\n";
	std::cout << "   clear                   - Clear all stored traces and start over.\n";
}

/** ArgHelp is used to allow a derived class to add a command line option
  * to the main list of options. This method is called at the end of
  * from the ::Setup method.
  * Does nothing useful by default.
  * \return Nothing.
  */
void scopeScanner::ArgHelp(){
	AddOption(optionExt("mod", required_argument, NULL, 'M', "<module>", "Module of signal of interest (default=0)"));
	AddOption(optionExt("chan", required_argument, NULL, 'C', "<channel>", "Channel of signal of interest (default=0)"));
}

/** SyntaxStr is used to print a linux style usage message to the screen.
  * \param[in]  name_ The name of the program.
  * \return Nothing.
  */
void scopeScanner::SyntaxStr(char *name_){ 
	std::cout << " usage: " << std::string(name_) << " [options]\n"; 
}

/** ExtraArguments is used to send command line arguments to classes derived
  * from ScanInterface. This method should loop over the optionExt elements
  * in the vector userOpts and check for those options which have been flagged
  * as active by ::Setup(). This should be overloaded in the derived class.
  * \return Nothing.
  */
void scopeScanner::ExtraArguments(){
	if(userOpts.at(0).active)
		std::cout << msgHeader << "Set module to (" << ((scopeUnpacker*)core)->SetMod(atoi(userOpts.at(0).argument.c_str())) << ").\n";
	if(userOpts.at(1).active)
		std::cout << msgHeader << "Set channel to (" << ((scopeUnpacker*)core)->SetChan(atoi(userOpts.at(1).argument.c_str())) << ").\n";
}

/** ExtraCommands is used to send command strings to classes derived
  * from ScanInterface. If ScanInterface receives an unrecognized
  * command from the user, it will pass it on to the derived class.
  * \param[in]  cmd_ The command to interpret.
  * \param[out] arg_ Vector or arguments to the user command.
  * \return True if the command was recognized and false otherwise.
  */
bool scopeScanner::ExtraCommands(const std::string &cmd_, std::vector<std::string> &args_){
	if(cmd_ == "set"){ // Toggle debug mode
		if(args_.size() == 2){
			// Clear all events from the event deque.
			ClearEvents();
		
			// Set the module and channel.
			((scopeUnpacker*)core)->SetMod(atoi(args_.at(0).c_str()));
			((scopeUnpacker*)core)->SetChan(atoi(args_.at(1).c_str()));

			resetGraph_ = true;
		}
		else{
			std::cout << msgHeader << "Invalid number of parameters to 'set'\n";
			std::cout << msgHeader << " -SYNTAX- set <module> <channel>\n";
		}
	}
	else if(cmd_ == "single") {
		singleCapture_ = !singleCapture_;
	}
	else if (cmd_ == "thresh") {
		if (args_.size() == 1) {
			((scopeUnpacker*)core)->SetThreshLow(atoi(args_.at(0).c_str()));
			((scopeUnpacker*)core)->SetThreshHigh(-1);
		}
		else if (args_.size() == 2) {
			((scopeUnpacker*)core)->SetThreshLow(atoi(args_.at(0).c_str()));
			((scopeUnpacker*)core)->SetThreshHigh(atoi(args_.at(1).c_str()));
		}
		else {
			std::cout << msgHeader << "Invalid number of parameters to 'thresh'\n";
			std::cout << msgHeader << " -SYNTAX- thresh <lowerThresh> [upperThresh]\n";
		}
	}
	else if (cmd_ == "fit") {
		if (args_.size() >= 1 && args_.at(0) == "off") { // Turn root fitting off.
			if(performFit_){
				std::cout << msgHeader << "Disabling root fitting.\n"; 
				delete graph->GetListOfFunctions()->FindObject(paulauskasFunc->GetName());
				GetCanvas()->Update();
				performFit_ = false;
			}
			else{ std::cout << msgHeader << "Fitting is not enabled.\n"; }
		}
		else if (args_.size() == 2) { // Turn root fitting on.
			fitLow_ = atoi(args_.at(0).c_str());
			fitHigh_ = atoi(args_.at(1).c_str());
			std::cout << msgHeader << "Setting root fitting range to [" << fitLow_ << ", " << fitHigh_ << "].\n"; 
			performFit_ = true;
		}
		else {
			std::cout << msgHeader << "Invalid number of parameters to 'fit'\n";
			std::cout << msgHeader << " -SYNTAX- fit <low> <high>\n";
			std::cout << msgHeader << " -SYNTAX- fit off\n";
		}
	}
	else if (cmd_ == "cfd") {
		cfdF_ = 0.5;
		cfdD_ = 1;
		cfdL_ = 1;
		if (args_.empty()) { performCfd_ = true; }
		else if (args_.size() == 1) { 
			if(args_.at(0) == "off"){ // Turn cfd analysis off.
				if(performCfd_){
					std::cout << msgHeader << "Disabling cfd analysis.\n"; 
					performCfd_ = false;
				}
				else{ std::cout << msgHeader << "Cfd is not enabled.\n"; }
			}
			else{
				cfdF_ = atof(args_.at(0).c_str());
				performCfd_ = true;
			}
		}
		else if (args_.size() == 2) {
			cfdF_ = atof(args_.at(0).c_str());
			cfdD_ = atoi(args_.at(1).c_str());
			performCfd_ = true;
		}
		else if (args_.size() == 3) {
			cfdF_ = atof(args_.at(0).c_str());
			cfdD_ = atoi(args_.at(1).c_str());
			cfdL_ = atoi(args_.at(2).c_str());
			performCfd_ = true;
		}
		if(performCfd_)
			std::cout << msgHeader << "Enabling cfd analysis with F=" << cfdF_ << ", D=" << cfdD_ << ", L=" << cfdL_ << std::endl;
	}
	else if (cmd_ == "avg") {
		if (args_.size() == 1) {
			numAvgWaveforms_ = atoi(args_.at(0).c_str());
		}
		else {
			std::cout << msgHeader << "Invalid number of parameters to 'avg'\n";
			std::cout << msgHeader << " -SYNTAX- avg <numWavefroms>\n";
		}
	}
	else if(cmd_ == "save") {
		if (args_.size() == 1) {
			saveFile_ = args_.at(0);
		}
		else {
			std::cout << msgHeader << "Invalid number of parameters to 'save'\n";
			std::cout << msgHeader << " -SYNTAX- save <fileName>\n";
		}
	}
	else if(cmd_ == "delay"){
		if(args_.size() == 1){ delay_ = atoi(args_.at(0).c_str()); }
		else{
			std::cout << msgHeader << "Invalid number of parameters to 'delay'\n";
			std::cout << msgHeader << " -SYNTAX- delay <time>\n";
		}
	}
	else if(cmd_ == "log"){
		if(GetCanvas()->GetLogy()){ 
			GetCanvas()->SetLogy(0);
			std::cout << msgHeader << "y-axis set to linear.\n"; 
		}
		else{ 
			GetCanvas()->SetLogy(1); 
			std::cout << msgHeader << "y-axis set to log.\n"; 
		}
	}
	else if(cmd_ == "clear"){
		ClearEvents();
		std::cout << msgHeader << "Event deque cleared.\n";
	}
	else{ return false; }

	return true;
}


#ifndef USE_HRIBF
int main(int argc, char *argv[]){
	// Define a new unpacker object.
	scopeScanner scanner;
	
	// Set the output message prefix.
	scanner.SetProgramName(std::string(PROG_NAME));	
	
	// Initialize the scanner.
	if(!scanner.Setup(argc, argv))
		return 1;

	// Run the main loop.
	int retval = scanner.Execute();
	
	scanner.Close();
	
	return retval;
}
#else
scopeScanner *scanner = NULL;

// Do some startup stuff.
extern "C" void startup_()
{
       scanner = new scopeScanner();   

       // Handle command line arguments from SCANOR
       scanner->Setup(GetNumberArguments(), GetArguments());
       
       // Get a pointer to a class derived from Unpacker.
       ScanorInterface::get()->SetUnpacker(scanner->GetCore());
}

///@brief Defines the main interface with the SCANOR library, the program
/// essentially starts here.
///@param [in] iexist : unused paramter from SCANOR call
extern "C" void drrsub_(uint32_t &iexist) {
       drrmake_();
       hd1d_(8000, 2, 256, 256, 0, 255, "Run DAMM you!", strlen("Run DAMM you!"));
       endrr_();
}

// Catch the exit call from scanor and clean up c++ objects CRT
extern "C" void cleanup_()
{
       // Do some cleanup.
       std::cout << "\nCleaning up..\n";
       scanner->Close();
       delete scanner;
}
#endif

