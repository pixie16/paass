// poll2_core.cpp

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <string.h>
#include <sstream>
#include <ctime>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "poll2_core.h"
#include "poll2_socket.h"

// Interface for the PIXIE-16
#include "Utility.h"
#include "pixie16app_export.h"
//#include "Buffer_Structure.h"
#include "StatsHandler.hpp"
#include "Display.h"

// Values associated with the minimum timing between pixie calls (in us)
// Adjusted to help alleviate the issue with data corruption
#define END_RUN_PAUSE 100
#define POLL_TRIES 100
#define WAIT_TRIES 100

Poll::Poll(){
	pif = new PixieInterface("pixie.cfg");

	clock_vsn = 1000;

	// System flags and variables
	sys_message_head = " POLL: ";
	kill_all = false; // Set to true when the program is exiting
	start_run = false; // Set to true when the command is given to start a run
	stop_run = false; // Set to true when the command is given to stop a run
	do_reboot = false; // Set to true when the user tells POLL to reboot PIXIE
	force_spill = false; // Force poll2 to dump the current data spill
	poll_running = false; // Set to true when run_command is recieving data from PIXIE
	run_ctrl_exit = false; // Set to true when run_command exits
	raw_time = 0;

	// Run control variables
	boot_fast = false;
	insert_wall_clock = true;
	is_quiet = false;
	send_alarm = false;
	show_module_rates = false;
	zero_clocks = false;
	debug_mode = false;
	init = false;

	// Options relating to output data file
	output_directory = "./"; // Set with 'fdir' command
	output_filename = "pixie"; // Set with 'ouf' command
	output_title = "PIXIE data file"; // Set with 'htit' command
	output_run_num = 0; // Set with 'hnum' command
	output_format = 0; // Set with 'oform' command

	// The main output data file and related variables
	current_file_num = 0;
	current_filename = "";
	
	stats_interval = -1; //< in seconds
	histo_interval = -1; //< in seconds

	runDone = NULL;
	fifoData = NULL;
	partialEventData = NULL;
	statsHandler = NULL;
}

bool Poll::initialize(){
	if(init){ return false; }

	// Set debug mode
	if(debug_mode){ 
		std::cout << sys_message_head << "Setting debug mode\n";
		output_file.SetDebugMode(); 
	}

	// Initialize the pixie interface and boot
	pif->GetSlots();
	if(!pif->Init()){ return false; }

	if(boot_fast){
		if(!pif->Boot(PixieInterface::DownloadParameters | PixieInterface::SetDAC | PixieInterface::ProgramFPGA)){ return false; } 
	}
	else{
		if(!pif->Boot(PixieInterface::BootAll)){ return false; }
	}

	// Check the scheduler
	Display::LeaderPrint("Checking scheduler");
	int startScheduler = sched_getscheduler(0);
	if(startScheduler == SCHED_BATCH){ std::cout << Display::InfoStr("BATCH") << std::endl; }
	else if(startScheduler == SCHED_OTHER){ std::cout << Display::InfoStr("STANDARD") << std::endl; }
	else{ std::cout << Display::WarningStr("UNEXPECTED") << std::endl; }

	if(!synch_mods()){ return false; }

	// Allocate memory buffers for FIFO
	n_cards = pif->GetNumberCards();
	
	// Two extra words to store size of data block and module number
	std::cout << "\nAllocating memory to store FIFO data (" << sizeof(word_t) * (EXTERNAL_FIFO_LENGTH + 2) * n_cards / 1024 << " kB)" << std::endl;
	fifoData = new word_t[(EXTERNAL_FIFO_LENGTH + 2) * n_cards];
	
	// Allocate data for partial events
	std::cout << "Allocating memory for partial events (" << sizeof(eventdata_t) * n_cards / 1024 << " kB)" << std::endl;
	partialEventData = new eventdata_t[n_cards];
	for(size_t card = 0; card < n_cards; card++){
		partialEventWords.push_back(0);
		waitWords.push_back(0);
	}

	dataWords = 0;
	statsTime = 0;
	histoTime = 0;

	if(histo_interval != -1.){ 
		std::cout << "Allocating memory to store HISTOGRAM data (" << sizeof(PixieInterface::Histogram)*n_cards*pif->GetNumberChannels()/1024 << " kB)" << std::endl;
		for (unsigned int mod=0; mod < n_cards; mod++){
			for (unsigned int ch=0; ch < pif->GetNumberChannels(); ch++){
			  chanid_t id(mod, ch);
			  histoMap[id] = PixieInterface::Histogram();
			}
		}
	}

	runDone = new bool[n_cards];
	isExiting = false;

	waitCounter = 0;
	nonWaitCounter = 0;
	partialBufferCounter = 0;
	
	init_server(45080); // This is the port # pacman uses (udptoipc actually)
	
	return init = true;
}

Poll::~Poll(){
	if(init){
		close();
	}
}

bool Poll::close(){
	if(!init){ return false; }
	
	close_server();
	
	if(runDone){ delete[] runDone; }
	if(fifoData){ delete[] fifoData; }
	if(partialEventData){ delete[] partialEventData; }
	
	delete pif;
	
	init = false;
}

