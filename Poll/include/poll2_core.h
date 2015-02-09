#ifndef POLL2_CORE_H
#define POLL2_CORE_H

#include <queue>
#include <termios.h>

#include "PixieInterface.h"
#include "hribf_buffers.h"

#define maxEventSize 4095 // (0x1FFE0000 >> 17)
#define POLL_VERSION 0.1

typedef PixieInterface::word_t word_t;
typedef word_t eventdata_t[maxEventSize];

extern PixieInterface pif;

class StatsHandler;

class Poll{
  public:
	struct termios SAVED_ATTRIBUTES;
	struct tm *TIME_INFO;
  
	// Maximum size of the shared memory buffer
	unsigned int MAX_SHM_SIZEL; // In pixie words
	unsigned int MAX_SHM_SIZE; // In bytes
	word_t CLOCK_VSN;	

	// System flags and variables
	std::string SYS_MESSAGE_HEAD; // Command line message header
	bool KILL_ALL; // Set to true when the program is exiting
	bool START_RUN; // Set to true when the command is given to start a run
	bool STOP_RUN; // Set to true when the command is given to stop a run
	bool DO_REBOOT; // Set to true when the user tells POLL to reboot PIXIE
	bool FORCE_SPILL; // Force poll2 to dump the current data spill
	bool POLL_RUNNING; // Set to true when run_command is recieving data from PIXIE
	bool RUN_CTRL_EXIT; // Set to true when run_command exits
	time_t RAW_TIME;

	// Run control variables
	bool BOOT_FAST;
	bool INSERT_WALL_CLOCK;
	bool IS_QUIET;
	bool SEND_ALARM;
	bool SHOW_MODULE_RATES;
	bool ZERO_CLOCKS;
	bool DEBUG_MODE;
	bool SHM_MODE;

	// Options relating to output data file
	std::string OUTPUT_DIRECTORY; // Set with 'fdir' command
	std::string OUTPUT_FILENAME; // Set with 'ouf' command
	std::string OUTPUT_TITLE; // Set with 'htit' command
	int OUTPUT_RUN_NUM; // Set with 'hnum' command
	int OUTPUT_FORMAT; // Set with 'oform' command

	// The main output data file and related variables
	unsigned int CURRENT_FILE_NUM;
	std::string CURRENT_FILENAME;
	PollOutputFile OUTPUT_FILE;
	
	int STATS_INTERVAL; //< in seconds
	int HISTO_INTERVAL; //< in seconds
	size_t N_CARDS;

	// Poll run variables
	double startTime;
	double spillTime, lastSpillTime, durSpill;
	double parseTime, waitTime, readTime, sendTime, pollTime;
	double lastStatsTime, statsTime;
	double lastHistoTime, histoTime;
	size_t dataWords;
	bool *runDone, isExiting, justEnded;
	unsigned int waitCounter, nonWaitCounter;
	unsigned int partialBufferCounter;
	time_t pollClock; 
	PixieInterface::Histogram deltaHisto;
	
	typedef std::pair<unsigned int, unsigned int> chanid_t;
	std::map<chanid_t, PixieInterface::Histogram> histoMap;

	// Data variables
	word_t *fifoData;
	eventdata_t *partialEventData;
	std::vector<word_t> partialEventWords;
	std::vector<word_t> waitWords;
	StatsHandler *statsHandler;
	word_t threshWords;

	Poll();
	~Poll();
	
	void set_stat_handler(StatsHandler *handler){ statsHandler = handler; }
	
	bool initialize();
	
	void restore_terminal();
	
	bool takeover_terminal();
	
	void command_control();
	
	void run_control();
	
	std::string get_filename();
	
	bool close_output_file();
	
	bool open_new_file();
	
	void help();
	
	bool synch_mods(PixieInterface &pif);
		
	int write_data(word_t *data, unsigned int nWords, bool shm_=false);
};

struct CLoption{
	char opt;
	std::string alias;
	std::string value;
	bool require_arg;
	bool optional_arg;
	bool is_active;
	
	CLoption(){
		Set("NULL", false, false);
		opt = 0x0;
		value = "";
		is_active = false;
	}
	
	CLoption(std::string name_, bool require_arg_, bool optional_arg_){
		Set(name_, require_arg_, optional_arg_);
		value = "";
		is_active = false;
	}
	
	void Set(std::string name_, bool require_arg_, bool optional_arg_){
		opt = name_[0]; alias = name_; require_arg = require_arg_; optional_arg = optional_arg_;
	}
};

// Function forward definitions
bool get_opt(int argc_, char **argv_, CLoption *options, unsigned int num_valid_opt_);
unsigned int cstrlen(char *str_);
std::string csubstr(char *str_, unsigned int start_index=0);
std::string pad_string(const std::string &input_, unsigned int length_);
std::string yesno(bool value_);

#endif
