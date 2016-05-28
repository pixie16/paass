/** \file ScanInterface.cpp
 * \brief A class to handle reading from various UTK/ORNL pixie16 data formats.
 *
 * This class is intended to be used as a replacement to the older and unsupported
 * 'scanor' program from the UPAK acq library specifically for .ldf files which are
 * constructed using the UTK/ORNL pixie16 style. This class also interfaces with poll2
 * shared memory output without the need to use pacman.
 * CRT
 *
 * \author C. R. Thornsberry
 * \date Feb. 12th, 2016
 */
#include <iostream>
#include <sstream>
#include <thread>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "Unpacker.hpp"
#include "poll2_socket.h"
#include "CTerminal.h"

#include "ScanInterface.hpp"

#if not defined(PROG_NAME)
#define PROG_NAME "ScanInterface"
#endif

void start_run_control(ScanInterface *main_){
	main_->RunControl();
}

void start_cmd_control(ScanInterface *main_){
	main_->CmdControl();
}

/////////////////////////////////////////////////////////////////////
// class fileInformation
/////////////////////////////////////////////////////////////////////

bool fileInformation::at(const size_t &index_, std::string &name, std::string &value){
	if(index_ >= parnames.size()){ return false; }
	name = parnames.at(index_);
	value = parvalues.at(index_);
	return true;
}

template <typename T>
bool fileInformation::push_back(const std::string & name_, const T &value_, const std::string &units_/*=""*/){
	if(!is_in(name_)){
		std::stringstream stream;
		stream << value_;
		if(units_.size() > 0){ 
			stream << " " << units_; 
		}
		parnames.push_back(name_);
		parvalues.push_back(stream.str());
		return true;
	}
	return false;
}

bool fileInformation::is_in(const std::string & name_){
	for(std::vector<std::string>::iterator iter = parnames.begin(); iter != parnames.end(); iter++){
		if(name_ == (*iter)){ return true; }
	}
	return false;
}

std::string fileInformation::print(const size_t &index_){
	if(index_ >= parnames.size()){ return ""; }
	return std::string(parnames.at(index_) + ": " + parvalues.at(index_));
}

void fileInformation::clear(){
	parnames.clear();
	parvalues.clear();
}

/////////////////////////////////////////////////////////////////////
// class ScanInterface
/////////////////////////////////////////////////////////////////////

/** Split a string into arguments using a specified delimiter.
  * \param[in]  str_       The input string.
  * \param[out] args       A vector containing the arguments from the input string.
  * \param[in]  delimiter_ The character about which to split the input string.
  * \return The number of sub-strings which were found.
  */
