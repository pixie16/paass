#include <string>

#include "hribf_buffers.h"

#define SCAN_VERSION "1.2.02"
#define SCAN_DATE "Nov. 5th, 2015"

class Server;
class Terminal;
class Unpacker;

class ScanMain{
  private:
	unsigned int maxShmSizeL; /// Max size of shared memory buffer in pixie words (4050 + 2 header words)
	unsigned int maxShmSize; /// Max size of shared memory buffer in bytes

	std::string prefix; /// Input filename prefix (without extension).
	std::string extension; /// Input file extension.

	int max_spill_size; /// Maximum size of a spill to read.
	int file_format; /// Input file format to use (0=.ldf, 1=.pld, 2=.root).
	
	unsigned long num_spills_recvd; /// The total number of good spills received from either the input file or shared memory.

	bool is_running; /// Set to true if the acqusition is running.
	bool is_verbose; /// Set to true if the user wishes verbose information to be displayed.
	bool debug_mode; /// Set to true if the user wishes to display debug information.
	bool dry_run_mode; /// Set to true if a dry run is to be performed i.e. data is to be read but not processed.
	bool shm_mode; /// Set to true if shared memory mode is to be used.

	bool kill_all; /// Set to true when user has sent kill command.
	bool run_ctrl_exit; /// Set to true when run control thread has exited.

	Server *poll_server; /// Poll2 shared memory server.

	std::ifstream input_file; /// Main input binary data file.
	std::streampos file_length; /// Main input file length (in bytes).

	PLD_header pldHead; /// PLD style HEAD buffer handler.
	PLD_data pldData; /// PLD style DATA buffer handler.
	DIR_buffer dirbuff; /// HRIBF DIR buffer handler.
	HEAD_buffer headbuff; /// HRIBF HEAD buffer handler.
	DATA_buffer databuff; /// HRIBF DATA buffer handler.
	EOF_buffer eofbuff; /// HRIBF EOF buffer handler.

	Terminal *term; /// ncurses terminal used for displaying output and handling user input.

	Unpacker *core; /// Pointer to class derived from Unpacker class.
  
	std::string sys_message_head; /// The string to print before program output.
  
	unsigned int split_str(std::string str_, std::vector<std::string> &args, char delimiter_=' ');
	
	std::string get_extension(std::string filename_, std::string &prefix);

  public:
	ScanMain(Unpacker *core_=NULL);

	~ScanMain();
	
	bool IsVerbose(){ return is_verbose; }
	
	bool DebugMode(){ return debug_mode; }
	
	bool DryRunMode(){ return dry_run_mode; }
	
	bool ShmMode(){ return shm_mode; }
	
	std::string GetMessageHeader(){ return sys_message_head; }
	
	void SetMessageHeader(const std::string &head_){ sys_message_head = head_; }
	
	bool SetVerboseMode(bool state_=true){ return (is_verbose = state_); }
	
	bool SetDebugMode(bool state_=true){ return (debug_mode = state_); }
	
	bool SetDryRunMode(bool state_=true){ return (dry_run_mode = state_); }
	
	bool SetShmMode(bool state_=true){ return (shm_mode = state_); }
	
	void RunControl();
	
	void CmdControl();
	
	void Help(char *name_, Unpacker *core_);
	
	int Execute(int argc, char *argv[]);
};
