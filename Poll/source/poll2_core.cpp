/** \file poll2_core.cpp
  * 
  * \brief Controls the poll2 command interpreter and data acquisition system
  * 
  * The Poll class is used to control the command interpreter
  * and data acqusition systems. Command input and the command
  * line interface of poll2 are handled by the external library
  * CTerminal. Pixie16 data acquisition is handled by interfacing
  * with the PixieInterface library.
  *
  * \author Cory R. Thornsberry
  * 
  * \date Oct. 6th, 2015
  * 
  * \version 1.3.10
*/

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <ctime>

#include <cmath>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "poll2_core.h"
#include "poll2_socket.h"
#include "poll2_stats.h"

#include "CTerminal.h"

// Interface for the PIXIE-16
#include "PixieSupport.h"
#include "Utility.h"
#include "Display.h"

#include "MCA_ROOT.h"
#include "MCA_DAMM.h"

// Values associated with the minimum timing between pixie calls (in us)
// Adjusted to help alleviate the issue with data corruption
#define POLL_TRIES 100

// 4 GB. Maximum allowable .ldf file size in bytes
#define MAX_FILE_SIZE 4294967296ll

// Length of shm packet header (in bytes)
#define PKT_HEAD_LEN 8

// Maximum shm packet size (in bytes)
#define MAX_PKT_DATA (MAX_ORPH_DATA - PKT_HEAD_LEN)

std::vector<std::string> chan_params = {"TRIGGER_RISETIME", "TRIGGER_FLATTOP", "TRIGGER_THRESHOLD", "ENERGY_RISETIME", "ENERGY_FLATTOP", "TAU", "TRACE_LENGTH",
									 "TRACE_DELAY", "VOFFSET", "XDT", "BASELINE_PERCENT", "EMIN", "BINFACTOR", "CHANNEL_CSRA", "CHANNEL_CSRB", "BLCUT",
									 "ExternDelayLen", "ExtTrigStretch", "ChanTrigStretch", "FtrigoutDelay", "FASTTRIGBACKLEN"};

std::vector<std::string> mod_params = {"MODULE_CSRA", "MODULE_CSRB", "MODULE_FORMAT", "MAX_EVENTS", "SYNCH_WAIT", "IN_SYNCH", "SLOW_FILTER_RANGE",
									"FAST_FILTER_RANGE", "MODULE_NUMBER", "TrigConfig0", "TrigConfig1", "TrigConfig2","TrigConfig3"};

const std::vector<std::string> Poll::runControlCommands_ ({"run", "stop", 
	"startacq", "startvme", "stopacq", "stopvme", "acq", "shm", "spill", "hup", 
	"prefix", "fdir", "title", "runnum", "oform", "close", "reboot", "stats", 
	"mca"});
const std::vector<std::string> Poll::paramControlCommands_ ({"dump", "pread", 
	"pmread", "pwrite", "pmwrite", "adjust_offsets", "find_tau", "toggle", 
	"toggle_bit", "csr_test", "bit_test", "get_traces"});
const std::vector<std::string> Poll::pollStatusCommands_ ({"status", "thresh", 
	"debug", "quiet",	"quit", "help", "version"});

MCA_args::MCA_args(){ 
	mca = NULL;
	Zero(); 
}
	
MCA_args::MCA_args(bool useRoot_, int totalTime_, std::string basename_){
	running = false;
	useRoot = useRoot_;
	totalTime = totalTime_;
	basename = basename_;
	
	mca = NULL;
}

MCA_args::~MCA_args(){
	if(mca){ delete mca; }
}

bool MCA_args::Initialize(PixieInterface *pif_){
	if(running){ return false; }

	pif_->RemovePresetRunLength(0);
	
	// Initialize the MCA object.
#if defined(USE_ROOT) && defined(USE_DAMM)
	if(useRoot)
		mca = (MCA*)(new MCA_ROOT(pif_, basename.c_str()));
	else
		mca = (MCA*)(new MCA_DAMM(pif_, basename.c_str()));
#elif defined(USE_ROOT)
	mca = (MCA*)(new MCA_ROOT(pif_, basename.c_str()));
#elif defined(USE_DAMM)
	mca = (MCA*)(new MCA_DAMM(pif_, basename.c_str()));
#endif

	pif_->StartHistogramRun();

	if(!mca->IsOpen()){
		pif_->EndRun();
		return false;
	}

	return (running = true);
}

bool MCA_args::Step(){ 
	if(!mca){ return false; }
	return mca->Step();
}

bool MCA_args::CheckTime(){ 
	if(!mca || (totalTime <= 0 || (mca->GetRunTime() >= totalTime))){ return false; }
	return true;
}

void MCA_args::Zero(){
	running = false;
	useRoot = false;
	totalTime = 0; // Needs to be zero for checking of MCA arguments
	basename = "MCA";
	
	if(mca){ 
		delete mca; 
		mca = NULL;
	}
}

void MCA_args::Close(PixieInterface *pif_){
	pif_->EndRun();
	Zero();
}

Poll::Poll() : 
	// System flags and variables
	sys_message_head(" POLL: "),
	kill_all(false), // Set to true when the program is exiting
	do_start_acq(false), // Set to true when the command is given to start a run
	do_stop_acq(false), // Set to true when the command is given to stop a run
	record_data(false), // Set to true if data is to be recorded to disk
	do_reboot(false), // Set to true when the user tells POLL to reboot PIXIE
	force_spill(false), // Force poll2 to dump the current data spill
	acq_running(false), // Set to true when run_command is recieving data from PIXIE
	run_ctrl_exit(false), // Set to true when run_command exits
	had_error(false), //Set to true when aborting due to an error.
	file_open(false), //Set to true when a file is opened.
	raw_time(0),
	do_MCA_run(false), // Set to true when the "mca" command is received
	// Run control variables
	boot_fast(false),
	insert_wall_clock(true),
	is_quiet(false),
	send_alarm(false),
	show_module_rates(false),
	zero_clocks(false),
	debug_mode(false),
	shm_mode(false),
	pac_mode(false),
	init(false),
	// Options relating to output data file
	output_directory("./"), // Set with 'fdir' command
	filename_prefix("run"),
	output_title("PIXIE data file"), // Set with 'title' command
	next_run_num(1), // Set with 'runnum' command
	output_format(0), // Set with 'oform' command
	current_file_num(0),
	// Some pacman stuff
	udp_sequence(0),
	total_spill_chunks(0)
{
	pif = new PixieInterface("pixie.cfg");
	
	// Check the scheduler (kernel priority)
	Display::LeaderPrint("Checking scheduler");
	int startScheduler = sched_getscheduler(0);
	if(startScheduler == SCHED_BATCH){ 
		std::cout << Display::InfoStr("SCHED_BATCH") << std::endl; 
	}
	else if(startScheduler == SCHED_OTHER){ 
		std::cout << Display::InfoStr("STANDARD (SCHED_OTHER)") << std::endl; 
	}
	else{ std::cout << Display::WarningStr("UNEXPECTED") << std::endl; }

	client = new Client();

}

Poll::~Poll(){
	if(init){
		Close();
	}

	delete pif;
}

bool Poll::Initialize(){
	if(init){ return false; }

	// Set debug mode
	if(debug_mode){ 
		std::cout << sys_message_head << "Setting debug mode\n";
		output_file.SetDebugMode(); 
	}

	// Initialize the pixie interface and boot
	pif->GetSlots();
	if(!pif->Init()){ return false; }

	//Boot the modules.
	if(boot_fast){
		if(!pif->Boot(PixieInterface::DownloadParameters | PixieInterface::SetDAC | PixieInterface::ProgramFPGA)){ return false; } 
	}
	else{
		if(!pif->Boot(PixieInterface::BootAll)){ return false; }
	}
	
	//Set module syncronization parameters
	if(!synch_mods()){ return false; }

	// Allocate memory buffers for FIFO
	n_cards = pif->GetNumberCards();
	
	if(pac_mode){ 
		//Initialize pacman data port
		client->Init("127.0.0.1", 45080);
		
		//Initialize pacman command port
		server = new Server();
		server->Init(45086, 1); // Set the timeout to 1 second. 
	}
	else{ 
		//Initialize Cory's shm port
		// This port number is used to avoid tying up udptoipc's port
		client->Init("127.0.0.1", 5555);
	}

	//Allocate an array of vectors to store partial events from the FIFO.
	partialEvents = new std::vector<word_t>[n_cards];

	//Create a stats handler and set the interval.
	statsHandler = new StatsHandler(n_cards);
	statsHandler->SetDumpInterval(statsInterval_);
	
	//Build the list of commands
	commands_.insert(commands_.begin(), pollStatusCommands_.begin(), pollStatusCommands_.end());
	commands_.insert(commands_.begin(), paramControlCommands_.begin(), paramControlCommands_.end());
	//Omit the run control commands.
	if (!pac_mode) {
		commands_.insert(commands_.begin(), runControlCommands_.begin(), runControlCommands_.end());
	}

	return init = true;
}