unsigned int ScanInterface::split_str(std::string str_, std::vector<std::string> &args, char delimiter_/*=' '*/){
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

/** Get the file extension from an input filename string.
  * \param[in]  filename_ The full input filename path.
  * \param[out] prefix    The input filename path without the file extension.
  * \return The file extension string.
  */
std::string ScanInterface::get_extension(std::string filename_, std::string &prefix){
	size_t count = 0;
	size_t last_index = 0;
	std::string output = "";
	prefix = "";
	
	if(filename_.find('.') != std::string::npos){
		// Find the final period in the filename
		for(count = 0; count < filename_.size(); count++){
			if(filename_[count] == '.'){ last_index = count; }
		}
	
		// Get the filename prefix and the extension
		for(size_t i = 0; i < count; i++){
			if(i < last_index){ prefix += filename_[i]; }
			else if(i > last_index){ output += filename_[i]; }
		}
	}
	else{ // The filename has no extension.
		prefix = filename_;
	}
	
	return output;
}

/** Start the scan, if ScanInterface is initialized and is not already running.
  * \return Nothing.
  */
void ScanInterface::start_scan(){
	if(!file_open)
		std::cout << " No input file loaded.\n";
	else if(!input_file.good())
		std::cout << " Error reading from input file!\n";
	else if(input_file.eof())
		std::cout << " Physical end-of-file reached.\n";
	else if(is_running)
		std::cout << " Already running.\n";
	else{
		core->Run();
		is_running = true;
		total_stopped = false;
		if(!batch_mode){ term->SetStatus("\033[0;33m[IDLE]\033[0m Waiting for Unpacker..."); }
		
		// Notify that the user has started the scan.
		Notify("START_SCAN");		
	}
}

/** Stop the scan, if it is running.
  * \return Nothing.
  */
void ScanInterface::stop_scan(){
	if(!scan_init){ std::cout << " Not initialized!\n"; }
	else if(!is_running){ std::cout << " Not running.\n"; }
	else{
		core->Stop();
		is_running = false;
	}
	
	// Notify that the user has stopped the scan.
	Notify("STOP_SCAN");
}

/** Print a command line argument help dialogue.
  * \param[in]  name_ The name of the program.
  * \return Nothing.
  */
void ScanInterface::help(char *name_){
	SyntaxStr(name_);
	std::cout << "  Available options:\n";
	std::cout << "   --help (-h)    - Display this dialogue\n";
	std::cout << "   --version (-v) - Display version information\n";
	std::cout << "   --debug        - Enable readout debug mode\n";
	std::cout << "   --shm          - Enable shared memory readout\n";
	std::cout << "   --quiet        - Toggle off verbosity flag\n";
	std::cout << "   --counts       - Write all recorded channel counts to a file\n";
	std::cout << "   --batch        - Run in batch mode (i.e. with no command line)\n";
	std::cout << "   --dry-run      - Extract spills from file, but do no processing\n";
	std::cout << "   --fast-fwd [word] - Skip ahead to a specified word in the file (start of file at zero)\n";
	ArgHelp();
}

/** Seek to a specified position in the file.
  * \param[in]  offset_ The position, with respect to the start of the file, to seek to.
  * \return True upon success and false otherwise.
  */
bool ScanInterface::rewind(const unsigned long &offset_/*=0*/){
	if(!scan_init){ return false; }

	// Ensure that the scan is not running.
	if(is_running){ 
		std::cout << " Cannot change file position while scan is running!\n";
		return false;
	}

	// Move to the first word in the file.
	std::cout << " Seeking to word no. " << offset_ << " in file\n";
	input_file.seekg(offset_*4, input_file.beg);
	std::cout << " Input file is now at " << input_file.tellg() << " bytes\n";

	// Notify that the user has rewound to the start of the file.
	Notify("REWIND_FILE");

	return true;
}

/** Open a new binary input file for reading.
  * \param[in]  fname_ Input filename to open for reading.
  * \return True upon successfully opening the file and false otherwise.
  */
bool ScanInterface::open_input_file(const std::string &fname_){
	if(is_running){ 
		std::cout << " ERROR! Unable to open input file while scan is running.\n"; 
		return false;
	}
	else if(shm_mode){
		std::cout << " ERROR! Unable to open input file in shm mode.\n"; 
		return false;
	}
	
	extension = get_extension(fname_, prefix);
	if(prefix == ""){
		std::cout << " ERROR! Input filename was not specified!\n";
		return false;
	}

	if(extension == "ldf"){ // List data format file
		file_format = 0;
	}
	else if(extension == "pld"){ // Pixie list data file format
		file_format = 1;
	}
	else{
		std::cout << " ERROR! Invalid file format '" << extension << "'\n";
		std::cout << "  The current valid data formats are:\n";
		std::cout << "   ldf - list data format (HRIBF)\n";
		std::cout << "   pld - pixie list data format\n";
		return false;
	}

	// Close the previous file, if one is open.
	if(file_open){
		std::cout << " Note: Closing previously opened file.\n";
		input_file.close();
	}

	file_open = true;

	// Load the input file.
	input_file.open(fname_.c_str(), std::ios::binary);
	if(!input_file.is_open() || !input_file.good()){
		std::cout << " ERROR! Failed to open input file '" << fname_ << "'! Check that the path is correct.\n";
		input_file.close();
		file_open = false;
		return false;
	}
	input_file.seekg(0, input_file.end);
 	file_length = input_file.tellg();
 	input_file.seekg(0, input_file.beg);

	if(!shm_mode){
		// Clear the file information container.
		finfo.clear();
	
		// Start reading the file
		// Every poll2 ldf file starts with a DIR buffer followed by a HEAD buffer
		int num_buffers;
		if(file_format == 0){
			dirbuff.Read(&input_file, num_buffers);
			headbuff.Read(&input_file);
			
			// Store the file information for later use.
			finfo.push_back("Run number", dirbuff.GetRunNumber());
			finfo.push_back("Number buffers", num_buffers);
			finfo.push_back("Facility", headbuff.GetFacility());
			finfo.push_back("Format", headbuff.GetFormat());
			finfo.push_back("Type", headbuff.GetType());
			finfo.push_back("Date", headbuff.GetDate());
			finfo.push_back("Title", headbuff.GetRunTitle());
			
			// Let's read out the file information from these buffers
			std::cout << " 'DIR ' buffer-\n";
			std::cout << "  " << finfo.print(0) << "\n";
			std::cout << "  " << finfo.print(1) << "\n";
			std::cout << " 'HEAD' buffer-\n";
			std::cout << "  " << finfo.print(2) << "\n";
			std::cout << "  " << finfo.print(3) << "\n";
			std::cout << "  " << finfo.print(4) << "\n";
			std::cout << "  " << finfo.print(5) << "\n";
			std::cout << "  " << finfo.print(6) << "\n";
			std::cout << "  Run number: " << headbuff.GetRunNumber() << "\n\n";
		}
		else if(file_format == 1){
			pldHead.Read(&input_file);
			
			max_spill_size = pldHead.GetMaxSpillSize();

			// Store the file information for later use.
			finfo.push_back("Facility", pldHead.GetFacility());
			finfo.push_back("Format", pldHead.GetFormat());
			finfo.push_back("Start", pldHead.GetStartDate());
			finfo.push_back("Stop", pldHead.GetEndDate());
			finfo.push_back("Title", pldHead.GetRunTitle());
			finfo.push_back("Run number", pldHead.GetRunNumber());
			finfo.push_back("Max spill", max_spill_size, "words");
			finfo.push_back("ACQ time", pldHead.GetRunTime(), "seconds");
			
			// Let's read out the file information from this buffer
			std::cout << " 'HEAD' buffer-\n";
			std::cout << "  " << finfo.print(0) << "\n";
			std::cout << "  " << finfo.print(1) << "\n";
			std::cout << "  " << finfo.print(2) << "\n";
			std::cout << "  " << finfo.print(3) << "\n";
			std::cout << "  " << finfo.print(4) << "\n";
			std::cout << "  " << finfo.print(5) << "\n";
			std::cout << "  " << finfo.print(6) << "\n";
			std::cout << "  " << finfo.print(7) << "\n\n";			
		}
	}

	// Notify that the user has loaded a new file.
	Notify("LOAD_FILE");
	
	return true;	
}

/** ExtraArguments is used to send command line arguments to classes derived
  * from ScanInterface. If ScanInterface receives an unrecognized
  * argument from the user, it will pass it on to the derived class.
  * Does nothing useful by default.
  * \param[in]  arg_    The argument to interpret.
  * \param[out] others_ The remaining arguments following arg_.
  * \param[out] ifname  The input filename to send back to use for reading. Set to arg_ by default.
  * \return True if the argument was recognized and false otherwise. Returns true by default.
  */
bool ScanInterface::ExtraArguments(const std::string &arg_, std::deque<std::string> &others_, std::string &ifname){
	ifname = arg_;
	return true;
}

/** SyntaxStr is used to print a linux style usage message to the screen.
  * Prints a standard usage message by default.
  * \param[in]  name_ The name of the program.
  * \return Nothing.
  */
void ScanInterface::SyntaxStr(char *name_){
	std::cout << " usage: " << name_ << " [input] [options] [output]\n";
}

/** Initialize the Unpacker object. 
  * Does nothing useful by default.
  * \param[in]  prefix_ String to append to the beginning of system output.
  * \return True upon successfully initializing and false otherwise.
  */
bool ScanInterface::Initialize(std::string prefix_){
	if(scan_init){ return false; }
	return (scan_init = true);
}

/** Return a pointer to the Unpacker object to use for data unpacking.
  * If no object has been initialized, create a new one.
  * \return Pointer to an Unpacker object.
  */
Unpacker *ScanInterface::GetCore(){ 
	if(!core){ core = new Unpacker(); }
	return core;
}

/** Default constructor.
  * \param[in]  core_ Pointer to an object derived from Unpacker.
  */
ScanInterface::ScanInterface(Unpacker *core_/*=NULL*/){
	prefix = "";
	extension = "";
	
	// Get the current working directory.
	char workingDirectory[1024];
	workDir = std::string(getcwd(workingDirectory, 1024));
	
	// Get the home directory.
	homeDir = getenv("HOME");

	maxShmSizeL = 4052;
	maxShmSize  = maxShmSizeL * 4;

	max_spill_size = 0;
	file_format = -1;

	file_start_offset = 0;
	num_spills_recvd = 0;
	
	total_stopped = true;
	write_counts = false;
	is_running = false;
	is_verbose = true;
	debug_mode = false;
	dry_run_mode = false;
	shm_mode = false;
	batch_mode = false;
	scan_init = false;
	file_open = false;

	kill_all = false;
	run_ctrl_exit = false;

	poll_server = NULL;
	term = NULL;
	
	// Set the Unpacker pointer, if one is specified.
	if(core_){ core = core_; }
	else{ core = NULL; }

	progName = std::string(PROG_NAME);
	msgHeader = progName + ": ";
}

/// Default destructor.
ScanInterface::~ScanInterface(){
	Close();
}

/// Main scan control method.
void ScanInterface::RunControl(){
	// Notify that we are starting run control.
	run_ctrl_exit = false;

	// Set debug mode, if enabled.
	if(debug_mode){
		pldHead.SetDebugMode();
		pldData.SetDebugMode();
		dirbuff.SetDebugMode();
		headbuff.SetDebugMode();
		databuff.SetDebugMode();
		eofbuff.SetDebugMode();
	}

	while(true){
		if(kill_all){ break; }

		// Now we're ready to read the first data buffer
		if(total_stopped){
			// Sleep while waiting for the user to scan more data.
			sleep(1);
			continue;
		}
		else if(shm_mode){
			std::cout << std::endl;
			unsigned int data[250000]; // Array for storing spill data. Larger than any RevF spill should be.
			unsigned int *shm_data = new unsigned int[maxShmSizeL]; // Array to store the temporary shm data (~16 kB)
			int dummy;
			int previous_chunk;
			int current_chunk;
			int total_chunks;
			int nWords;
			unsigned int nTotalWords;
	
			bool full_spill = false;
		
			while(true){
				if(kill_all == true){ 
					break;
				}
				else if(!is_running){
					sleep(1);
					continue;
				}

				int select_dummy;
				previous_chunk = 0;
				current_chunk = 0;
				total_chunks = -1;
				nTotalWords = 0;
				full_spill = true;

				if(!poll_server->Select(dummy)){
					if(!batch_mode){ term->SetStatus("\033[0;33m[IDLE]\033[0m Waiting for a spill..."); }
					else{ std::cout << "\r\033[0;33m[IDLE]\033[0m Waiting for a spill..."; }
					IdleTask();
					continue; 
				}
		
				if(!poll_server->Select(select_dummy)){ continue; } // Server timeout
		
				// Get the spill
				while(current_chunk != total_chunks){
					if(!poll_server->Select(select_dummy)){ // Server timeout
						std::cout << msgHeader << "Network timeout before recv full spill!\n";
						full_spill = false;
						break;
					} 

					nWords = poll_server->RecvMessage((char*)shm_data, maxShmSize) / 4; // Read from the socket
					if(strcmp((char*)shm_data, "$CLOSE_FILE") == 0 || strcmp((char*)shm_data, "$OPEN_FILE") == 0 || strcmp((char*)shm_data, "$KILL_SOCKET") == 0){ continue; } // Poll2 network flags
					// Did not read enough bytes
					else if(nWords < 2){
						continue;
					}

					if(debug_mode){ std::cout << "debug: Received " << nWords << " words from the network\n"; }
					memcpy((char *)&current_chunk, &shm_data[0], 4);
					memcpy((char *)&total_chunks, &shm_data[1], 4);

					if(previous_chunk == -1 && current_chunk != 1){ // Started reading in the middle of a spill, ignore the rest of it
						if(debug_mode){ std::cout << "debug: Skipping chunk " << current_chunk << " of " << total_chunks << std::endl; }
						continue;
					}
					else if(previous_chunk != current_chunk - 1){ // We missed a spill chunk somewhere
						if(debug_mode){ std::cout << "debug: Found chunk " << current_chunk << " but expected chunk " << previous_chunk+1 << std::endl; }
						break;
					}

					previous_chunk = current_chunk;
		
					// Copy the shm spill chunk into the data array
					if(nTotalWords + 2 + nWords <= 250000){ // This spill chunk will fit into the data buffer
						memcpy(&data[nTotalWords], &shm_data[2], (nWords - 2)*4);
						nTotalWords += (nWords - 2);				
					}
					else{ 
						if(debug_mode){ std::cout << "debug: Abnormally full spill buffer with " << nTotalWords + 2 + nWords << " words!\n"; }
						break; 
					}
				}

				std::stringstream status;
				status << "\033[0;32m" << "[RECV] " << "\033[0m" << nTotalWords << " words";
				if(!batch_mode){ term->SetStatus(status.str()); }
				else{ std::cout << "\r" << status.str(); }
		
				if(debug_mode){ std::cout << "debug: Retrieved spill of " << nTotalWords << " words (" << nTotalWords*4 << " bytes)\n"; }
				if(!dry_run_mode && full_spill){ 
					int word1 = 2, word2 = 9999;
					memcpy(&data[nTotalWords], (char *)&word1, 4);
					memcpy(&data[nTotalWords+1], (char *)&word2, 4);
					core->ReadSpill(data, nTotalWords + 2, is_verbose); 
				}
			
				if(!full_spill){ std::cout << msgHeader << "Not processing spill fragment!\n"; }
				else{ num_spills_recvd++; }
			}
		
			delete[] shm_data;
		}
		else if(file_format == 0){
			unsigned int *data = NULL;
			bool full_spill;
			bool bad_spill;
			unsigned int nBytes;
		
			if(!dry_run_mode){ data = new unsigned int[250000]; }
		
			// Reset the buffer reader to default values.
			databuff.Reset();
		
			while(true){ 
				if(kill_all == true){ 
					break;
				}
				else if(!is_running){
					sleep(1);
					continue;
				}

				if(!databuff.Read(&input_file, (char*)data, nBytes, 1000000, full_spill, bad_spill, dry_run_mode)){
					if(databuff.GetRetval() == 1){
						if(debug_mode){ std::cout << "debug: Encountered single EOF buffer (end of run).\n"; }
					}
					else if(databuff.GetRetval() == 2){
						if(debug_mode){ std::cout << "debug: Encountered double EOF buffer (end of file).\n"; }
						break;
					}
					else if(databuff.GetRetval() == 3){
						if(debug_mode){ std::cout << "debug: Encountered unknown ldf buffer type.\n"; }
					}
					else if(databuff.GetRetval() == 4){
						if(debug_mode){ std::cout << "debug: Encountered invalid spill chunk.\n"; }
					}
					else if(databuff.GetRetval() == 5){
						if(debug_mode){ std::cout << "debug: Received bad spill footer size.\n"; }
					}
					else if(databuff.GetRetval() == 6){
						if(debug_mode){ std::cout << "debug: Failed to read buffer from input file.\n"; }
						break;
					}
					continue;
				}

				std::stringstream status;			
				status << "\033[0;32m" << "[READ] " << "\033[0m" << nBytes/4 << " words (" << 100*input_file.tellg()/file_length << "%), ";
				status << "GOOD = " << databuff.GetNumChunks() << ", LOST = " << databuff.GetNumMissing();
				if(!batch_mode){ term->SetStatus(status.str()); }
				else{ std::cout << "\r" << status.str(); }
		
				if(full_spill){ 
					if(debug_mode){ 
						std::cout << "debug: Retrieved spill of " << nBytes << " bytes (" << nBytes/4 << " words)\n"; 
						std::cout << "debug: Read up to word number " << input_file.tellg()/4 << " in input file\n";
					}
					if(!dry_run_mode){ 
						if(!bad_spill){ 
							core->ReadSpill(data, nBytes/4, is_verbose); 
						}
						else{ std::cout << " WARNING: Spill has been flagged as corrupt, skipping (at word " << input_file.tellg()/4 << " in file)!\n"; }
					}
				}
				else if(debug_mode){ 
					std::cout << "debug: Retrieved spill fragment of " << nBytes << " bytes (" << nBytes/4 << " words)\n"; 
					std::cout << "debug: Read up to word number " << input_file.tellg()/4 << " in input file\n";
				}
				num_spills_recvd++;
			}

			if(!dry_run_mode){ delete[] data; }
		
			if(!batch_mode){ term->SetStatus("\033[0;33m[IDLE]\033[0m Finished scanning file."); }
			else{ std::cout << std::endl << std::endl; }
		}
		else if(file_format == 1){
			unsigned int *data = NULL;
			int nBytes;
		
			if(!dry_run_mode){ data = new unsigned int[max_spill_size+2]; }
		
			// Reset the buffer reader to default values.
			pldData.Reset();
		
			while(pldData.Read(&input_file, (char*)data, nBytes, 4*max_spill_size, dry_run_mode)){ 
				if(kill_all == true){ 
					break;
				}
				else if(!is_running){
					sleep(1);
					continue;
				}

				std::stringstream status;
				status << "\033[0;32m" << "[READ] " << "\033[0m" << nBytes/4 << " words (" << 100*input_file.tellg()/file_length << "%)";
				if(!batch_mode){ term->SetStatus(status.str()); }
				else{ std::cout << "\r" << status.str(); }
		
				if(debug_mode){ 
					std::cout << "debug: Retrieved spill of " << nBytes << " bytes (" << nBytes/4 << " words)\n"; 
					std::cout << "debug: Read up to word number " << input_file.tellg()/4 << " in input file\n";
				}
			
				if(!dry_run_mode){ 
					int word1 = 2, word2 = 9999;
					memcpy(&data[(nBytes/4)], (char *)&word1, 4);
					memcpy(&data[(nBytes/4)+1], (char *)&word2, 4);
					core->ReadSpill(data, nBytes/4 + 2, is_verbose); 
				}
				num_spills_recvd++;
			}

			if(eofbuff.ReadHeader(&input_file)){
				std::cout << msgHeader << "Encountered EOF buffer.\n";
			}
			else{
				std::cout << msgHeader << "Failed to find end of file buffer!\n";
			}
		
			if(!dry_run_mode){ delete[] data; }
		
			if(!batch_mode){ term->SetStatus("\033[0;33m[IDLE]\033[0m Finished scanning file."); }
			else{ std::cout << std::endl << std::endl; }
		}
		else if(file_format == 2){
		}

		// Notify that the scan has completed.
		Notify("SCAN_COMPLETE");
		
		total_stopped = true;
		stop_scan();
		
		if(batch_mode){ break; }
	}
	
	// Notify that run control is exiting.
	run_ctrl_exit = true;
}

/// Main command interpreter method.
void ScanInterface::CmdControl(){
	if(!core){ return; }

	std::string cmd = "", arg;
	bool waiting_for_run = false;

	while(true){
		if(run_ctrl_exit){ break; }

		if(waiting_for_run){
			if(!is_running){ waiting_for_run = false; }
			else{
				term->flush(); // Update the terminal so the user knows something is happening.
				sleep(1); // Sleep and wait for the run to finish.
				continue;
			}
		}
	
		cmd = term->GetCommand();
		if(cmd == "_SIGSEGV_"){
			std::cout << "\033[0;31m[SEGMENTATION FAULT]\033[0m\n";
			exit(EXIT_FAILURE);
		}
		else if(cmd == "CTRL_D"){ 
			std::cout << msgHeader << "Received EOF (ctrl-d) signal. Exiting...\n";
			cmd = "quit"; 
		}
		else if(cmd == "CTRL_C"){ 
			std::cout << msgHeader << "Warning! Received SIGINT (ctrl-c) signal.\n";
			continue; 
		}
		else if(cmd == "CTRL_Z"){ 
			std::cout << msgHeader << "Warning! Received SIGTSTP (ctrl-z) signal.\n";
			continue; 
		}
		term->flush();

		if(cmd == ""){ continue; }
		
		// Replace the '~' with the user's home directory.
		if(cmd.find('~') != std::string::npos)
			cmd.replace(cmd.find('~'), 1, homeDir);
		
		size_t index = cmd.find(' ');
		if(index != std::string::npos){
			arg = cmd.substr(index+1, cmd.size()-index); // Get the argument from the full input string
			cmd = cmd.substr(0, index); // Get the command from the full input string
		}
		else{ arg = ""; }

		std::vector<std::string> arguments;
		unsigned int p_args = split_str(arg, arguments);
		
		if(cmd == "quit" || cmd == "exit"){
			stop_scan();
			kill_all = true;
			while(!run_ctrl_exit){ sleep(1); }
			break;
		}
		else if(cmd == "version" || cmd == "v"){
			std::cout << "  " << PROG_NAME << "      v" << SCAN_VERSION << " (" << SCAN_DATE << ")\n";
			std::cout << "  Poll2 Socket  v" << POLL2_SOCKET_VERSION << " (" << POLL2_SOCKET_DATE << ")\n"; 
			std::cout << "  HRIBF Buffers v" << HRIBF_BUFFERS_VERSION << " (" << HRIBF_BUFFERS_DATE << ")\n"; 
			std::cout << "  CTerminal     v" << CTERMINAL_VERSION << " (" << CTERMINAL_DATE << ")\n";
		}
		else if(cmd == "help" || cmd == "h"){
			std::cout << "  Help:\n";
			std::cout << "   debug           - Toggle debug mode flag (default=false)\n";
			std::cout << "   quiet           - Toggle quiet mode flag (default=false)\n";
			std::cout << "   quit            - Close the program\n";
			std::cout << "   help (h)        - Display this dialogue\n";
			std::cout << "   version (v)     - Display Poll2 version information\n";
			std::cout << "   run             - Start acquisition\n";
			std::cout << "   stop            - Stop acquisition\n";
			std::cout << "   file <filename> - Load an input file\n";
			std::cout << "   rewind [offset] - Rewind to the beginning of the file\n";
			std::cout << "   sync            - Wait for the current run to finish\n";
			CmdHelp("   ");
		}
		else if(cmd == "run"){ // Start acquisition.
			start_scan();
		}
		else if(cmd == "stop"){ // Stop acquisition.
			stop_scan();
		}
		else if(cmd == "debug"){ // Toggle debug mode
			if(debug_mode){
				std::cout << msgHeader << "Toggling debug mode OFF\n";
				debug_mode = false;
			}
			else{
				std::cout << msgHeader << "Toggling debug mode ON\n";
				debug_mode = true;
			}
			core->SetDebugMode(debug_mode);
		}
		else if(cmd == "quiet"){ // Toggle quiet mode
			if(!is_verbose){
				std::cout << msgHeader << "Toggling quiet mode OFF\n";
				is_verbose = true;
			}
			else{
				std::cout << msgHeader << "Toggling quiet mode ON\n";
				is_verbose = false;
			}
		}
		else if(cmd == "file"){ // Rewind the file to the start position
			if(p_args > 0){
				if(!open_input_file(arguments.at(0))){
					std::cout << msgHeader << "Failed to open input file!\n";
				}
			}
			else{
				std::cout << msgHeader << "Invalid number of parameters to 'file'\n";
				std::cout << msgHeader << " -SYNTAX- file <filename>\n";
			}
		}
		else if(cmd == "rewind"){ // Rewind the file to the start position
			if(p_args > 0){ rewind(strtoul(arguments.at(0).c_str(), NULL, 0)); }
			else{ rewind(); }
		}
		else if(cmd == "sync"){ // Wait until the current run is completed.
			if(is_running){
				std::cout << msgHeader << "Waiting for current scan to complete.\n";
				waiting_for_run = true;
			}
			else{ std::cout << msgHeader << "Scan is not running.\n"; }
		}
		else if(!ExtraCommands(cmd, arguments)){ // Unrecognized command. Send it to a derived object.
			std::cout << msgHeader << "Unknown command '" << cmd << "'\n";
		}
	}		
}

/** Setup user options and initialize all required objects.
  * \param[in]  argc Number of arguments passed from the command line.
  * \param[in]  argv Array of strings passed as arguments from the command line.
  * \return True upon success and false otherwise.
  */
bool ScanInterface::Setup(int argc, char *argv[]){
	if(scan_init){ return false; }

	// If a pointer to an Unpacker derived class is not specified, call the
	// extern function GetCore() to get a pointer to a new object.
	if(!core){ GetCore(); }

	// Link this object to the Unpacker for cross-calls.
	core->SetInterface(this);

	if(argc >= 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)){ // A stupid way to do this... for now.
		help(argv[0]);
		return false;
	}
	else if(argc >= 2 && (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0)){ // Display version information
		std::cout << "  " << PROG_NAME << "      v" << SCAN_VERSION << " (" << SCAN_DATE << ")\n";
		std::cout << "  Poll2 Socket  v" << POLL2_SOCKET_VERSION << " (" << POLL2_SOCKET_DATE << ")\n"; 
		std::cout << "  HRIBF Buffers v" << HRIBF_BUFFERS_VERSION << " (" << HRIBF_BUFFERS_DATE << ")\n"; 
		std::cout << "  CTerminal     v" << CTERMINAL_VERSION << " (" << CTERMINAL_DATE << ")\n";
		return false;
	}
	
	debug_mode = false;
	dry_run_mode = false;
	shm_mode = false;
	
	num_spills_recvd = 0;

	// Fill the argument list.
	std::deque<std::string> scan_args;
	std::deque<std::string> optargs;
	int arg_index = 1;
	while(arg_index < argc){
		scan_args.push_back(std::string(argv[arg_index++]));
	}

	// Loop through the arg list and extract ScanInterface arguments.
	std::string current_arg;
	std::string input_filename = "";
	while(!scan_args.empty()){
		current_arg = scan_args.front();
		scan_args.pop_front();
		if(current_arg == "--debug"){ 
			core->SetDebugMode();
			debug_mode = true;
		}
		else if(current_arg == "--counts"){
			write_counts = true;
		}
		else if(current_arg == "--batch"){
			batch_mode = true;
		}
		else if(current_arg == "--dry-run"){
			dry_run_mode = true;
		}
		else if(current_arg == "--fast-fwd"){
			if(scan_args.empty()){
				std::cout << " FATAL ERROR! Missing required argument to option '--fast-fwd'!\n";
				help(argv[0]);
				return false;
			}
			file_start_offset = atoll(scan_args.front().c_str());
			scan_args.pop_front();
		}
		else if(current_arg == "--quiet"){
			is_verbose = false;
		}
		else if(current_arg == "--shm"){ 
			file_format = 0;
			shm_mode = true;
			std::cout << " Using shm mode!\n";
		}
		else if(!ExtraArguments(current_arg, scan_args, input_filename)){ // Unrecognized option.
			std::cout << " Error: Unrecognized command line argument '" << current_arg << "'.\n";
			help(argv[0]);
			return false;
		}
	}

	// Initialize everything.
	std::cout << msgHeader << "Initializing derived class.\n";
	if(!Initialize(msgHeader)){ // Failed to initialize the object. Clean up and exit.
		std::cout << " FATAL ERROR! Failed to initialize derived class!\n";
		std::cout << "\nCleaning up...\n";
		if(write_counts)
			core->Write();
		return false;
	}

	if(shm_mode){
		poll_server = new Server();
		if(!poll_server->Init(5555, 1)){
			std::cout << " FATAL ERROR! Failed to open shm socket 5555!\n";
			std::cout << "\nCleaning up...\n";
			if(write_counts)
				core->Write();
			return false;
		}	
		if(batch_mode){
			std::cout << msgHeader << "Unable to enable batch mode for shared-memory mode!\n";
			batch_mode = false;
		}
	}

	// Initialize the terminal.
	if(!batch_mode){
		term = new Terminal();
		term->Initialize();
		term->SetCommandHistory(("."+progName+".cmd").c_str());
		term->SetPrompt((progName+" $ ").c_str());
		term->AddStatusWindow();
		term->SetStatus("\033[0;31m[STOP]\033[0m Acquisition stopped.");
	}

	std::cout << "\n " << PROG_NAME << " v" << SCAN_VERSION << "\n"; 
	std::cout << " ==  ==  ==  ==  == \n\n"; 

	if(debug_mode){ std::cout << msgHeader << "Using debug mode.\n\n"; }
	if(dry_run_mode){ std::cout << msgHeader << "Doing a dry run.\n\n"; }
	if(shm_mode){ 
		std::cout << msgHeader << "Using shared-memory mode.\n\n"; 
		std::cout << msgHeader << "Listening on poll2 SHM port 5555\n\n";
	}

	// Do any last minute initialization.
	try{ FinalInitialization(); }
	catch(...){ std::cout << "\nFinal initialization failed!\n"; }

	scan_init = true;

	// Load the input file, if the user has supplied a filename.
	if(!shm_mode && !input_filename.empty()){
		std::cout << msgHeader << "Using filename " << input_filename << ".\n";
		if(open_input_file(input_filename)){
			// Start the scan.
			start_scan();
		}
		else{ std::cout << msgHeader << "Failed to load input file!\n"; }
	}
	
	return true;
}