/* Safely close current data file if one is open. */
bool Poll::close_output_file(){
	if(output_file.IsOpen()){ // A file is already open and must be closed
		if(poll_running){
			std::cout << sys_message_head << "Warning! attempted to close file while acquisition running.\n";
			return false;
		}
		else if(start_run){
			std::cout << sys_message_head << "Warning! attempted to close file while acquisition is starting.\n";
			return false;		
		}
		else if(stop_run){
			std::cout << sys_message_head << "Warning! attempted to close file while acquisition is stopping.\n";
			return false;			
		}
		std::cout << sys_message_head << "Closing output file.\n";
		output_file.CloseFile();
		return true;
	}
	std::cout << sys_message_head << "No file is open.\n";
	return true;
}

bool Poll::synch_mods(){
	static bool firstTime = true;
	static char synchString[] = "IN_SYNCH";
	static char waitString[] = "SYNCH_WAIT";

	bool hadError = false;
	Display::LeaderPrint("Synchronizing");

	if(firstTime){
		// only need to set this in the first module once
		if(!pif->WriteSglModPar(waitString, 1, 0)){ hadError = true; }
		firstTime = false;
	}
	
	for(unsigned int mod = 0; mod < pif->GetNumberCards(); mod++){
		if (!pif->WriteSglModPar(synchString, 0, mod)){ hadError = true; }
	}

	if (!hadError){ std::cout << Display::OkayStr() << std::endl; }
	else{ std::cout << Display::ErrorStr() << std::endl; }

	return !hadError;
}

int Poll::write_data(word_t *data, unsigned int nWords){
	// Broadcast a spill notification to the network
	int packet_size;
	char *packet = output_file.BuildPacket(packet_size);
	std::cout << packet << "\t" << packet_size << std::endl;
	server_send_message(packet, packet_size);

	// Handle the writing of buffers to the file
	return output_file.Write((char*)data, nWords);

	return -1;
}

/* Print help dialogue for POLL options. */
void Poll::help(){
	std::cout << "  Help:\n";
	std::cout << "   quit           - Close the program\n";
	std::cout << "   help (h)       - Display this dialogue\n";
	std::cout << "   version (v)    - Display Poll2 version information\n";
	std::cout << "   status         - Display system status information\n";
	std::cout << "   run (trun)     - Start recording data to disk\n";
	std::cout << "   stop (tstop)   - Stop recording data to disk\n";
	std::cout << "   reboot         - Reboot PIXIE crate\n";
	std::cout << "   force (hup)    - Force dump of current spill\n";
	std::cout << "   debug          - Toggle debug mode flag (default=false)\n";
	std::cout << "   fdir [path]    - Set the output file directory (default='./')\n";
	std::cout << "   ouf [filename] - Set the output filename (default='pixie.xxx')\n";
	std::cout << "   close (clo)    - Safely close the current data output file\n";
	std::cout << "   htit [title]   - Set the title of the current run (default='PIXIE Data File)\n";
	std::cout << "   hnum [number]  - Set the number of the current run (default=0)\n";
	std::cout << "   oform [0,1,2]  - Set the format of the output file (default=0)\n";
	std::cout << "   stats [time]   - Set the time delay between statistics dumps (default=-1)\n";
	//std::cout << "   mca [filename] [time]            - Use MCA to record data for debugging purposes\n";
	std::cout << "   pread [mod] [chan] [param]        - Read parameters from individual PIXIE channels\n";
	std::cout << "   pmread [mod] [param]              - Read parameters from PIXIE modules\n";
	std::cout << "   pwrite [mod] [chan] [param] [val] - Write parameters to individual PIXIE channels\n";
	std::cout << "   pmwrite [mod] [param] [val]       - Write parameters to PIXIE modules\n";
	std::cout << "   csr_test [number]                 - Output the CSRA parameters for a given integer\n";
	std::cout << "   adjust_offsets [module]           - Adjusts the baselines of a pixie module\n";
	std::cout << "   find_tau [module] [channel]       - Finds the decay constant for an active pixie channel\n";
}

/* Print help dialogue for reading/writing pixie channel parameters. */
void Poll::pchan_help(){
	std::cout << "  Valid Pixie16 channel parameters:\n";
	std::cout << "   CHANNEL_CSRA\n";
	std::cout << "   ENERGY_FLATTOP\n";
	std::cout << "   ENERGY_RISETIME\n";
	std::cout << "   TRIGGER_FLATTOP\n";
	std::cout << "   TRIGGER_RISETIME\n";
	std::cout << "   TRIGGER_THRESHOLD\n";
	std::cout << "   TAU\n";
	std::cout << "   TRACE_DELAY\n";
	std::cout << "   TRACE_LENGTH\n";
	std::cout << "   ChanTrigStretch\n";
	std::cout << "   ExternDelayLen\n";
	std::cout << "   ExtTrigStretch\n";
	std::cout << "   FtrigoutDelay\n";
	std::cout << "   FASTTRIGBACKLEN\n";
	std::cout << "   BLCUT\n";
}

/* Print help dialogue for reading/writing pixie module parameters. */
void Poll::pmod_help(){
	std::cout << "  Valid Pixie16 module parameters:\n";
	std::cout << "   MODULE_CSRB\n";
	std::cout << "   MODULE_FORMAT\n";
	std::cout << "   SYNCH_WAIT\n";
	std::cout << "   IN_SYNCH\n";
	std::cout << "   TrigConfig0\n";
	std::cout << "   TrigConfig1\n";
	std::cout << "   TrigConfig2\n";
}

///////////////////////////////////////////////////////////////////////////////
// Poll::command_control
///////////////////////////////////////////////////////////////////////////////