/**Clean up things that are created during Poll::Initialize().
 *
 * \return Returns true if successful.
 */
bool Poll::Close(){
	//We return if the class has not been initialized.
	if(!init){ return false; }
	
	//Send message to Cory's SHM that we are closing.
	if(!pac_mode){ client->SendMessage((char *)"$KILL_SOCKET", 13); }
	//Close the pacman command port.
	else{ server->Close(); }
	//Close the UDP data / SHM port.
	client->Close();
	
	// Just to be safe
	CloseOutputFile();

	//Delete the array of partial event vectors.
	delete[] partialEvents;
	partialEvents = NULL;

	delete statsHandler;
	statsHandler = NULL;

	// We are no longer initialized.
	init = false;
	
	return true;
}

/** Safely close current data file if one is open. The scalers are cleared when this
 * is called.
 *
 * \param[in] continueRun Flag indicating whether we are continuing the same run,
 *  but opening a new continuation file.
 *	\return True if the file was closed successfully.
 */
bool Poll::CloseOutputFile(const bool continueRun /*=false*/){
	//No file was open.
	if(!output_file.IsOpen()){ 
		std::cout << sys_message_head << "No file is open.\n";
		file_open = false;
		return true;
	}

	//Clear the stats
	if (!continueRun){
		statsHandler->Clear();
		statsHandler->Dump();
	}

	std::cout << sys_message_head << "Closing output file.\n";

	//Broadcast to Cory's SHM that the file is now closed.
	if(!pac_mode){ client->SendMessage((char *)"$CLOSE_FILE", 12); }

	output_file.CloseFile();

	//We call get next file name to update the run number.
	if (!continueRun) {
		output_file.GetNextFileName(next_run_num,filename_prefix,output_directory);
	}

	//Set the falg that no file is open.
	file_open = false;

	return true;
}

/**Opens a new file if no file is currently open. The new file is 
 * determined from the output directory, run number and prefix. The run 
 * number may be iterated forward if a file already exists. 
 * If this is a continuation run the run number is not iterated and 
 * instead a suffix number is incremented.
 *
 * The scalers are cleared when this is called if a file is not open already.
 *
 * \param [in] continueRun Flag indicating that this file should be a 
 *  continuation run and that the run number should not be iterated.
 *  \return True if successfully opened a new file.
 */
bool Poll::OpenOutputFile(bool continueRun){
	//A file was already open
	if(output_file.IsOpen()){ 
		std::cout << Display::WarningStr() << ": A file is already open. Close the current file before opening a new one.\n"; 
		return false;
	}

	//Try to open a file and check if unsuccessful.
	if(!output_file.OpenNewFile(output_title, next_run_num, filename_prefix, output_directory, continueRun)){
		//Unsuccessful when opening file print a message.
		std::cout << Display::ErrorStr() << ": Failed to open output file! Check that the path is correct.\n";
	
		//Set the error flag and disable data recording.
		had_error = true;
		record_data = false;

		return false;
	}

	//Clear the stats
	statsHandler->Clear();
	statsHandler->Dump();

	std::cout << sys_message_head << "Opening output file '" << output_file.GetCurrentFilename() << "'.\n";
	//When using Cory's SHM send a message that the file is open.
	if(!pac_mode){ client->SendMessage((char *)"$OPEN_FILE", 12); }

	file_open = true;

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
	// Open an output file if needed
	if(!output_file.IsOpen()){
		std::cout << Display::ErrorStr() << " Recording data, but no file is open! Opening a new file.\n";
		OpenOutputFile();
	}

	// Handle the writing of buffers to the file
	std::streampos current_filesize = output_file.GetFilesize();
	if(current_filesize + (std::streampos)(4*nWords + 65552) > MAX_FILE_SIZE){
		// Adding nWords plus 2 EOF buffers to the file will push it over MAX_FILE_SIZE.
		// Open a new output file instead
		std::cout << sys_message_head << "Current filesize is " << current_filesize + (std::streampos)65552 << " bytes.\n";
		std::cout << sys_message_head << "Opening new file.\n";
		CloseOutputFile(true);
		OpenOutputFile(true);
	}

	if (!is_quiet) std::cout << "Writing " << nWords << " words.\n";

	return output_file.Write((char*)data, nWords);
}

void Poll::broadcast_data(word_t *data, unsigned int nWords) {
	// Maximum size of the shared memory buffer
	static const unsigned int maxShmSizeL = 4050; // in pixie words
	static const unsigned int maxShmSize  = maxShmSizeL * sizeof(word_t); // in bytes

	if(pac_mode){ // Broadcast the spill onto the network using the classic pacman shm style
		unsigned int nBufs = nWords / maxShmSizeL;
		unsigned int wordsLeft = nWords % maxShmSizeL;

		unsigned int totalBufs = nBufs + 1 + ((wordsLeft != 0) ? 1 : 0);

		if (debug_mode) std::cout << "Broadcasting " << totalBufs << " pacman buffers.\n";

		word_t *pWrite = (word_t *)AcqBuf.Data;
		
		// Chop the data and send it through the network
		for(size_t buf=0; buf < nBufs; buf++){
			// Get a long sized pointer to the data block for writing
			// put a header on each shared memory buffer
			AcqBuf.BufLen = (maxShmSizeL + 3) * sizeof(word_t);
			pWrite[0] = AcqBuf.BufLen; // size
			pWrite[1] = totalBufs; // number of buffers we expect
			pWrite[2] = buf;
			memcpy(&pWrite[3], &data[buf * maxShmSizeL], maxShmSize);

			broadcast_pac_data();
			usleep(1);
		}

		// Send the last fragment (if there is any)
		if (wordsLeft != 0) {
			AcqBuf.BufLen = (wordsLeft + 3) * sizeof(word_t);
			pWrite[0] = AcqBuf.BufLen;
			pWrite[1] = totalBufs;
			pWrite[2] = nBufs;
			memcpy(&pWrite[3], &data[nBufs * maxShmSizeL], 
			wordsLeft * sizeof(word_t) );

			broadcast_pac_data();
			usleep(1);
		}

		// Send a buffer to say that we are done
		AcqBuf.BufLen = 5 * sizeof(word_t);
		pWrite[0] = AcqBuf.BufLen;
		pWrite[1] = totalBufs;
		pWrite[2] = totalBufs - 1;
		
		// Pacman looks for the following data
		pWrite[3] = 0x2; 
		pWrite[4] = 0x270f; // vsn 9999

		broadcast_pac_data();  
	}
	else if(shm_mode){ // Broadcast the spill onto the network using the new shm style
		int shm_data[maxShmSizeL+2]; // packets of data
		unsigned int num_net_chunks = nWords / maxShmSizeL;
		unsigned int num_net_remain = nWords % maxShmSizeL;
		if(num_net_remain != 0){ num_net_chunks++; }
		
		unsigned int net_chunk = 1;
		unsigned int words_bcast = 0;
		if(debug_mode){ std::cout << " debug: Splitting " << nWords << " words into network spill of " << num_net_chunks << " chunks (fragment = " << num_net_remain << " words)\n"; }
		
		while(words_bcast < nWords){
			if(nWords - words_bcast > maxShmSizeL){ // Broadcast the spill chunks
				memcpy(&shm_data[0], &net_chunk, 4);
				memcpy(&shm_data[1], &num_net_chunks, 4);
				memcpy(&shm_data[2], &data[words_bcast], maxShmSize);
				client->SendMessage((char *)shm_data, maxShmSize+8);
				words_bcast += maxShmSizeL;
			}
			else{ // Broadcast the spill remainder
				memcpy(&shm_data[0], &net_chunk, 4);
				memcpy(&shm_data[1], &num_net_chunks, 4);
				memcpy(&shm_data[2], &data[words_bcast], (nWords-words_bcast)*4);
				client->SendMessage((char *)shm_data, (nWords - words_bcast + 2)*4);
				words_bcast += nWords-words_bcast;
			}
			usleep(1);
			net_chunk++;
		}
	}
	else{ // Broadcast a spill notification to the network
		output_file.SendPacket(client);
	}
}