/** Run the scan program.
  * This should be called from main().
  * \return Integer return value. 0 on success and 1 otherwise.
  */
int ScanInterface::Execute(){
	if(!scan_init){
		std::cout << " FATAL ERROR! ScanInterface is not initialized!\n";
		return 1; 
	}

	// Seek to the beginning of the file.
	if(file_start_offset != 0){ rewind(); }

	// Process the file.
	if(!batch_mode){
		// Start the run control thread
		std::cout << "\n Starting data control thread\n";
		std::thread runctrl(start_run_control, this);

		// Start the command control thread. This needs to be the last thing we do to
		// initialize, so the user cannot enter commands before setup is complete
		std::cout << " Starting command thread\n\n";
		std::thread comctrl(start_cmd_control, this);

		// Synchronize the threads and wait for completion
		comctrl.join();
		runctrl.join();
	}
	else{ start_run_control(this); }
	
	return 0;
}

/** Shutdown cleanly. Uninitialize the ScanInterface object.
  * \return True upon success and false if ScanInterface has not been initialized.
  */
bool ScanInterface::Close(){
	if(!scan_init){ return false; }

	// Close the socket and restore the terminal
	if(!batch_mode){
		term->Close();
	}
	
	// Only close the server if this is shared memory mode. Otherwise
	// the server would never have been initialized.
	if(shm_mode){ poll_server->Close(); }
	
	//Reprint the leader as the carriage was returned
	std::cout << "Running " << PROG_NAME << " v" << SCAN_VERSION << " (" << SCAN_DATE << ")\n";
	
	std::cout << msgHeader << "Retrieved " << num_spills_recvd << " spills!\n";

	if(input_file.good()){
		input_file.close();	
	}

	// Clean up detector driver
	std::cout << "\n" << msgHeader << "Cleaning up...\n";
	
	// Show the number of lost spill chunks.
	std::cout << msgHeader << "Read " << databuff.GetNumChunks() << " spill chunks.\n";
	std::cout << msgHeader << "Lost at least " << databuff.GetNumMissing() << " spill chunks.\n";
	
	if(write_counts)
		core->Write();
	
	if(poll_server){ delete poll_server; }
	if(term){ delete term; }
	if(core){ delete core; }
	
	scan_init = false;
	return true;
}