/* Function to control the POLL command line interface */
void Poll::command_control(Terminal *poll_term_){
	char c;
	std::string cmd = "", arg;
	
#ifdef USE_NCURSES
	bool cmd_ready = true;
#else
	bool cmd_ready = false;
#endif
	
	while(true){
#ifdef USE_NCURSES
		cmd = poll_term_->GetCommand();
		poll_term_->print((cmd+"\n").c_str()); // This will force a write before the cout stream dumps to the screen
		if(cmd == "CTRL_D" || cmd == "CTRL_C"){ 
			cmd = "quit";
		}
#else
		read(STDIN_FILENO, &c, 1);
		
		// check for system control commands
		if(c == '\004'){ break; } // ctrl + c
		else if(c == '\n' || c == '\r'){
			cmd_ready = true;
		}
		else if(c == '\033'){
			read(STDIN_FILENO, &c, 1); // skip the '['
			read(STDIN_FILENO, &c, 1);
		}
		else{ cmd += c; }
#endif

		if(cmd_ready){			
			if(cmd == ""){ continue; }
			
			size_t index = cmd.find(" ");
			if(index != std::string::npos){
				arg = cmd.substr(index+1, cmd.size()-index); // Get the argument from the full input string
				cmd = cmd.substr(0, index); // Get the command from the full input string
			}
			
			// check for defined commands
			if(cmd == "quit" || cmd == "exit"){
				if(poll_running){ std::cout << sys_message_head << "Warning! cannot quit while acquisition running\n"; }
				else{ 
					kill_all = true;
					while(!run_ctrl_exit){ sleep(1); }
					break;
				}
			}
			else if(cmd == "kill"){
				if(poll_running){ 
					std::cout << sys_message_head << "Sending KILL signal\n";
					stop_run = true; 
				}
				kill_all = true;
				while(!run_ctrl_exit){ sleep(1); }
				break;
			}
			else if(cmd == "help" || cmd == "h"){ help(); }
			else if(cmd == "version" || cmd == "v"){ std::cout << "  Poll v" << POLL_VERSION << std::endl; }
			else if(cmd == "status"){
				std::cout << "  Poll Status:\n";
				std::cout << "   Run starting - " << yesno(start_run) << std::endl;
				std::cout << "   Run stopping - " << yesno(stop_run) << std::endl;
				std::cout << "   Rebooting    - " << yesno(do_reboot) << std::endl;
				std::cout << "   Acq running  - " << yesno(poll_running) << std::endl;
			}
			else if(cmd == "trun" || cmd == "run"){ start_run = true; } // Tell POLL to start taking data
			else if(cmd == "tstop" || cmd == "stop"){ stop_run = true; } // Tell POLL to stop taking data
			else if(cmd == "reboot"){ // Tell POLL to attempt a PIXIE crate reboot
				if(poll_running){ std::cout << sys_message_head << "Warning! cannot quit while acquisition running\n"; }
				else{ do_reboot = true; }
			}
			else if(cmd == "clo" || cmd == "close"){ close_output_file(); } // Tell POLL to close the current data file
			else if(cmd == "hup" || cmd == "force"){ force_spill = true; } // Force spill write to file
			else if(cmd == "debug"){ // Toggle debug mode
				if(debug_mode){
					std::cout << sys_message_head << "Toggling debug mode OFF\n";
					output_file.SetDebugMode(false);
					debug_mode = false;
				}
				else{
					std::cout << sys_message_head << "Toggling debug mode ON\n";
					output_file.SetDebugMode();
					debug_mode = true;
				}
			}
			else if(cmd == "fdir"){ // Change the output file directory
				output_directory = arg; 
				current_file_num = 0;
				
				// Append a '/' if the user did not include one
				if(*(output_directory.end()-1) != '/'){ output_directory += '/'; }
				std::cout << sys_message_head << "Set output directory to '" << output_directory << "'\n";
			} 
			else if(cmd == "ouf"){ // Change the output file name
				output_filename = arg; 
				current_file_num = 0;
				output_file.SetFilenamePrefix(output_filename);
				std::cout << sys_message_head << "Set output filename to '" << output_filename << "'\n";
			} 
			else if(cmd == "htit"){ // Change the title of the output file
				output_title = arg; 
				std::cout << sys_message_head << "Set run title to '" << output_title << "'\n";
			} 
			else if(cmd == "hnum"){ // Tell POLL to attempt a PIXIE crate reboot
				output_run_num = atoi(arg.c_str()); 
				std::cout << sys_message_head << "Set run number to '" << output_run_num << "'\n";
			} 
			else if(cmd == "oform"){ // Change the output file format
				int format = atoi(arg.c_str());
				if(format == 0 || format == 1 || format == 2){
					output_format = atoi(arg.c_str());
					std::cout << sys_message_head << "Set output file format to '" << output_format << "'\n";
					if(output_format == 1){ std::cout << "  Warning! this output format is experimental and is not recommended\n"; }
					else if(output_format == 2){ std::cout << "  Warning! this output format is slow and should only be used for debugging/troubleshooting\n"; }
					output_file.SetFileFormat(output_format);
				}
				else{ 
					std::cout << sys_message_head << "Unknown output file format ID '" << format << "'\n";
					std::cout << "  Available file formats include:\n";
					std::cout << "   0 - .ldf (HRIBF) file format (default)\n";
					std::cout << "   1 - .pld (PIXIE) file format (experimental)\n";
					std::cout << "   2 - .root file format (slow, not recommended)\n";
				}
			}
			else if(cmd == "stats"){
				stats_interval = atoi(arg.c_str());
				if(stats_interval > 0){ std::cout << sys_message_head << "Dumping statistics information every " << stats_interval << " seconds\n"; } // Stats are turned on
				else{ 
					std::cout << sys_message_head << "Disabling statistics output\n"; 
					stats_interval = -1;
				}
			}
			else if(cmd == "pwrite" || cmd == "pmwrite"){ // Write pixie parameters
				if(poll_running){ 
					std::cout << sys_message_head << "Warning! cannot edit pixie parameters while acquisition is running!\n"; 
					continue;
				}
			
				std::vector<std::string> arguments;
				unsigned int p_args = split_str(arg, arguments);
			
				if(cmd == "pwrite"){ // Syntax "pwrite <module> <channel> <parameter name> <value>"
					if(p_args > 0 && arguments.at(0) == "help"){ pchan_help(); }
					else if(p_args >= 4){
						int mod = atoi(arguments.at(0).c_str());
						int ch = atoi(arguments.at(1).c_str());
						float value = atof(arguments.at(3).c_str());
					
						ParameterChannelWriter writer;
						if(forChannel(pif, mod, ch, writer, make_pair(arguments.at(2), value))){ pif->SaveDSPParameters(); }
					}
					else{
						std::cout << sys_message_head << "Invalid number of parameters to pwrite\n";
						std::cout << sys_message_head << " -SYNTAX- pwrite [module] [channel] [parameter] [value]\n";
					}
				}
				else if(cmd == "pmwrite"){ // Syntax "pmwrite <module> <parameter name> <value>"
					if(p_args > 0 && arguments.at(0) == "help"){ pmod_help(); }
					else if(p_args >= 3){
						int mod = atoi(arguments.at(0).c_str());
						unsigned long value = (unsigned long)atol(arguments.at(2).c_str());
					
						ParameterModuleWriter writer;
						if(forModule(pif, mod, writer, make_pair(arguments.at(1), value))){ pif->SaveDSPParameters(); }
					}
					else{
						std::cout << sys_message_head << "Invalid number of parameters to pmwrite\n";
						std::cout << sys_message_head << " -SYNTAX- pmwrite [module] [parameter] [value]\n";
					}
				}
			}
			else if(cmd == "pread" || cmd == "pmread"){ // Read pixie parameters
				std::vector<std::string> arguments;
				unsigned int p_args = split_str(arg, arguments);
							
				if(cmd == "pread"){ // Syntax "pread <module> <channel> <parameter name>"
					if(p_args > 0 && arguments.at(0) == "help"){ pchan_help(); }
					else if(p_args >= 3){
						int mod = atoi(arguments.at(0).c_str());
						int ch = atoi(arguments.at(1).c_str());
					
						ParameterChannelReader reader;
						forChannel(pif, mod, ch, reader, arguments.at(2));
					}
					else{
						std::cout << sys_message_head << "Invalid number of parameters to pread\n";
						std::cout << sys_message_head << " -SYNTAX- pread [module] [channel] [parameter]\n";
					}
				}
				else if(cmd == "pmread"){ // Syntax "pmread <module> <parameter name>"
					if(p_args > 0 && arguments.at(0) == "help"){ pmod_help(); }
					else if(p_args >= 2){
						int mod = atoi(arguments.at(0).c_str());
					
						ParameterModuleReader reader;
						forModule(pif, mod, reader, arguments.at(1));
					}
					else{
						std::cout << sys_message_head << "Invalid number of parameters to pmread\n";
						std::cout << sys_message_head << " -SYNTAX- pread [module] [parameter]\n";
					}
				}
			}
			else if(cmd == "adjust_offsets"){
				std::vector<std::string> arguments;
				unsigned int p_args = split_str(arg, arguments);
			
				if(p_args >= 1){
					int mod = atoi(arguments.at(0).c_str());
					
					OffsetAdjuster adjuster;
					if(forModule<int>(pif, mod, adjuster)){ pif->SaveDSPParameters(); }
				}
				else{
					std::cout << sys_message_head << "Invalid number of parameters to adjust_offsets\n";
					std::cout << sys_message_head << " -SYNTAX- adjust_offsets [module]\n";
				}
			}
			else if(cmd == "find_tau"){
				std::vector<std::string> arguments;
				unsigned int p_args = split_str(arg, arguments);
			
				if(p_args >= 2){
					int mod = atoi(arguments.at(0).c_str());
					int ch = atoi(arguments.at(1).c_str());

					TauFinder finder;
					forChannel<int>(pif, mod, ch, finder);
				}
				else{
					std::cout << sys_message_head << "Invalid number of parameters to find_tau\n";
					std::cout << sys_message_head << " -SYNTAX- find_tau [module] [channel]\n";
				}
			}
			else if(cmd == "csr_test"){
				std::vector<std::string> arguments;
				unsigned int p_args = split_str(arg, arguments);
				
				if(p_args >= 1){ CSRA_test(atoi(arguments.at(0).c_str())); }
				else{
					std::cout << sys_message_head << "Invalid number of parameters to csr_test\n";
					std::cout << sys_message_head << " -SYNTAX- csr_test [number]\n";
				}
			}
			else{ std::cout << sys_message_head << "Unknown command '" << cmd << "'\n"; }
			std::cout << std::endl;

#ifndef USE_NCURSES
			cmd = "";
			cmd_ready = false;
#endif
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Poll::run_control
///////////////////////////////////////////////////////////////////////////////

/// Function to control the gathering and recording of PIXIE data
void Poll::run_control(){
	std::vector<word_t> nWords(n_cards);
	std::vector<word_t>::iterator maxWords;
	parseTime = waitTime = readTime = 0.;

  read_again:
	while(true){
		if(kill_all){ // Supercedes all other commands
			if(poll_running){ stop_run = true; }
			else{ break; }
		}
		
		if(do_reboot){ // Attempt to reboot the PIXIE crate
			if(poll_running){ stop_run = true; }
			else{
				std::cout << sys_message_head << "Attempting PIXIE crate reboot\n";
				pif->Boot(PixieInterface::BootAll);
				do_reboot = false;
			}
		}

		// MAIN DATA ACQUISITION SECTION!!!
		if(poll_running){
			if(start_run){
				std::cout << sys_message_head << "Already running!\n";
				start_run = false;
			}
			
			// Data acquisition
		    // Check if it's time to dump statistics
			if(stats_interval != -1 && usGetTime(startTime) > lastStatsTime + stats_interval * 1e6){
				usGetDTime(); // start timer
				for (size_t mod = 0; mod < n_cards; mod++) {
					pif->GetStatistics(mod);
					PixieInterface::stats_t &stats = pif->GetStatisticsData();
					fifoData[dataWords++] = PixieInterface::STAT_SIZE + 3;
					fifoData[dataWords++] = mod;
					fifoData[dataWords++] = ((PixieInterface::STAT_SIZE + 1) << 17 ) & ( 1 << 12 ) & ( pif->GetSlotNumber(mod) << 4);
					memcpy(&fifoData[dataWords], &stats, sizeof(stats));
					dataWords += PixieInterface::STAT_SIZE;

					if(!is_quiet){
						std::cout << "\nSTATS " << mod << " : ICR ";
						for (size_t ch = 0; ch < pif->GetNumberChannels(); ch++) {
							std::cout.precision(2);
							std::cout << " " << pif->GetInputCountRate(mod, ch);
						}
						std::cout << std::endl << std::flush;
					}
				}
				write_data(fifoData, dataWords);
				dataWords = 0;
				statsTime += usGetDTime();
				lastStatsTime = usGetTime(startTime);
			}
			
			// check whether it is time to dump histograms
			if(histo_interval != -1 && usGetTime(startTime) > lastHistoTime + histo_interval * 1e6){
				usGetDTime(); // start timer
				std::ofstream out("histo.dat", std::ios::trunc);
				std::ofstream deltaout("deltahisto.dat", std::ios::trunc);

				for (size_t mod=0; mod < n_cards; mod++) {
					for (size_t ch=0; ch < pif->GetNumberChannels(); ch++) {
						chanid_t id(mod, ch);
						PixieInterface::Histogram &histo = histoMap[id];

						// Copy the old histogram data to the delta histogram temporarily
						deltaHisto = histo;
						
						// Performance improvement possible using Pixie16EMbufferIO directly to fetch all channels
						histo.Read(*pif, mod, ch);
						histo.Write(out);
						
						// Calculate the change using the temporarily stored previous histogram
						deltaHisto = histo - deltaHisto;
						deltaHisto.Write(deltaout);
					}
				}
				out.close();
				deltaout.close();

				histoTime += usGetDTime();
				lastHistoTime = usGetTime(startTime);
			}
						
			// Check whether we have any data
			usGetDTime(); // Start timer
			for (unsigned int timeout = 0; timeout < (stop_run ? 1 : POLL_TRIES); timeout++){ // See if the modules have any data for us
				for (size_t mod = 0; mod < n_cards; mod++) {
					if(!runDone[mod]){ nWords[mod] = pif->CheckFIFOWords(mod); }
					else{ nWords[mod] = 0; }
				}
				maxWords = std::max_element(nWords.begin(), nWords.end());
				if(*maxWords > threshWords){ break; }
			}
			time(&pollClock);
			pollTime = usGetDTime();
			
			int maxmod = maxWords - nWords.begin();
			bool readData = (*maxWords > threshWords || stop_run);
			if(force_spill){
				if(!readData){ readData = true; }
				force_spill = false;
			}
			if(readData){
				// if not timed out, we have data to read	
				// read the data, starting with the module with the most words			
				int mod = maxmod;			
				mod = maxmod = 0; //! tmp, read out in a fixed order

				do{
					bool fullFIFO = (nWords[mod] == EXTERNAL_FIFO_LENGTH);
					if(nWords[mod] > 0){
						usGetDTime(); // Start read timer
						word_t &bufferLength = fifoData[dataWords];

						// fifoData[dataWords++] = nWords[mod] + 2
						fifoData[dataWords++] = nWords[mod] + partialEventWords[mod] + 2;
						fifoData[dataWords++] = mod;
						word_t beginData = dataWords;

						// Only add to fifo stream if we have enough words to complete event?
						if(partialEventWords[mod] > 0){
							memcpy(&fifoData[dataWords], partialEventData[mod], sizeof(word_t) * partialEventWords[mod]);
							dataWords += partialEventWords[mod];
							partialEventWords[mod] = 0;
						}

						if(!pif->ReadFIFOWords(&fifoData[dataWords], nWords[mod], mod)){
							std::cout << "Error reading FIFO, bailing out!" << std::endl;
							// Something is wrong
							//BailOut(send_alarm, alarmArgument);
						} 
						else{
							word_t parseWords = beginData;
							word_t eventSize;

							waitWords[mod] = 0; // No longer waiting (hopefully)
							readTime += usGetDTime(); // And starts parse timer
						
							do{ // Unfortuantely, we have to parse the data to make sure, we grabbed complete events
								word_t slotRead = ((fifoData[parseWords] & 0xF0) >> 4);
								word_t chanRead = (fifoData[parseWords] & 0xF);
								word_t slotExpected = pif->GetSlotNumber(mod);
								bool virtualChannel = ((fifoData[parseWords] & 0x20000000) != 0);

								eventSize = ((fifoData[parseWords] & 0x1FFE0000) >> 17);
								if(!virtualChannel && statsHandler){ statsHandler->AddEvent(mod, chanRead, sizeof(word_t) * eventSize); }

								if(eventSize == 0 || slotRead != slotExpected){
									if( slotRead != slotExpected ){ std::cout << "Slot read (" << slotRead << ") not the same as" << " module expected (" << slotExpected << ")" << std::endl; }
									if(eventSize == 0){ std::cout << "ZERO EVENT SIZE" << std::endl; }
									std::cout << "First header words: " << std::hex << fifoData[parseWords] << " " << fifoData[parseWords + 1] << " " << fifoData[parseWords + 2];
									std::cout << " at position " << std::dec << parseWords << "\n	parse started at position " << beginData << " reading " << nWords[mod] << " words." << std::endl;
									//! how to proceed from here
									// BailOut(send_alarm, alarmArgument);
									//--------- THIS IS A ROUGH HACK TO FIX THE CORRUPT DATA ISSUE
									goto read_again;
								}
								parseWords += eventSize;				
							} while(parseWords < dataWords + nWords[mod]);		 
							parseTime += usGetDTime();

							if(parseWords > dataWords + nWords[mod]){
								waitCounter++;
								// If we have ended the run, we should have all the words
								if(stop_run){
									std::cout << Display::ErrorStr("Words missing at end of run.") << std::endl;
									//BailOut(send_alarm, alarmArgument);
								} 
								else{ // we have a deficit of words, now we must wait for the remainder
									if( fullFIFO ){ // the FIFO was full so the rest of the partial event is likely lost
										parseWords -= eventSize;
										// update the buffer length
										nWords[mod]	= parseWords;
										bufferLength = nWords[mod] + 2;
									} 
									else{
										waitWords[mod] = parseWords - (dataWords + nWords[mod]);
										unsigned int timeout = 0;

										usGetDTime(); // start wait timer

										if(!is_quiet){ std::cout << "Waiting for " << waitWords[mod] << " words in module " << mod << std::flush; }

										word_t testWords;

										while (timeout++ < WAIT_TRIES){
											testWords = pif->CheckFIFOWords(mod);
											if(testWords >= std::max(waitWords[mod], 2U)){ break; }
										} 
										waitTime += usGetDTime();

										if(timeout >= WAIT_TRIES){
											if(!is_quiet){ std::cout << " --- TIMED OUT," << std::endl << Display::InfoStr("\t\tmoving partial event to next buffer") << std::endl; }
											partialBufferCounter++;
											partialEventWords[mod] = eventSize - waitWords[mod];
											memcpy(partialEventData[mod], &fifoData[dataWords + nWords[mod] - partialEventWords[mod]], sizeof(word_t) * partialEventWords[mod]);
											nWords[mod] = parseWords - beginData - eventSize;

											// Update the size of the buffer;
											bufferLength = nWords[mod] + 2;
										} 
										else{
											if(!is_quiet){ std::cout << std::endl; }
											int testWords = pif->CheckFIFOWords(mod);
											if(!pif->ReadFIFOWords(&fifoData[dataWords + nWords[mod]], waitWords[mod], mod)){
												std::cout << "Error reading FIFO, bailing out!" << std::endl;

												//BailOut(send_alarm, alarmArgument); // Something is wrong 
												kill_all = true;
											} 
											else{
												nWords[mod] += waitWords[mod];
											
												// no longer waiting for words
												waitWords[mod] = 0;
											
												// and update the length of the buffer
												bufferLength = nWords[mod] + 2;
											} // check success of read
										} // if we DID NOT time out waiting for words
									} // if we DID NOT have a full FIFO
								} // if we ARE NOT on the final read at the end of a run
							} // if there are words remaining	
							else{ nonWaitCounter++; }
						} // check success of read
					} 
					else{ // If we had any words
						// Write an empty buffer if there is no data
						fifoData[dataWords++] = 2;
						fifoData[dataWords++] = mod;			
					}
					if(nWords[mod] > EXTERNAL_FIFO_LENGTH * 9 / 10){
						std::cout << "Abnormally full FIFO with " << nWords[mod] << " words in module " << mod << std::endl;
						if(fullFIFO){
							pif->EndRun();
							stop_run = true;
						}
					}
					if(!is_quiet){
						if(fullFIFO){ std::cout << "Read " << Display::WarningStr("full FIFO") << " in"; }
						else{ std::cout << "Read " << nWords[mod] << " words from"; }
					
						std::cout << " module " << mod << " to buffer position " << dataWords;
						if(partialEventWords[mod] > 0){ std::cout << ", " << partialEventWords[mod] << " words reserved."; }
						std::cout << std::endl;
					}
					dataWords += nWords[mod];
	
					// Read the remainder of the modules in a modulo ring
					mod = (mod + 1) % n_cards;
				} while(mod != maxmod);
			} // If we have data to read 

			// If we don't have enough words, poll socket and modules once more
			if(!readData){ continue; }
			
			if(insert_wall_clock){
				// Add the "wall time" in artificially
				size_t timeWordsNeeded = sizeof(time_t) / sizeof(word_t);
				if((sizeof(time_t) % sizeof(word_t)) != 0){ timeWordsNeeded++; }
				fifoData[dataWords++] = 2 + timeWordsNeeded;
				fifoData[dataWords++] = clock_vsn;
				memcpy(&fifoData[dataWords], &pollClock, sizeof(time_t));
				if(!is_quiet){ std::cout << "Read " << timeWordsNeeded << " words for time to buffer position " << dataWords << std::endl; }
				dataWords += timeWordsNeeded;
			}
			
			spillTime = usGetTime(startTime);
			durSpill = spillTime - lastSpillTime;
			lastSpillTime = spillTime;

			usGetDTime(); // start send timer
			
			int nBufs = write_data(fifoData, dataWords);
			sendTime = usGetDTime();

			if(statsHandler){ statsHandler->AddTime(durSpill * 1e-6); }
			
			if (!is_quiet) {
				std::cout << nBufs << " BUFFERS with " << dataWords << " WORDS, " << std::endl;
				std::cout.setf(std::ios::scientific, std::ios::floatfield);
				std::cout.precision(1);
				std::cout << "    SPILL " << durSpill << " us " << " POLL  " << pollTime << " us " << " PARSE " << parseTime << " us" << std::endl;
				std::cout << "    WAIT  " << waitTime << " us " << " READ  " << readTime << " us " << " SEND  " << sendTime << " us" << std::endl;	 
				
				// Add some blank spaces so STATS or HISTO line up
				std::cout << "   ";
				if(stats_interval != -1){ std::cout << " STATS " << statsTime << " us "; }
				if(histo_interval != -1){ std::cout << " HISTO " << histoTime << " us "; }
				if(stats_interval != -1 || histo_interval != -1){ std::cout << std::endl; }
				std::cout << std::endl;
			} 
			else{
				std::cout.setf(std::ios::scientific, std::ios::floatfield);
				std::cout.precision(1);

				if(!show_module_rates){ std::cout << nBufs << " bufs : " << "SEND " << sendTime << " / SPILL " << durSpill << "     \r"; } 
				else if(statsHandler){      
					for(size_t i=0; i < n_cards; i++){
						std::cout << "M" << i << ", " << statsHandler->GetEventRate(i) / 1000. << " kHz";
						std::cout << " (" << statsHandler->GetDataRate(i) / 1000000. << " MB/s)";
					}  
					std::cout << "    \r";
				}
			}
			
			// Reset the number of words of fifo data
			dataWords = 0;
			histoTime = statsTime = 0;
			if(stop_run){ 
				time(&raw_time);
				time_info = localtime(&raw_time);
				std::cout << sys_message_head << "Stopping run at " << asctime(time_info);
				pif->EndRun();
				
				stop_run = false;
				poll_running = false;
				usleep(END_RUN_PAUSE);	
				
				// Update whether the run has ended with the data read out
				for(size_t mod = 0; mod < n_cards; mod++){
					if(!pif->CheckRunStatus(mod)){
						runDone[mod] = true;
						std::cout << "Run ended in module " << mod << std::endl;
					}
					if(pif->CheckRunStatus(mod)){
						std::cout << "Run not properly finished in module " << mod << std::endl;
					}
				}			
			}
		}
		else{ 	
			if(start_run){
				if(!output_file.IsOpen()){ 
					if(!output_file.OpenNewFile(output_title, output_run_num, current_filename, output_directory)){
						std::cout << sys_message_head << "Failed to open output file! Check that the path is correct.\n";
						start_run = false;
						continue;
					}
					std::cout << sys_message_head << "Opening output file '" << current_filename << "'.\n";
				}
				if(zero_clocks){ synch_mods(); }
				for(size_t mod=0; mod < n_cards; mod++){ runDone[mod] = false; }
				lastHistoTime = lastStatsTime = lastSpillTime = usGetTime(startTime);
				
				time(&raw_time);
				time_info = localtime(&raw_time);
				std::cout << sys_message_head << "Starting run at " << asctime(time_info);				
				if(pif->StartListModeRun(LIST_MODE_RUN, NEW_RUN)){
					poll_running = true;
					nonWaitCounter = 0;
				}
				else{ std::cout << sys_message_head << "Failed to start list mode run. Try rebooting PIXIE\n"; }
				start_run = false;
			}
			else if(stop_run){
				std::cout << sys_message_head << "Not running!\n";
				stop_run = false;
			}
			sleep(1); 
		}
	}

	if(waitCounter + nonWaitCounter != 0){ 
		std::cout << "Waiting for " << waitCounter * 100 / (waitCounter + nonWaitCounter) << "% of the spills." << std::endl;
		std::cout << "  " << partialBufferCounter << " partial buffers" << std::endl;
	}

	run_ctrl_exit = true;
}

///////////////////////////////////////////////////////////////////////////////
// Pixie16 parameter functions
///////////////////////////////////////////////////////////////////////////////

bool ParameterChannelWriter::operator()(PixieFunctionParms< std::pair<std::string, float> > &par){
	if(par.pif.WriteSglChanPar(par.par.first.c_str(), par.par.second, par.mod, par.ch)){
		par.pif.PrintSglChanPar(par.par.first.c_str(), par.mod, par.ch);
		return true;
	}
	return false;
}

bool ParameterModuleWriter::operator()(PixieFunctionParms< std::pair<std::string, unsigned long> > &par){
	if(par.pif.WriteSglModPar(par.par.first.c_str(), par.par.second, par.mod)){
		par.pif.PrintSglModPar(par.par.first.c_str(), par.mod);
		return true;
	} 
	return false;
}

bool ParameterChannelReader::operator()(PixieFunctionParms<std::string> &par){
	par.pif.PrintSglChanPar(par.par.c_str(), par.mod, par.ch);
	return true;
}

bool ParameterModuleReader::operator()(PixieFunctionParms<std::string> &par){
	par.pif.PrintSglModPar(par.par.c_str(), par.mod);
	return true;
}

bool OffsetAdjuster::operator()(PixieFunctionParms<int> &par){
	bool hadError = par.pif.AdjustOffsets(par.mod);
	for(size_t ch = 0; ch < par.pif.GetNumberChannels(); ch++){
		par.pif.PrintSglChanPar("VOFFSET", par.mod, ch);
	}

	return hadError;
}

bool TauFinder::operator()(PixieFunctionParms<> &par){
	double tau[16];
  
	int errorNum = Pixie16TauFinder(par.mod, tau);
	if(par.ch < 16){
		std::cout << "TAU: " << tau[par.ch] << std::endl;
	}
	std::cout << "Errno: " << errorNum << std::endl;

	return (errorNum >= 0);
}

void CSRA_test(int input_){
	const size_t num_bits = 19;

	std::string CSR_TXT[num_bits];

#ifdef PIF_REVA
	CSR_TXT[0] = "Respond to group triggers only";
	CSR_TXT[1] = "Measure individual live time";
	CSR_TXT[3] = "Read always";
	CSR_TXT[4] = "Enable trigger";
	CSR_TXT[6] = "GFLT";
#else  // Rev. A
	CSR_TXT[CCSRA_TRACEENA] = "Enable trace capture";
	CSR_TXT[CCSRA_QDCENA]   = "Enable QDC sums capture";
	CSR_TXT[10] = "Enable CFD trigger mode";
	CSR_TXT[11] = "Enable global trigger validation";
	CSR_TXT[12] = "Enable raw energy sums capture";
	CSR_TXT[13] = "Enable channel trigger validation";
	CSR_TXT[15] = "Pileup rejection control";
	CSR_TXT[16] = "Hybrid bit";
	CSR_TXT[18] = "SHE single trace capture";
#endif // (else) Rev.A

	CSR_TXT[CCSRA_GOOD]     = "Good Channel";
	CSR_TXT[CCSRA_POLARITY] = "Trigger positive";
	CSR_TXT[CCSRA_ENARELAY] = "HI/LO gain";

	std::cout << "  Input: " << std::dec << input_ << " (0x" << std::hex << input_ << ")\n\n";
	std::cout << "   CSRA bits:\n";

	size_t max_len = 0;
	for (size_t k = 0; k < num_bits; k++){ 
		if(CSR_TXT[k].length() > max_len)
			max_len = CSR_TXT[k].length();
	}

	for(size_t k = 0; k < num_bits; k++){
		int retval = APP32_TstBit(k, input_);
		std::cout << "   " << retval << " " << std::setw(max_len) << CSR_TXT[k] << "  " << std::setw(2) << k << "  " << (1 << k) * retval << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Support Functions
///////////////////////////////////////////////////////////////////////////////

unsigned int split_str(std::string str_, std::vector<std::string> &args, char delimiter_){
	args.clear();
	std::string temp = "";
	unsigned int count = 0;
	for(unsigned int i = 0; i < str_.size(); i++){
		if(str_[i] == delimiter_ || i == str_.size()-1){
			if(i == str_.size()-1){ temp += str_[i]; }
			args.push_back(temp);
			temp = "";
			count++;
		}
		else{ temp += str_[i]; }		
	}
	return count;
}

/* Pad a string with '.' to a specified length. */
std::string pad_string(const std::string &input_, unsigned int length_){
	std::string output = input_;
	for(unsigned int i = input_.size(); i <= length_; i++){
		output += '.';
	}
	return output;
}

std::string yesno(bool value_){
	if(value_){ return "Yes"; }
	return "No";
}

template<typename T>
bool forChannel(PixieInterface *pif, int mod, int ch, PixieFunction<T> &f, T par){
    PixieFunctionParms<T> parms(*pif, par);
    
    bool hadError = false;
    
	if(mod < 0){
		for(parms.mod = 0; parms.mod < pif->GetNumberCards(); parms.mod++){
			if(ch < 0){
				for (parms.ch = 0; parms.ch < pif->GetNumberChannels(); parms.ch++) {
					if(!f(parms)){ hadError = true; }
				}
			} 
			else{
				parms.ch = ch;
				if(!f(parms)){ hadError = true; }
			}
		}
	} 
	else{
		parms.mod = mod;
		if(ch < 0){
			for(parms.ch = 0; parms.ch < pif->GetNumberChannels(); parms.ch++){
				if(!f(parms)){ hadError = true; }
			}
		} 
		else{
			parms.ch = ch;
			hadError = !f(parms);
		}
	}

    return !hadError;
}

template<typename T>
bool forModule(PixieInterface *pif, int mod, PixieFunction<T> &f, T par){
    PixieFunctionParms<T> parms(*pif, par);
    bool hadError = false;
    
	if(mod < 0){
		for(parms.mod = 0; parms.mod < pif->GetNumberCards(); parms.mod++){
			if(!f(parms)){ hadError = true; }
		}
	} 
	else{
		parms.mod = mod;
		hadError = !f(parms);
	}
    
    return !hadError;
}