void Poll::broadcast_pac_data(){
	const int BufEnd=0xFFFFFFFF;
	short int status, cont_pkt;
	char *bufptr;
	int size,bytes;

	size = AcqBuf.BufLen;
	memcpy(AcqBuf.Data+size, &BufEnd, 4);
	size += 4;
	if(size % 2 != 0){ 
		memcpy(AcqBuf.Data+size,&BufEnd,2);
		size+=2;
	}

	if (debug_mode) std::cout << "PAC broadcast size " << size << "/" << MAX_PKT_DATA << " B.\n";

	if(size <= MAX_PKT_DATA){ 
		AcqBuf.Sequence = udp_sequence++;
		AcqBuf.DataSize = size + 8;    
		AcqBuf.TotalEvents = ++total_spill_chunks;
		AcqBuf.Events = 1;
		AcqBuf.Cont=0; 

		status = client->SendMessage((char *)&AcqBuf, size + PKT_HEAD_LEN + 8);
		if(status < 0){ std::cout << Display::WarningStr("[WARNING]") << ": Error sending UDP message to pacman."; }

		return;
	}

	cont_pkt = 1;
	bufptr =(char *)&AcqBuf;
	total_spill_chunks++;
	//This loops through the large data packet and writes the header info into 
	// data_pack::Data for each packet as it goes along. It does not follow the structure.
	do {
		if(size > MAX_PKT_DATA){ 
			bytes = MAX_PKT_DATA;
			*((unsigned short *)(bufptr+PKT_HEAD_LEN+4)) = 0; //Events
		} 
		else{
			bytes = size;
			*((unsigned short *)(bufptr+PKT_HEAD_LEN+4)) = 1; //Events
		}
		*((int *)(bufptr+PKT_HEAD_LEN)) = total_spill_chunks; //TotalEvents 
		*((unsigned short *)(bufptr+PKT_HEAD_LEN+6)) = cont_pkt; //Cont
		*((unsigned int *)(bufptr)) = udp_sequence++; //Sequence
		*((int *)(bufptr+4)) = bytes+8; //DataSize

		status = client->SendMessage((char *)bufptr, bytes + PKT_HEAD_LEN + 8);
		if(status < 0){ std::cout << Display::WarningStr("[WARNING]") << ": Error sending UDP message to pacman."; }

		cont_pkt++;
		bufptr += bytes;
		size = size - bytes;
	} while (size > 0);
}

/* Print help dialogue for POLL options. */
void Poll::help(){
	std::cout << "  Help:\n";
	if(!pac_mode){
		std::cout << "   run                 - Start data acquisition and start recording data to disk\n";
		std::cout << "   stop                - Stop data acqusition and stop recording data to disk\n";	
		std::cout << "   startacq (startvme) - Start data acquisition\n";
		std::cout << "   stopacq (stopvme)   - Stop data acquisition\n";
		std::cout << "   acq (shm)           - Run in \"shared-memory\" mode\n";
		std::cout << "   spill (hup)         - Force dump of current spill\n";
		std::cout << "   prefix [name]       - Set the output filename prefix (default='run_#.ldf')\n";
		std::cout << "   fdir [path]         - Set the output file directory (default='./')\n";
		std::cout << "   title [runTitle]    - Set the title of the current run (default='PIXIE Data File)\n";
		std::cout << "   runnum [number]     - Set the number of the current run (default=0)\n";
		std::cout << "   oform [0|1|2]       - Set the format of the output file (default=0)\n";
		std::cout << "   close (clo)         - Safely close the current data output file\n";
		std::cout << "   reboot              - Reboot PIXIE crate\n";
		std::cout << "   stats [time]        - Set the time delay between statistics dumps (default=-1)\n";
		std::cout << "   mca [root|damm] [time] [filename]     - Use MCA to record data for debugging purposes\n";
	}
	std::cout << "   dump [filename]                       - Dump pixie settings to file (default='Fallback.set')\n";
	std::cout << "   pread [mod] [chan] [param]            - Read parameters from individual PIXIE channels\n";
	std::cout << "   pmread [mod] [param]                  - Read parameters from PIXIE modules\n";
	std::cout << "   pwrite [mod] [chan] [param] [val]     - Write parameters to individual PIXIE channels\n";
	std::cout << "   pmwrite [mod] [param] [val]           - Write parameters to PIXIE modules\n";
	std::cout << "   adjust_offsets [module]               - Adjusts the baselines of a pixie module\n";
	std::cout << "   find_tau [module] [channel]           - Finds the decay constant for an active pixie channel\n";
	std::cout << "   toggle [module] [channel] [bit]       - Toggle any of the 19 CHANNEL_CSRA bits for a pixie channel\n";
	std::cout << "   toggle_bit [mod] [chan] [param] [bit] - Toggle any bit of any parameter of 32 bits or less\n";
	std::cout << "   csr_test [number]                     - Output the CSRA parameters for a given integer\n";
	std::cout << "   bit_test [num_bits] [number]          - Display active bits in a given integer up to 32 bits long\n";
	std::cout << "   get_traces [mod] [chan] <threshold>   - Get traces for all channels in a specified module\n";
	std::cout << "   status              - Display system status information\n";
	std::cout << "   thresh [threshold]  - Modify or display the current polling threshold.\n";
	std::cout << "   debug               - Toggle debug mode flag (default=false)\n";
	std::cout << "   quiet               - Toggle quiet mode flag (default=false)\n";
	std::cout << "   quit                - Close the program\n";
	std::cout << "   help (h)            - Display this dialogue\n";
	std::cout << "   version (v)         - Display Poll2 version information\n";
}

std::vector<std::string> Poll::TabComplete(std::string cmd) {
	std::vector<std::string> matches;
	std::vector<std::string>::iterator it;
	
	//If we have no space then we are auto completing a command.	
	if (cmd.find(" ") == std::string::npos) {
		for (it=commands_.begin(); it!=commands_.end();++it) {
			if ((*it).find(cmd) == 0) {
				matches.push_back((*it).substr(cmd.length()));
			}
		}
	}
	else {
		//Get the trailing str part to complete
		std::string strToComplete = cmd.substr(cmd.find_last_of(" ")+1);

		//If the inital command is pwrite or pread we try to auto complete the param names
		if (cmd.find("pwrite") == 0 || cmd.find("pread") == 0) {
			for (auto it=chan_params.begin(); it!=chan_params.end();++it) {
				if ((*it).find(strToComplete) == 0) 
					matches.push_back((*it).substr(strToComplete.length()));
			}
		}

		//If the inital command is pmwrite or pmread we try to auto complete the param names
		if (cmd.find("pmwrite") == 0 || cmd.find("pmread") == 0) {
			for (auto it=mod_params.begin(); it!=mod_params.end();++it) {
				if ((*it).find(strToComplete) == 0) 
					matches.push_back((*it).substr(strToComplete.length()));
			}
		}

	}

	return matches; 
}

/* Print help dialogue for reading/writing pixie channel parameters. */
void Poll::pchan_help(){
	std::cout << "  Valid Pixie16 channel parameters:\n";
	for(unsigned int i = 0; i < chan_params.size(); i++){
		std::cout << "   " << chan_params[i] << "\n";
	}
}

/* Print help dialogue for reading/writing pixie module parameters. */
void Poll::pmod_help(){
	std::cout << "  Valid Pixie16 module parameters:\n";
	for(unsigned int i = 0; i < mod_params.size(); i++){
		std::cout << "   " << mod_params[i] << "\n";
	}
}

/**Starts a data recording run. Open data file is closed, the run number is iterated and a new file is opened.
 * If the file was successfully opened the acquisition is started.
 * If a run is already started a warning is displayed and the process is stopped.
 *
 * \return Returns true if successfully starts a run.
 */
bool Poll::start_run() {
	if(do_MCA_run){
		std::cout << sys_message_head << "Warning! Cannot run acquisition while MCA program is running\n";
		return false;
	}
	else if(acq_running){ 
		std::cout << sys_message_head << "Acquisition is already running\n";
		return false;
	}

	//Close a file if open
	if(output_file.IsOpen()){ CloseOutputFile();	}

	//Preapre the output file
	if (!OpenOutputFile()) return false;
	record_data = true;

	//Start the acquistion
	start_acq();

	return true;
}

/**Current run is stopped. This includes disabling data recording.
 * This command stops the acquisition even if data recording is not active.
 *
 * \return Returns true if successful.
 */
bool Poll::stop_run() {
	if(!acq_running && !do_MCA_run){ 
		std::cout << sys_message_head << "Acquisition is not running\n"; 
		return false;
	}

	stop_acq();
	
	if (record_data) {
		std::stringstream output;
		output << "Run " << output_file.GetRunNumber() << " time";
		Display::LeaderPrint(output.str());
		std::cout << statsHandler->GetTotalTime() << "s\n";
	}

	record_data = false;
	return true;
}

/**Starts data acquistion. The process then waits until the acquistion is running.
 *	
 *	\return Returns true if successful.
 */
