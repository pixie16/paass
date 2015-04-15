#ifndef POLL2_CORE_H
#define POLL2_CORE_H

#include <vector>

#include "PixieInterface.h"
#include "hribf_buffers.h"
#include "CTerminal.h"

#define maxEventSize 4095 // (0x1FFE0000 >> 17)
#define POLL_VERSION "1.1.08"

typedef PixieInterface::word_t word_t;
typedef word_t eventdata_t[maxEventSize];

class StatsHandler;

class Poll{
  public:
  	struct tm *time_info;
 
	PixieInterface *pif; // The main pixie interface pointer 
  
	word_t clock_vsn;	

	// System flags and variables
	std::string sys_message_head; // Command line message header
	bool kill_all; // Set to true when the program is exiting
	bool start_run; // Set to true when the command is given to start a run
	bool stop_run; // Set to true when the command is given to stop a run
	bool do_reboot; // Set to true when the user tells POLL to reboot PIXIE
	bool force_spill; // Force poll2 to dump the current data spill
	bool poll_running; // Set to true when run_command is recieving data from PIXIE
	bool run_ctrl_exit; // Set to true when run_command exits
	time_t raw_time;

	// Run control variables
	bool boot_fast; //
	bool insert_wall_clock; //
	bool is_quiet; //
	bool send_alarm; //
	bool show_module_rates; //
	bool zero_clocks; //
	bool debug_mode; //
	bool init; //

	// Options relating to output data file
	std::string output_directory; // Set with 'fdir' command
	std::string output_filename; // Set with 'ouf' command
	std::string output_title; // Set with 'htit' command
	int output_run_num; // Set with 'hnum' command
	int output_format; // Set with 'oform' command

	// The main output data file and related variables
	int current_file_num;
	std::string current_filename;
	PollOutputFile output_file;

	size_t n_cards;

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
	
	int histo_interval, stats_interval;
	
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
		
	int write_data(word_t *data, unsigned int nWords);
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

class OffsetAdjuster : public PixieFunction<int>{
  public:
	bool operator()(PixieFunctionParms<int> &par);
};

class TauFinder : public PixieFunction<>{
  public:
	bool operator()(PixieFunctionParms<> &par);
};

void CSRA_test(int input_);

// Function forward definitions
unsigned int split_str(std::string str_, std::vector<std::string> &args, char delimiter_=' ');
std::string pad_string(const std::string &input_, unsigned int length_);
std::string yesno(bool value_);

template<typename T>
bool forModule(PixieInterface *pif, int mod, PixieFunction<T> &f, T par = T() );

template<typename T>
bool forChannel(PixieInterface *pif, int mod, int ch, PixieFunction<T> &f, T par = T() );

#endif
