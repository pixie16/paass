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
  * \date May 11th, 2015
  * 
  * \version 1.3.01
*/


#ifndef POLL2_CORE_H
#define POLL2_CORE_H

#include <vector>

#include "PixieInterface.h"
#include "hribf_buffers.h"
#define maxEventSize 4095 // (0x1FFE0000 >> 17)

#define POLL2_CORE_VERSION "1.3.01"
#define POLL2_CORE_DATE "May 11th, 2015"

typedef PixieInterface::word_t word_t;
typedef word_t eventdata_t[maxEventSize];

struct MCA_args{
	bool useRoot;
	int totalTime;
	std::string basename;
	
	MCA_args();
	MCA_args(bool useRoot_, int totalTime_, std::string basename_);
	
	void Zero();
};

// Forward class declarations
class StatsHandler;
class Client;
class Terminal;

class Poll{
	private:
		Terminal *poll_term_;
		///Start a data recording run.
		bool StartRun();
		///Stop an active data recording run.
		bool StopRun();
		///Starts data acquisition.
		bool StartAcq();
		///Stops data acquisition.
		bool StopAcq();

		std::vector<std::string> TabComplete(std::string cmd);

		///Routine to read Pixie FIFOs
		bool ReadFIFO();
		///Time when the acquistion was started.
		double startTime;
		///Time when the last spill finished.
		double lastSpillTime;

	public:
  	struct tm *time_info;

	Client *client; /// UDP client for network access

	PixieInterface *pif; /// The main pixie interface pointer 
  
	word_t clock_vsn;	

	// System flags and variables
	std::string sys_message_head; /// Command line message header
	bool kill_all; /// Set to true when the program is exiting
	bool start_acq; /// Set to true when the command is given to start a run
	bool stop_acq; /// Set to true when the command is given to stop a run
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
	bool shm_mode; //
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

	size_t n_cards;

	typedef std::pair<unsigned int, unsigned int> chanid_t;
	std::map<chanid_t, PixieInterface::Histogram> histoMap;

	StatsHandler *statsHandler;
	word_t threshWords;

	Poll();
	~Poll();
	
	bool initialize();
	
	void set_stat_handler(StatsHandler *handler){ statsHandler = handler; }

	bool close();
		
	/// Print help dialogue for POLL options.
	void help();

	/// Print help dialogue for reading/writing pixie channel parameters.
	void pchan_help();

	/// Print help dialogue for reading/writing pixie module parameters.
	void pmod_help();
		
	void command_control();
		
	void run_control();
	
	std::string get_filename();
	
	bool close_output_file(bool continueRun = false);
	
	///Open a new output file.
	bool open_output_file(bool continueRun=false);
	
	bool synch_mods();
		
	int write_data(word_t *data, unsigned int nWords);
	void broadcast_data(word_t *data, unsigned int nWords);

	///Set the terminal pointer.
	void set_terminal(Terminal *term) {poll_term_ = term;};
};

// Function forward definitions
std::string humanReadable(double size);
unsigned int split_str(std::string str_, std::vector<std::string> &args, char delimiter_=' ');
std::string pad_string(const std::string &input_, unsigned int length_);
std::string yesno(bool value_);

#endif