bool Poll::start_acq() {
	if(do_MCA_run){ 
		std::cout << sys_message_head << "Warning! Cannot run acquisition while MCA program is running\n"; 
		return false;
	}
	else if (acq_running) { 
		std::cout << sys_message_head << "Acquisition is already running\n"; 
		return false;
	}

	//Set start acq flag to be intercepted by run control.
	do_start_acq = true;

	return true;
}

/**Stops data acquistion. The process waits until the acquistion is stopped.
 * 
 * \return Returns true if succesful.
 */
bool Poll::stop_acq() {
	if(!acq_running && !do_MCA_run){ 
		std::cout << sys_message_head << "Acquisition is not running\n"; 
		return false;
	}

	//Set stop_acq flag to be intercepted by run control.
	do_stop_acq = true;

	return true;
}

void Poll::show_status(){
	std::cout << "  Poll Run Status:\n";
	std::cout << "   Acq starting    - " << yesno(do_start_acq) << std::endl;
	std::cout << "   Acq stopping    - " << yesno(do_stop_acq) << std::endl;
	std::cout << "   Acq running     - " << yesno(acq_running) << std::endl;
	if(!pac_mode){
		std::cout << "   Shared memory   - " << yesno(shm_mode) << std::endl;
		std::cout << "   Write to disk   - " << yesno(record_data) << std::endl;
		std::cout << "   File open       - " << yesno(output_file.IsOpen()) << std::endl;
		std::cout << "   Rebooting       - " << yesno(do_reboot) << std::endl;
		std::cout << "   Force Spill     - " << yesno(force_spill) << std::endl;
		std::cout << "   Do MCA run      - " << yesno(do_MCA_run) << std::endl;	
	}
	else{ std::cout << "   Pacman mode     - " << yesno(pac_mode) << std::endl; }
	std::cout << "   Run ctrl Exited - " << yesno(run_ctrl_exit) << std::endl;
				
	std::cout << "\n  Poll Options:\n";
	std::cout << "   Boot fast   - " << yesno(boot_fast) << std::endl;
	std::cout << "   Wall clock  - " << yesno(insert_wall_clock) << std::endl;
	std::cout << "   Is quiet    - " << yesno(is_quiet) << std::endl;
	std::cout << "   Send alarm  - " << yesno(send_alarm) << std::endl;
	std::cout << "   Show rates  - " << yesno(show_module_rates) << std::endl;
	std::cout << "   Zero clocks - " << yesno(zero_clocks) << std::endl;
	std::cout << "   Debug mode  - " << yesno(debug_mode) << std::endl;
	std::cout << "   Initialized - " << yesno(init) << std::endl;
}

void Poll::show_thresh() {
	float threshPercent = (float) threshWords / EXTERNAL_FIFO_LENGTH * 100;
	std::cout << sys_message_head << "Polling Threshold = " << threshPercent << "% (" << threshWords << "/" << EXTERNAL_FIFO_LENGTH << ")\n";
}

/// Acquire raw traces from a pixie module.
void Poll::get_traces(int mod_, int chan_, int thresh_/*=0*/){
	size_t trace_size = PixieInterface::GetTraceLength();
	size_t module_size = pif->GetNumberChannels() * trace_size;
	std::cout << sys_message_head << "Searching for traces from mod = " << mod_ << ", chan = " << chan_ << " above threshold = " << thresh_ << ".\n";
	std::cout << sys_message_head << "Allocating " << (trace_size+module_size)*sizeof(unsigned short) << " bytes of memory for pixie traces.\n";
	std::cout << sys_message_head << "Searching for traces. Please wait...\n";
	poll_term_->flush();
	
	unsigned short *trace_data = new unsigned short[trace_size];
	unsigned short *module_data = new unsigned short[module_size];
	memset(trace_data, 0, sizeof(unsigned short)*trace_size);
	memset(module_data, 0, sizeof(unsigned short)*module_size);

	GetTraces gtraces(module_data, module_size, trace_data, trace_size, thresh_);
	forChannel(pif, mod_, chan_, gtraces, (int)0); 

	if(!gtraces.GetStatus()){ std::cout << sys_message_head << "Failed to find trace above threshold in " << gtraces.GetAttempts() << " attempts!\n"; }
	else{ std::cout << sys_message_head << "Found trace above threshold in " << gtraces.GetAttempts() << " attempts.\n"; }

	std::cout << "  Baselines:\n";
	for(unsigned int channel = 0; channel < pif->GetNumberChannels(); channel++){
		if(channel == (unsigned)chan_){ std::cout << "\033[0;33m"; }
		if(channel < 10){ std::cout << "   0" << channel << ": "; }
		else{ std::cout << "   " << channel << ": "; }
		std::cout << "\t" << gtraces.GetBaseline(channel);
		std::cout << "\t" << gtraces.GetMaximum(channel) << std::endl;
		if(channel == (unsigned)chan_){ std::cout << "\033[0m"; }
	}
	
	std::ofstream get_traces_out("/tmp/traces.dat");
	if(!get_traces_out.good()){ std::cout << sys_message_head << "Could not open /tmp/traces.dat!\n"; }
	else{ // Write the output file.
		// Add a header.
		get_traces_out << "time";
		for(size_t channel = 0; channel < pif->GetNumberChannels(); channel++){
			if(channel < 10){ get_traces_out << "\tC0" << channel; }
			else{ get_traces_out << "\tC" << channel; }
		}
		get_traces_out << std::endl;
		
		// Write channel traces.
		for(size_t index = 0; index < trace_size; index++){
			get_traces_out << index;
			for(size_t channel = 0; channel < pif->GetNumberChannels(); channel++){
				get_traces_out << "\t" << module_data[(channel * trace_size) + index];
			}
			get_traces_out << std::endl;
		} 
		std::cout << sys_message_head << "Traces written to '/tmp/traces.dat'." << std::endl;
	}
	get_traces_out.close();

	delete[] trace_data;
	delete[] module_data;
}

///////////////////////////////////////////////////////////////////////////////
// Poll::CommandControl
///////////////////////////////////////////////////////////////////////////////

