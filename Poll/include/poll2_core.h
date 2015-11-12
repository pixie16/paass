/** \file poll2_core.h
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

#ifndef POLL2_CORE_H
#define POLL2_CORE_H

#include <vector>

#include "PixieInterface.h"
#include "hribf_buffers.h"
#define maxEventSize 4095 // (0x1FFE0000 >> 17)

#define POLL2_CORE_VERSION "1.4.0"
#define POLL2_CORE_DATE "Nov. 12th, 2015"

// Maximum length of UDP data packet (in bytes)
#define MAX_ORPH_DATA 1464

typedef PixieInterface::word_t word_t;
typedef word_t eventdata_t[maxEventSize];

class MCA;

class MCA_args{
  private:
	bool running;
	bool useRoot;
	int totalTime;
	std::string basename;
	
	MCA *mca;

  public:	
	MCA_args();
	
	MCA_args(bool useRoot_, int totalTime_, std::string basename_);
	
	~MCA_args();
	
	bool IsRunning(){ return running; }
	
	bool UseRoot(){ return useRoot; }
	
	int GetTotalTime(){ return totalTime; }
	
	std::string GetBasename(){ return basename; }
	
	MCA *GetMCA(){ return mca; }
	
	void SetUseRoot(bool state_=true){ useRoot = state_; }
	
	void SetTotalTime(int totalTime_){ totalTime = totalTime_; }
	
	void SetBasename(std::string basename_){ basename = basename_; }

	bool Initialize(PixieInterface *pif_);
	
	bool Step();
	
	bool CheckTime();
	
	void Zero();
	
	void Close(PixieInterface *pif_);
};

struct UDP_Packet {
	int Sequence; /// Sequence number for reliable transport
	int DataSize; /// Number of useable bytes in Data
	unsigned char Data[MAX_ORPH_DATA]; /// The data to be transmitted
};

struct data_pack{
	int Sequence; /// Sequence number for reliable transport
	int DataSize; /// Number of useable bytes in Data
	int TotalEvents; /// Total number of events
	unsigned short Events; /// Number of events in this packet
	unsigned short Cont; /// Continuation flag for large events
	unsigned char Data[4*(4050 + 4)]; /// The data to be transmitted
	int BufLen; /// Length of original Pixie buffer
};

// Forward class declarations
class StatsHandler;
class Client;
class Server;
class Terminal;

class Poll{
  private:
	Terminal *poll_term_;
	///A vector to store the partial events
	std::vector<word_t> *partialEvents;
	
	double startTime; ///Time when the acquistion was started.
	double lastSpillTime; ///Time when the last spill finished.

  	struct tm *time_info;

	Client *client; /// UDP client for network access
	Server *server; /// UDP server to listen for pacman commands

	PixieInterface *pif; /// The main pixie interface pointer 
  
	// System flags and variables
	std::string sys_message_head; /// Command line message header
	bool kill_all; /// Set to true when the program is exiting
	bool do_start_acq; /// Set to true when the command is given to start a run
	bool do_stop_acq; /// Set to true when the command is given to stop a run
	bool record_data; /// Set to true if data is to be recorded to disk
	bool do_reboot; /// Set to true when the user tells POLL to reboot PIXIE
	bool force_spill; /// Force poll2 to dump the current data spill
	bool acq_running; /// Set to true when run_command is recieving data from PIXIE
	bool run_ctrl_exit; /// Set to true when run_command exits
	bool had_error;
	bool file_open;
	time_t raw_time;

	// System MCA flags
	bool do_MCA_run; /// Set to true when the "mca" command is received
	MCA_args mca_args; /// Structure to hold arguments for MCA program

	// Run control variables
	bool boot_fast; //
	bool insert_wall_clock; //
	bool is_quiet; //
	bool send_alarm; //
	bool show_module_rates; //
	bool zero_clocks; //
	bool debug_mode; //
	bool shm_mode; /// New style shared-memory mode.
	bool pac_mode; /// Pacman shared-memory mode.
	bool init; //

	// Options relating to output data file
	std::string output_directory; /// Set with 'fdir' command
	std::string filename_prefix; /// Set with 'ouf' command
	std::string output_title; /// Set with 'htit' command
	int next_run_num; /// Set with 'hnum' command
	int output_format; /// Set with 'oform' command

	// The main output data file and related variables
	int current_file_num;
	PollOutputFile output_file;

	///Pacman related variables
	unsigned int udp_sequence; ///< The number of UDP packets transmitted.
	unsigned int total_spill_chunks; ///< Total number of poll data spill chunks sent over the network

	size_t n_cards;
	size_t threshWords;

	typedef std::pair<unsigned int, unsigned int> chanid_t;
	std::map<chanid_t, PixieInterface::Histogram> histoMap;

	StatsHandler *statsHandler;
	static const int statsInterval_ = 3; ///<The amount time between scaler reads in seconds.

	const static std::vector<std::string> runControlCommands_;
	const static std::vector<std::string> paramControlCommands_;
	const static std::vector<std::string> pollStatusCommands_; 
	std::vector<std::string> commands_;

	
	data_pack AcqBuf; /// Data packet for class shared-memory broadcast
	
	/// Print help dialogue for POLL options.
	void help();

	/// Print help dialogue for reading/writing pixie channel parameters.
	void pchan_help();

	/// Print help dialogue for reading/writing pixie module parameters.
	void pmod_help();

	/// Start a data recording run.
	bool start_run();
	
	/// Stop an active data recording run.
	bool stop_run();
	
	/// Starts data acquisition.
	bool start_acq();
	
	/// Stops data acquisition.
	bool stop_acq();
	
	/// Display run status information.
	void show_status();
	
	/// Display polling threshold.
	void show_thresh();

	/// Acquire raw traces from a pixie module.
	void get_traces(int mod_, int chan_, int thresh_=0);

	/// Method responsible for handling tab complete of commands and pread/pwrite parameters
	std::vector<std::string> TabComplete(std::string cmd);

	///Routine to read Pixie FIFOs
	bool ReadFIFO();
	///Routine to read Pixie scalers.
	void ReadScalers();

	///Routine to update the status message.
	void UpdateStatus();

	/// Set IN_SYNCH and SYNCH_WAIT parameters on all modules.
	bool synch_mods();

	/// Return the current output filename.
	std::string get_filename();
	
	/// Close the current output file, if one is open.
	bool CloseOutputFile(const bool continueRun = false);
	
	/// Opens a new file if no file is currently open.
	bool OpenOutputFile(bool continueRun = false);
	
	/// Write a data spill to disk.
	int write_data(word_t *data, unsigned int nWords);

	/// Broadcast a data spill onto the network.
	void broadcast_data(word_t *data, unsigned int nWords);

	/// Broadcast a data spill onto the network in the classic pacman format.
	void broadcast_pac_data();

  public:
  	/// Default constructor.
	Poll();
	
	/// Destructor.
	~Poll();
	
	/// Initialize the poll object.
	bool Initialize();
	
	// Set methods.
	void SetBootFast(bool input_=true){ boot_fast = input_; }
	
	void SetWallClock(bool input_=true){ insert_wall_clock = input_; }
	
	void SetQuietMode(bool input_=true){ is_quiet = input_; }
	
	void SetSendAlarm(bool input_=true){ send_alarm = input_; }
	
	void SetShowRates(bool input_=true){ show_module_rates = input_; }
	
	void SetZeroClocks(bool input_=true){ zero_clocks = input_; }
	
	void SetDebugMode(bool input_=true){ debug_mode = input_; }
	
	void SetShmMode(bool input_=true){ shm_mode = input_; }
	
	void SetPacmanMode(bool input_=true){ pac_mode = input_; }
	
	void SetNcards(const size_t &n_cards_){ n_cards = n_cards_; }
	
	void SetThreshWords(const size_t &thresh_){ threshWords = thresh_; }

	///Set the terminal pointer.
	void SetTerminal(Terminal *term){ poll_term_ = term; };

	// Get methods.
	bool GetBootFast(){ return boot_fast; }
	
	bool GetWallClock(){ return insert_wall_clock; }
	
	bool GetQuietMode(){ return is_quiet; }
	
	bool GetSendAlarm(){ return send_alarm; }
	
	bool GetShowRates(){ return show_module_rates; }
	
	bool GetZeroClocks(){ return zero_clocks; }
	
	bool GetDebugMode(){ return debug_mode; }
	
	bool GetShmMode(){ return shm_mode; }
	
	bool GetPacmanMode(){ return pac_mode; }
	
	size_t GetNcards(){ return n_cards; }
	
	size_t GetThreshWords(){ return threshWords; }
	
	/// Main control loop for handling user input.
	void CommandControl();
		
	/// Main acquisition control loop for handling data acq.
	void RunControl();
	
	/// Close the sockets, any open files, and clean up.
	bool Close();
};
	
/// Convert a rate number to more useful form.
std::string humanReadable(double size);

/// Split a string by specified delimiter_ and place the result in a vector.
unsigned int split_str(std::string str_, std::vector<std::string> &args, char delimiter_=' ');

/// Pad a string with periods until it is the specified length_.
std::string pad_string(const std::string &input_, unsigned int length_);

/// Return a string containing "yes" for value_==true and "no" for value_==false.
std::string yesno(bool value_);

#endif
