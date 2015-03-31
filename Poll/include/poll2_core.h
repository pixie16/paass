#ifndef POLL2_CORE_H
#define POLL2_CORE_H

#include <vector>

#include "PixieInterface.h"
#include "hribf_buffers.h"
#include "CTerminal.h"

#define maxEventSize 4095 // (0x1FFE0000 >> 17)
#define POLL_VERSION "1.1.05"

typedef PixieInterface::word_t word_t;
typedef word_t eventdata_t[maxEventSize];

class StatsHandler;

class Poll{
  public:
  	struct tm *TIME_INFO;
 
	PixieInterface *pif; // The main pixie interface pointer 
  
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
	bool init;

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
	
	bool initialize();
	
	void set_stat_handler(StatsHandler *handler){ statsHandler = handler; }

	bool close();
		
	/// Print help dialogue for POLL options.
	void help();

	/// Print help dialogue for reading/writing pixie channel parameters.
	void pchan_help();

	/// Print help dialogue for reading/writing pixie module parameters.
	void pmod_help();
		
	void command_control(Terminal *poll_term_);
		
	void run_control();
	
	std::string get_filename();
	
	bool close_output_file();
	
	bool open_new_file();
	
	bool synch_mods();
		
	int write_data(word_t *data, unsigned int nWords, bool shm_=false);
};

template<typename T=int>
struct PixieFunctionParms{
    PixieInterface &pif;
    unsigned int mod;
    unsigned int ch;
    T par;
  
    PixieFunctionParms(PixieInterface &p, T x) : pif(p) {par=x;}
};

template<typename T=int>
class PixieFunction : public std::unary_function<bool, struct PixieFunctionParms<T> >{
    public:
    virtual bool operator()(struct PixieFunctionParms<T> &par) = 0;
    virtual ~PixieFunction() {};
};

class ParameterChannelWriter : public PixieFunction< std::pair<std::string, float> >{
  public:
	bool operator()(PixieFunctionParms< std::pair<std::string, float> > &par);
};

class ParameterModuleWriter : public PixieFunction< std::pair<std::string, unsigned long> >{
  public:
	bool operator()(PixieFunctionParms< std::pair<std::string, unsigned long> > &par);
};

class ParameterChannelReader : public PixieFunction<std::string>{
  public:
	bool operator()(PixieFunctionParms<std::string> &par);
};

class ParameterModuleReader : public PixieFunction<std::string>{
  public:
	bool operator()(PixieFunctionParms<std::string> &par);
};

// Function forward definitions
unsigned int split_str(std::string str_, std::vector<std::string> &args, char delimiter_=' ');
std::string pad_string(const std::string &input_, unsigned int length_);
std::string yesno(bool value_);

template<typename T>
bool forModule(PixieInterface *pif, int mod, PixieFunction<T> &f, T par = T() );

template<typename T>
bool forChannel(PixieInterface *pif, int mod, int ch, PixieFunction<T> &f, T par = T() );

#endif