/* Function to control the POLL command line interface */
void Poll::CommandControl(){
	std::string cmd = "", arg;

	while(true){
		if(kill_all){ // Check if poll has been killed externally (pacman)
			while(!run_ctrl_exit){ sleep(1); }
			break;
		}
		if(pac_mode){ // Check for commands from pacman, if enabled
			int select_dummy;
			if(server->Select(select_dummy)){
				UDP_Packet pacman_command;
			
				// We have a pacman command. Retrieve the command
				server->RecvMessage((char *)&pacman_command, sizeof(UDP_Packet));
				
				/* Valid poll commands
				    0x11 - INIT_ACQ
				    0x22 - START_ACQ
				    0x33 - STOP_ACQ
				    0x44 - STATUS_ACQ
				    0x55 - PAC_FILE
				    0x66 - HOST
				    0x77 - ZERO_CLK
				   Return codes:
				    0x00 - ACQ_OK
				    0x01 - ACQ_RUN
				    0x02 - ACQ_STOP
				    0xFB - ACQ_STP_HALT
				    0xFC - ACQ_STR_RUN*/

				if(pacman_command.Data[0] == 0x11){ // INIT_ACQ
					std::cout << "RECV PACMAN COMMAND 0x11 (INIT_ACQ)\n";
					pacman_command.Data[0] = 0x00;
				}
				else if(pacman_command.Data[0] == 0x22){ // START_ACQ
					std::cout << "RECV PACMAN COMMAND 0x22 (START_ACQ)\n";
					if(acq_running){ pacman_command.Data[0] = 0xFC; }
					else{ pacman_command.Data[0] = 0x00; }
					do_start_acq = true;
				}
				else if(pacman_command.Data[0] == 0x33){ // STOP_ACQ
					std::cout << "RECV PACMAN COMMAND 0x33 (STOP_ACQ)\n";
					if(!acq_running){ pacman_command.Data[0] = 0xFB; } 
					else{ pacman_command.Data[0] = 0x00; }
					do_stop_acq = true;
				}
				else if(pacman_command.Data[0] == 0x44){ // STATUS_ACQ
					std::cout << "RECV PACMAN COMMAND 0x44 (STATUS_ACQ)\n";
					pacman_command.Data[0] = 0x00;
					pacman_command.Data[1] = acq_running ? 0x01 : 0x02;
				}
				else if(pacman_command.Data[0] == 0x55){ // PAC_FILE
					std::cout << "RECV PACMAN COMMAND 0x55 (PAC_FILE)\n";
					pacman_command.Data[0] = 0x00;
				}
				else if(pacman_command.Data[0] == 0x66){ // HOST
					std::cout << "RECV PACMAN COMMAND 0x66 (HOST)\n";
					pacman_command.Data[0] = 0x00;
				}
				else if(pacman_command.Data[0] == 0x77){ // ZERO_CLK
					std::cout << "RECV PACMAN COMMAND 0x77 (ZERO_CLK)\n";
					pacman_command.Data[0] = 0x00;
				}
				else{ 
					std::cout << "RECV PACMAN COMMAND 0x" << std::hex << (int)pacman_command.Data[0] << std::dec << " (?)\n";
					pacman_command.Data[0] = 0x00; 
				}
				
				// Send the reply to pacman
				server->SendMessage((char *)&pacman_command, sizeof(UDP_Packet));
			}
		}
	
		cmd = poll_term_->GetCommand();
		if(cmd == "_SIGSEGV_"){
			std::cout << Display::ErrorStr("SEGMENTATION FAULT") << std::endl;
			Close();
			exit(EXIT_FAILURE);
		}
		else if(cmd == "CTRL_D"){ 
			std::cout << sys_message_head << "Received EOF (ctrl-d) signal. Exiting...\n";
			cmd = "quit"; 
		}
		else if(cmd == "CTRL_C"){ 
			std::cout << sys_message_head << "Received SIGINT (ctrl-c) signal.";
			if (do_MCA_run) { 
				std::cout << " Stopping MCA...\n";
				cmd = "stop";
			}
			else {
				std::cout << " Ignoring signal.\n";
				continue;
			}
		}
		else if(cmd == "CTRL_Z"){ 
			std::cout << sys_message_head << "Warning! Received SIGTSTP (ctrl-z) signal.\n";
			continue; 
		}	

		if (cmd.find("\t") != std::string::npos) {
			poll_term_->TabComplete(TabComplete(cmd.substr(0,cmd.length()-1)));
			continue;
		}
		poll_term_->flush();

		if(cmd == ""){ continue; }
		
		size_t index = cmd.find(" ");
		if(index != std::string::npos){
			arg = cmd.substr(index+1, cmd.size()-index); // Get the argument from the full input string
			cmd = cmd.substr(0, index); // Get the command from the full input string
		}
		else{ arg = ""; }

		std::vector<std::string> arguments;
		unsigned int p_args = split_str(arg, arguments);
		
		//We clear the error flag when a command is entered.
		had_error = false;
		// check for defined commands
		if(cmd == "quit" || cmd == "exit"){
			if(do_MCA_run){ std::cout << sys_message_head << "Warning! Cannot quit while MCA program is running\n"; }
			else if(acq_running){ std::cout << sys_message_head << "Warning! Cannot quit while acquisition running\n"; }
			else{
				kill_all = true;
				while(!run_ctrl_exit){ sleep(1); }
				break;
			}
		}
		else if(cmd == "kill"){
			if(acq_running || do_MCA_run){ 
				std::cout << sys_message_head << "Sending KILL signal\n";
				do_stop_acq = true; 
			}
			kill_all = true;
			while(!run_ctrl_exit){ sleep(1); }
			break;
		}
		else if(cmd == "help" || cmd == "h"){ help(); }
		else if(cmd == "version" || cmd == "v"){ 
			std::cout << "  Poll2 Core    v" << POLL2_CORE_VERSION << " (" << POLL2_CORE_DATE << ")\n"; 
			std::cout << "  Poll2 Socket  v" << POLL2_SOCKET_VERSION << " (" << POLL2_SOCKET_DATE << ")\n"; 
			std::cout << "  HRIBF Buffers v" << HRIBF_BUFFERS_VERSION << " (" << HRIBF_BUFFERS_DATE << ")\n"; 
			std::cout << "  CTerminal     v" << CTERMINAL_VERSION << " (" << CTERMINAL_DATE << ")\n";
		}
		else if(cmd == "status"){
			show_status();
		}
		else if(cmd == "thresh"){
			if (p_args==1) {
				SetThreshWords(EXTERNAL_FIFO_LENGTH * atof(arguments.at(0).c_str()) / 100.0);
			}
			show_thresh();
		}
		else if(cmd == "dump"){ // Dump pixie parameters to file
			std::ofstream ofile;
			
			if(p_args >= 1){
				ofile.open(arg.c_str());
				if(!ofile.good()){
					std::cout << sys_message_head << "Failed to open output file '" << arg << "'\n";
					std::cout << sys_message_head << "Check that the path is correct\n";
					continue;
				}
			}
			else{
				ofile.open("./Fallback.set");
				if(!ofile.good()){
					std::cout << sys_message_head << "Failed to open output file './Fallback.set'\n";
					continue;
				}
			}

			ParameterChannelDumper chanReader(&ofile);
			ParameterModuleDumper modReader(&ofile);

			// Channel dependent settings
			for(unsigned int param = 0; param < chan_params.size(); param++){
				forChannel<std::string>(pif, -1, -1, chanReader, chan_params[param]);
			}

			// Channel independent settings
			for(unsigned int param = 0; param < mod_params.size(); param++){
				forModule(pif, -1, modReader, mod_params[param]);
			}

			if(p_args >= 1){ std::cout << sys_message_head << "Successfully wrote output parameter file '" << arg << "'\n"; }
			else{ std::cout << sys_message_head << "Successfully wrote output parameter file './Fallback.set'\n"; }
			ofile.close();
		}
		else if(cmd == "pwrite" || cmd == "pmwrite"){ // Write pixie parameters
			if(acq_running || do_MCA_run){ 
				std::cout << sys_message_head << "Warning! Cannot edit pixie parameters while acquisition is running\n\n"; 
				continue;
			}
		
			if(cmd == "pwrite"){ // Syntax "pwrite <module> <channel> <parameter name> <value>"
				if(p_args > 0 && arguments.at(0) == "help"){ pchan_help(); }
				else if(p_args >= 4){
					int mod = atoi(arguments.at(0).c_str());
					int ch = atoi(arguments.at(1).c_str());
					double value = std::strtod(arguments.at(3).c_str(), NULL);
				
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
					unsigned int value = (unsigned int)std::strtoul(arguments.at(2).c_str(), NULL, 0);
				
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
			if(acq_running || do_MCA_run){ 
				std::cout << sys_message_head << "Warning! Cannot view pixie parameters while acquisition is running\n\n"; 
				continue;
			}
		
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
		else if(cmd == "adjust_offsets"){ // Run adjust_offsets
			if(acq_running || do_MCA_run){ 
				std::cout << sys_message_head << "Warning! Cannot edit pixie parameters while acquisition is running\n\n"; 
				continue;
			}

			if(p_args >= 1){
				int mod = atoi(arguments.at(0).c_str());
				
				OffsetAdjuster adjuster;
				if(forModule(pif, mod, adjuster, 0)){ pif->SaveDSPParameters(); }
			}
			else{
				std::cout << sys_message_head << "Invalid number of parameters to adjust_offsets\n";
				std::cout << sys_message_head << " -SYNTAX- adjust_offsets [module]\n";
			}
		}
		else if(cmd == "find_tau"){ // Run find_tau
			if(acq_running || do_MCA_run){ 
				std::cout << sys_message_head << "Warning! Cannot edit pixie parameters while acquisition is running\n\n"; 
				continue;
			}
		
			if(p_args >= 2){
				int mod = atoi(arguments.at(0).c_str());
				int ch = atoi(arguments.at(1).c_str());

				TauFinder finder;
				forChannel(pif, mod, ch, finder, 0);
			}
			else{
				std::cout << sys_message_head << "Invalid number of parameters to find_tau\n";
				std::cout << sys_message_head << " -SYNTAX- find_tau [module] [channel]\n";
			}
		}
		else if(cmd == "toggle"){ // Toggle a CHANNEL_CSRA bit
			if(acq_running || do_MCA_run){ 
				std::cout << sys_message_head << "Warning! Cannot edit pixie parameters while acquisition is running\n\n"; 
				continue;
			}

			BitFlipper flipper;

			if(p_args >= 3){ 
				flipper.SetCSRAbit(arguments.at(2));
				
				std::string dum_str = "CHANNEL_CSRA";
				if(forChannel(pif, atoi(arguments.at(0).c_str()), atoi(arguments.at(1).c_str()), flipper, dum_str)){
					pif->SaveDSPParameters();
				}
			}
			else{
				std::cout << sys_message_head << "Invalid number of parameters to toggle\n";
				std::cout << sys_message_head << " -SYNTAX- toggle [module] [channel] [CSRA bit]\n\n";
				flipper.Help();				
			}
		}
		else if(cmd == "toggle_bit"){ // Toggle any bit of any parameter under 32 bits long
			if(acq_running || do_MCA_run){ 
				std::cout << sys_message_head << "Warning! Cannot edit pixie parameters while acquisition is running\n\n"; 
				continue;
			}

			BitFlipper flipper;

			if(p_args >= 4){ 
				flipper.SetBit(arguments.at(3));

				if(forChannel(pif, atoi(arguments.at(0).c_str()), atoi(arguments.at(1).c_str()), flipper, arguments.at(2))){
					pif->SaveDSPParameters();
				}
			}
			else{
				std::cout << sys_message_head << "Invalid number of parameters to toggle_any\n";
				std::cout << sys_message_head << " -SYNTAX- toggle_any [module] [channel] [parameter] [bit]\n\n";
			}
		}
		else if(cmd == "csr_test"){ // Run CSRAtest method
			BitFlipper flipper;
			if(p_args >= 1){ flipper.CSRAtest((unsigned int)atoi(arguments.at(0).c_str())); }
			else{
				std::cout << sys_message_head << "Invalid number of parameters to csr_test\n";
				std::cout << sys_message_head << " -SYNTAX- csr_test [number]\n";
			}
		}
		else if(cmd == "bit_test"){ // Run Test method
			BitFlipper flipper;
			if(p_args >= 2){ flipper.Test((unsigned int)atoi(arguments.at(0).c_str()), std::strtoul(arguments.at(1).c_str(), NULL, 0)); }
			else{
				std::cout << sys_message_head << "Invalid number of parameters to bit_test\n";
				std::cout << sys_message_head << " -SYNTAX- bit_test [num_bits] [number]\n";
			}
		}
		else if(cmd == "get_traces"){ // Run GetTraces method
			if(acq_running || do_MCA_run){ 
				std::cout << sys_message_head << "Warning! Cannot view live traces while acquisition is running\n\n"; 
				continue;
			}

			if(p_args >= 2){
				int mod = atoi(arguments.at(0).c_str());
				int chan = atoi(arguments.at(1).c_str());
				
				if(mod < 0 || chan < 0){
					std::cout << sys_message_head << "Error! Must select one module and one channel to trigger on!\n";
					continue;
				}
				else if(mod > (int)n_cards){
					std::cout << sys_message_head << "Error! Invalid module specification (" << mod << ")!\n";
					continue;
				}
				else if(chan > NUMBER_OF_CHANNELS){
					std::cout << sys_message_head << "Error! Invalid channel specification (" << chan << ")!\n";
					continue;
				}

				int trace_threshold = 0;
				if(p_args >= 3){
					trace_threshold = atoi(arguments.at(2).c_str());
					if(trace_threshold < 0){
						std::cout << sys_message_head << "Cannot set negative threshold!\n";
						trace_threshold = 0;
					}
				}
				
				get_traces(mod, chan, trace_threshold);
			}
			else{
				std::cout << sys_message_head << "Invalid number of parameters to get_traces\n";
				std::cout << sys_message_head << " -SYNTAX- get_traces [mod] [chan] <threshold> <correct-baselines>\n";
			}
		}
		else if(cmd == "quiet"){ // Toggle quiet mode
			if(is_quiet){
				std::cout << sys_message_head << "Toggling quiet mode OFF\n";
				is_quiet = false;
			}
			else{
				std::cout << sys_message_head << "Toggling quiet mode ON\n";
				is_quiet = true;
			}
		}
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
		else if(!pac_mode){ // These command are only available when not running in pacman mode!
			if(cmd == "run"){ start_run(); } // Tell POLL to start acq and start recording data to disk
			else if(cmd == "startacq" || cmd == "startvme"){ // Tell POLL to start data acquisition
				start_acq();
			}
			else if(cmd == "stop"){ // Tell POLL to stop recording data to disk and stop acq
				stop_run();
			} 
			else if(cmd == "stopacq" || cmd == "stopvme"){ // Tell POLL to stop data acquisition
				stop_acq();
			}
			else if(cmd == "shm"){ // Toggle "shared-memory" mode
				if(shm_mode){
					std::cout << sys_message_head << "Toggling shared-memory mode OFF\n";
					shm_mode = false;
				}
				else{
					std::cout << sys_message_head << "Toggling shared-memory mode ON\n";
					shm_mode = true;
				}
			}
			else if(cmd == "reboot"){ // Tell POLL to attempt a PIXIE crate reboot
				if(do_MCA_run){ std::cout << sys_message_head << "Warning! Cannot reboot while MCA is running\n"; }
				else if(acq_running || do_MCA_run){ std::cout << sys_message_head << "Warning! Cannot reboot while acquisition running\n"; }
				else{ 
					do_reboot = true; 
					poll_term_->pause(do_reboot);
				}
			}
			else if(cmd == "clo" || cmd == "close"){ // Tell POLL to close the current data file
				if(do_MCA_run){ std::cout << sys_message_head << "Command not available for MCA run\n"; }
				else if(acq_running && record_data){ std::cout << sys_message_head << "Warning! Cannot close file while acquisition running\n"; }
				else{ CloseOutputFile(); }
			}
			else if(cmd == "hup" || cmd == "spill"){ // Force spill
				if(do_MCA_run){ std::cout << sys_message_head << "Command not available for MCA run\n"; }
				else if(!acq_running){ std::cout << sys_message_head << "Acquisition is not running\n"; }
				else{ force_spill = true; }
			}
			else if(cmd == "fdir"){ // Change the output file directory
				if (arg == "") { std::cout << sys_message_head << "Using output directory '" << output_directory << "'\n"; }
				else if (file_open) {
					std::cout << sys_message_head << Display::WarningStr("Warning:") << " Directory cannot be changed while a file is open!\n";
				}
				else {
					output_directory = arg; 
					current_file_num = 0;
			
					// Append a '/' if the user did not include one
					if(*(output_directory.end()-1) != '/'){ output_directory += '/'; }

					std::cout << sys_message_head << "Set output directory to '" << output_directory << "'.\n";

					//Check what run files already exist.
					int temp_run_num = next_run_num;
					std::string filename = output_file.GetNextFileName(next_run_num,filename_prefix, output_directory);
					if (temp_run_num != next_run_num) {
						std::cout << sys_message_head << Display::WarningStr("Warning") << ": Run file existed for run " << temp_run_num << "! Next run number will be " << next_run_num << ".\n";
					}

					std::cout << sys_message_head << "Next file will be '" << filename << "'.\n";
				}
			} 
			else if (cmd == "prefix") {
				if (arg == "") {
					std::cout << sys_message_head << "Using output filename prefix '" << filename_prefix << "'.\n";
				}
				else if (file_open) {
					std::cout << sys_message_head << Display::WarningStr("Warning:") << " Prefix cannot be changed while a file is open!\n";
				}
				else {
					filename_prefix = arg;
					next_run_num = 1;

					//Check what run files already exist.
					std::string filename = output_file.GetNextFileName(next_run_num,filename_prefix, output_directory);
					if (next_run_num != 1) {
						std::cout << sys_message_head << Display::WarningStr("Warning") << ": Some run files existed! Next run number will be " << next_run_num << ".\n";
					}

					std::cout << sys_message_head << "Set output filename prefix to '" << filename_prefix << "'.\n";
					std::cout << sys_message_head << "Next file will be '" << output_file.GetNextFileName(next_run_num,filename_prefix, output_directory) << "'.\n";
				}
			}
			else if(cmd == "title"){ // Change the title of the output file
				if (arg == "") { std::cout << sys_message_head << "Using output file title '" << output_title << "'.\n"; }
				else if (file_open) {
					std::cout << sys_message_head << Display::WarningStr("Warning:") << " Run title cannot be changed while a file is open!\n";
				}
				else {
					output_title = arg; 
					std::cout << sys_message_head << "Set run title to '" << output_title << "'.\n";
				}
			} 
			else if(cmd == "runnum"){ // Change the run number to the specified value
				if (arg == "") { 
					if (output_file.IsOpen()) 
						std::cout << sys_message_head << "Current output file run number '" << output_file.GetRunNumber() << "'.\n"; 
					if (!output_file.IsOpen() || next_run_num != output_file.GetRunNumber()) 
						std::cout << sys_message_head << "Next output file run number '" << next_run_num << "' for prefix '" << filename_prefix << "'.\n"; 
				}
				else if (file_open) {
					std::cout << sys_message_head << Display::WarningStr("Warning:") << " Run number cannot be changed while a file is open!\n";
				}
				else {
					next_run_num = atoi(arg.c_str()); 
					std::string filename = output_file.GetNextFileName(next_run_num,filename_prefix, output_directory);
					if (next_run_num != atoi(arg.c_str())) {
						std::cout << sys_message_head << Display::WarningStr("Wanring") << ": Run file existed for run " << atoi(arg.c_str()) << ".\n";
					}
					std::cout << sys_message_head << "Set run number to '" << next_run_num << "'.\n";
					std::cout << sys_message_head << "Next file will be '" << filename << "'.\n";
				}
			} 
			else if(cmd == "oform"){ // Change the output file format
				if(arg != ""){
					int format = atoi(arg.c_str());
					if(format == 0 || format == 1 || format == 2){
						output_format = atoi(arg.c_str());
						std::cout << sys_message_head << "Set output file format to '" << output_format << "'\n";
						if(output_format == 1){ std::cout << "  Warning! This output format is experimental and is not recommended for data taking\n"; }
						else if(output_format == 2){ std::cout << "  Warning! This output format is experimental and is not recommended for data taking\n"; }
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
				else{ std::cout << sys_message_head << "Using output file format '" << output_format << "'\n"; }
				if(output_file.IsOpen()){ std::cout << sys_message_head << "New output format used for new files only! Current file is unchanged.\n"; }
			}
			else if(cmd == "mca" || cmd == "MCA"){ // Run MCA program using either root or damm
				if(do_MCA_run){
					std::cout << sys_message_head << "MCA program is already running\n\n";
					continue;
				}
				else if(acq_running){ 
					std::cout << sys_message_head << "Warning! Cannot run MCA program while acquisition is running\n\n";
					continue;
				}

				if (p_args >= 1) {
					std::string type = arguments.at(0);
					
					if(type == "root"){ mca_args.SetUseRoot(); }
					else if(type != "damm"){ mca_args.SetTotalTime(atoi(type.c_str())); }
					
					if(p_args >= 2){
						if(mca_args.GetTotalTime() == 0){ mca_args.SetTotalTime(atoi(arguments.at(1).c_str())); }
						else{ mca_args.SetBasename(arguments.at(1)); }
						if(p_args >= 3){ mca_args.SetBasename(arguments.at(2)); }
					}
				}
				
				if(mca_args.GetTotalTime() == 0){ 
					mca_args.SetTotalTime(10);
					std::cout << sys_message_head << "Using default MCA time of 10 seconds\n";
				}
		
				do_MCA_run = true;
			}
			else{ std::cout << sys_message_head << "Unknown command '" << cmd << "'\n"; }
		}
		else{ std::cout << sys_message_head << "Unknown command '" << cmd << "'\n"; }
	}
}

///////////////////////////////////////////////////////////////////////////////
// Poll::RunControl
///////////////////////////////////////////////////////////////////////////////

/// Function to control the gathering and recording of PIXIE data
void Poll::RunControl(){
	while(true){
		if(kill_all){ // Supersedes all other commands
			if(acq_running || mca_args.IsRunning()){ do_stop_acq = true; } // Safety catch
			else{ break; }
		}
		
		if(do_reboot){ // Attempt to reboot the PIXIE crate
			if(acq_running){ do_stop_acq = true; } // Safety catch
			else{
				std::cout << sys_message_head << "Attempting PIXIE crate reboot\n";
				pif->Boot(PixieInterface::BootAll);
				printf("Press any key to continue...");
				std::cin.get();
				do_reboot = false;
			}
		}

		if(do_MCA_run){ // Do an MCA run, if the acq is not running.
			if(acq_running){ do_stop_acq = true; } // Safety catch.
			else{
				if(!mca_args.IsRunning()){ // Start the pixie histogram run.
					if(mca_args.GetTotalTime() > 0.0){ std::cout << sys_message_head << "Performing MCA data run for " << mca_args.GetTotalTime() << " s\n"; }
					else{ std::cout << sys_message_head << "Performing infinite MCA data run. Type \"stop\" to quit\n"; }

					if(!mca_args.Initialize(pif)){
						std::cout << Display::ErrorStr("Run TERMINATED") << std::endl;
						do_MCA_run = false;
						had_error = true;
						continue;
					}
				}
				
				if(!mca_args.CheckTime() || do_stop_acq){ // End the run.
					pif->EndRun();
					std::cout << sys_message_head << "Ending MCA run.\n";
					std::cout << sys_message_head << "Ran for " << mca_args.GetMCA()->GetRunTime() << " s.\n";
					mca_args.Close(pif);
					do_stop_acq = false;
					do_MCA_run = false;
				}
				else{
					sleep(1); // Sleep for a small amount of time.
					if(!mca_args.Step()){ // Update the histograms.
						std::cout << Display::ErrorStr("Run TERMINATED") << std::endl;
						mca_args.Close(pif);
						do_MCA_run = false;
						had_error = true;
					}
				}
			}
		}

		//Start acquistion
		if (do_start_acq) {
			if (!acq_running) {
				//Start list mode
				if(pif->StartListModeRun(LIST_MODE_RUN, NEW_RUN)) {
					time_t currTime;
					time(&currTime);
					if (record_data) std::cout << "Run " << output_file.GetRunNumber();
					else std::cout << "Acq";
					std::cout << " started on " << ctime(&currTime);

					acq_running = true;
					startTime = usGetTime(0);
					lastSpillTime = 0;
				}
				else{ 
					std::cout << sys_message_head << "Failed to start list mode run. Try rebooting PIXIE\n"; 
					acq_running = false;
					had_error = true;
				}
				do_start_acq = false;
			} //if(!acq_running)
			else  {
				std::cout << sys_message_head << "Already running!\n";
				do_start_acq = false;
			}
		}

		if(acq_running){
			ReadFIFO();

			//Handle a stop signal
			if(do_stop_acq){ 
				pif->EndRun();

				time_t currTime;
				time(&currTime);
				
				// Check if each module has ended its run properly.
				for(size_t mod = 0; mod < n_cards; mod++){
					//If the run status is 1 then the run has not finished in the module.
					// We need to read it out.
					if(pif->CheckRunStatus(mod) == 1) {
						if (!is_quiet) std::cout << "Module " << mod << " still has " << pif->CheckFIFOWords(mod) << " words in the FIFO.\n";
						//We set force_spill to true in case the remaining words is small.
						force_spill = true;
						//We sleep to allow the module to finish.
						sleep(1);
						//We read the FIFO out.
						ReadFIFO();
					}

					//Print the module status.
					std::stringstream leader;
					leader << "Run end status in module " << mod;
					if (!partialEvents[mod].empty()) {
						///\bug Warning Str colors oversets the number of characters.
						leader << Display::WarningStr(" (partial evt)");
						partialEvents[mod].clear();
					}
					
					Display::LeaderPrint(leader.str());
					if(!pif->CheckRunStatus(mod)){
						std::cout << Display::OkayStr() << std::endl;
					}
					else {
						std::cout << Display::ErrorStr() << std::endl;
						had_error = true;
					}
				}

				if (record_data) std::cout << "Run " << output_file.GetRunNumber();
				else std::cout << "Acq";
				std::cout << " stopped on " << ctime(&currTime);

				statsHandler->ClearRates();
				statsHandler->Dump();
				statsHandler->ClearTotals();

				//Reset status flags
				do_stop_acq = false;
				acq_running = false;
			} //if (do_stop_acq) -- End of handling a stop acq flag
		}
	
		UpdateStatus();

		//Sleep the run control if idle to reduce CPU utilization.
		if (!acq_running && !do_MCA_run) sleep(1);
	}

	run_ctrl_exit = true;
	std::cout << "Run Control exited\n";
}
void Poll::UpdateStatus() {
	//Build status string
	std::stringstream status;
	if (had_error) status << Display::ErrorStr("[ERROR]");
	else if (acq_running && record_data) status << Display::OkayStr("[ACQ]");
	else if (acq_running && !record_data) status << Display::WarningStr("[ACQ]");
	else if (do_MCA_run) status << Display::OkayStr("[MCA]");
	else status << Display::InfoStr("[IDLE]");

	if (file_open) status << " Run " << output_file.GetRunNumber();

	if(do_MCA_run){
		status << " " << (int)mca_args.GetMCA()->GetRunTime() << "s";
		status << " of " << mca_args.GetTotalTime() << "s";
	}
	else{
		//Add run time to status
		status << " " << (long long) statsHandler->GetTotalTime() << "s";
		//Add data rate to status
		status << " " << humanReadable(statsHandler->GetTotalDataRate()) << "/s";
	}

	if (file_open) {
		if (acq_running && !record_data) status << TermColors::DkYellow;
		//Add file size to status
		status << " " << humanReadable(output_file.GetFilesize());
		status << " " << output_file.GetCurrentFilename();
		if (acq_running && !record_data) status << TermColors::Reset;
	}

	//Update the status bar
	poll_term_->SetStatus(status.str());
}


void Poll::ReadScalers() {
	static std::vector< std::pair<double, double> > xiaRates(16, std::make_pair<double, double>(0,0));
	static int numChPerMod = pif->GetNumberChannels();

	for (unsigned short mod=0;mod < n_cards; mod++) {
		//Tell interface to get stats data from the modules.
		pif->GetStatistics(mod);

		for (int ch=0;ch< numChPerMod; ch++) 
			xiaRates[ch] = std::make_pair<double, double>(pif->GetInputCountRate(mod, ch),pif->GetOutputCountRate(mod,ch));

		//Populate Stats Handler with ICR and OCR.
		statsHandler->SetXiaRates(mod, &xiaRates);
	}
}
bool Poll::ReadFIFO() {
	static word_t *fifoData = new word_t[(EXTERNAL_FIFO_LENGTH + 2) * n_cards];

	if (!acq_running) return false;

	//Number of words in the FIFO of each module.
	std::vector<word_t> nWords(n_cards);
	//Iterator to determine which card has the most words.
	std::vector<word_t>::iterator maxWords;

	//We loop until the FIFO has reached the threshold for any module unless we are stopping and then we skip the loop.
	for (unsigned int timeout = 0; timeout < POLL_TRIES; timeout++){ 
		//Check the FIFO size for every module
		for (unsigned short mod=0; mod < n_cards; mod++) {
			nWords[mod] = pif->CheckFIFOWords(mod);
		}
		//Find the maximum module
		maxWords = std::max_element(nWords.begin(), nWords.end());
		if(*maxWords > threshWords){ break; }
	}

	//We need to read the data out of the FIFO
	if (*maxWords > threshWords || force_spill) {
		force_spill = false;
		//Number of data words read from the FIFO
		size_t dataWords = 0;

		//Loop over each module's FIFO
		for (unsigned short mod=0;mod < n_cards; mod++) {

			//if the module has no words in the FIFO we continue to the next module
			if (nWords[mod] < MIN_FIFO_READ) {
				// write an empty buffer if there is no data
				fifoData[dataWords++] = 2;
				fifoData[dataWords++] = mod;	    
				continue;
			}
			else if (nWords[mod] < 0) {
				std::cout << Display::WarningStr("Number of FIFO words less than 0") << " in module " << mod << std::endl;
				// write an empty buffer if there is no data
				fifoData[dataWords++] = 2;
				fifoData[dataWords++] = mod;	    
				continue;
			}

			//Check if the FIFO is overfilled
			bool fullFIFO = (nWords[mod] >= EXTERNAL_FIFO_LENGTH);
			if (fullFIFO) {
				std::cout << Display::ErrorStr() << " Full FIFO in module " << mod 
					<< " size: " << nWords[mod] << "/" 
					<< EXTERNAL_FIFO_LENGTH << Display::ErrorStr(" ABORTING!") << std::endl;
				had_error = true;
				do_stop_acq = true;
				return false;
			}

			//We inject two words describing the size of the FIFO spill and the module.
			//We inject the size after it has been computedos we skip it for now and only add the module number.
			dataWords++;
			fifoData[dataWords++] = mod;

			//We store the partial event if we had one
			for (size_t i=0;i<partialEvents[mod].size();i++)
				fifoData[dataWords + i] = partialEvents[mod].at(i);

			//Try to read FIFO and catch errors.
			if(!pif->ReadFIFOWords(&fifoData[dataWords + partialEvents[mod].size()], nWords[mod], mod, debug_mode)){
				std::cout << Display::ErrorStr() << " Unable to read " << nWords[mod] << " from module " << mod << "\n";
				had_error = true;
				do_stop_acq = true;
				return false;
			}

			//Print a message about what we did	
			if(!is_quiet || debug_mode) {
				std::cout << "Read " << nWords[mod] << " words from module " << mod;
				if (!partialEvents[mod].empty())
					std::cout << " and stored " << partialEvents[mod].size() << " partial event words";
				std::cout << " to buffer position " << dataWords << std::endl;
			}

			//After reading the FIFO and printing a sttus message we can update the number of words to include the partial event.
			nWords[mod] += partialEvents[mod].size();
			//Clear the partial event
			partialEvents[mod].clear();

			//We now ned to parse the event to determine if there is a hanging event. Also, allows a check for corrupted data.
			size_t parseWords = dataWords;
			//We declare the eventSize outside the loop in case there is a partial event.
			word_t eventSize = 0;
			word_t slotExpected = pif->GetSlotNumber(mod);
			while (parseWords < dataWords + nWords[mod]) {
				//Check first word to see if data makes sense.
				// We check the slot, channel and event size.
				word_t slotRead = ((fifoData[parseWords] & 0xF0) >> 4);
				word_t chanRead = (fifoData[parseWords] & 0xF);
				eventSize = ((fifoData[parseWords] & 0x7FFE2000) >> 17);
				bool virtualChannel = ((fifoData[parseWords] & 0x20000000) != 0);

				if( slotRead != slotExpected ){ 
					std::cout << Display::ErrorStr() << " Slot read (" << slotRead 
						<< ") not the same as" << " slot expected (" 
						<< slotExpected << ")" << std::endl; 
					break;
				}
				else if (chanRead < 0 || chanRead > 15) {
					std::cout << Display::ErrorStr() << " Channel read (" << chanRead << ") not valid!\n";
					break;
				}
				else if(eventSize == 0){ 
					std::cout << Display::ErrorStr() << "ZERO EVENT SIZE in mod " << mod << "!\n"; 
					break;
				}

				// Update the statsHandler with the event (for monitor.bash)
				if(!virtualChannel && statsHandler){ 
					statsHandler->AddEvent(mod, chanRead, sizeof(word_t) * eventSize); 
				}

				//Iterate to the next event and continue parsing
				parseWords += eventSize;
			}

			//We now check the outcome of the data parsing.
			//If we have too many words as an event was not completely pulled form the FIFO
			if (parseWords > dataWords + nWords[mod]) {
				word_t missingWords = parseWords - dataWords - nWords[mod];
				word_t partialSize = eventSize - missingWords;
				if (debug_mode) std::cout << "Partial event " << partialSize << "/" << eventSize << " words!\n";

				//We could get the words now from the FIFO, but me may have to wait. Instead we store the partial event for the next FIFO read.
				for(unsigned short i=0;i< partialSize;i++) 
					partialEvents[mod].push_back(fifoData[parseWords - eventSize + i]);

				//Update the number of words to indicate removal or partial event.
				nWords[mod] -= partialSize;

			}
			//If parseWords is small then the parse failed for some reason
			else if (parseWords < dataWords + nWords[mod]) {
				std::cout << Display::ErrorStr() << " Parsing indicated corrupted data at " << parseWords - dataWords << " words into FIFO.\n";

				std::cout << std::hex;
				//Print the previous words
				std::cout << "Words prior to parsing error:\n";
				for(int i=0;i< 100;i++) {
					if (i%10 == 0) std::cout << std::endl << "\t";
					std::cout << fifoData[dataWords + parseWords - 100 + i] << " ";
				}
				//Print the following words 
				std::cout << "Words following parsing error:\n";
				for(int i=0;i< 100;i++) {
					if (i%10 == 0) std::cout << std::endl << "\t";
					std::cout << fifoData[dataWords + parseWords + i] << " ";
				}
				std::cout << std::dec << std::endl;

				do_stop_acq = true;
				had_error = true;
				return false;
			}

			//Assign the first injected word of spill to final spill length
			fifoData[dataWords - 2] = nWords[mod] + 2;
			//The data should be good so we iterate the position in the storage array.
			dataWords += nWords[mod];
		} //End loop over modules for reading FIFO

		//Get the length of the spill
		double spillTime = usGetTime(startTime);
		double durSpill = spillTime - lastSpillTime;
		lastSpillTime = spillTime;

		// Add time to the statsHandler and check if interval has been exceeded.
		//If exceed interval we read the scalers from the modules and dump the stats.
		if (statsHandler->AddTime(durSpill * 1e-6)) {
			ReadScalers();
			statsHandler->Dump();
			statsHandler->ClearRates();
		}

		if (!is_quiet || debug_mode) std::cout << "Writing/Broadcasting " << dataWords << " words.\n";
		//We have read the FIFO now we write the data	
		if (record_data && !pac_mode) write_data(fifoData, dataWords); 
		broadcast_data(fifoData, dataWords);

	} //If we had exceeded the threshold or forced a flush

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Support Functions
///////////////////////////////////////////////////////////////////////////////

std::string humanReadable(double size) {
	int power = std::log10(size);
	std::stringstream output;
	output << std::setprecision(3);
	if (power >= 9) output << size/pow(1024,3) << "GB";
	else if (power >= 6) output << size/pow(1024,2) << "MB";
	else if (power >= 3) output << size/1024 << "kB";
	else output << " " << size << "B";
	return output.str(); 
}


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
