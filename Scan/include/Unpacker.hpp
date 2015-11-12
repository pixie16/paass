#ifndef UNPACKER_HPP
#define UNPACKER_HPP

#include <deque>
#include <vector>
#include <string>

#ifndef MAX_PIXIE_MOD
#define MAX_PIXIE_MOD 12
#endif
#ifndef MAX_PIXIE_CHAN
#define MAX_PIXIE_CHAN 15
#endif

class ChannelEvent;

class TFile;
class TTree;

class Unpacker{
  protected:
	unsigned int TOTALREAD; /// Maximum number of data words to read.
	unsigned int maxWords; /// Maximum number of data words for revision D.
	
	unsigned int event_width; /// The width of the raw event in pixie clock ticks (8 ns).
	
	unsigned int channel_counts[MAX_PIXIE_MOD+1][MAX_PIXIE_CHAN+1]; /// Counters for each channel in each module.
	
	bool kill_all; /// Set to true if kill all signal received from ScanMain.
	bool debug_mode; /// True if debug mode is set.
	bool shm_mode; /// Set to true if shared memory mode is to be used.
	bool init; /// True if the class has been properly initialized.

	std::deque<ChannelEvent*> eventList; /// The list of all events in the spill.
	std::deque<ChannelEvent*> rawEvent; /// The list of all events in the event window.

	TFile *root_file;
	TTree *root_tree;
	
	std::string message_head; /// Prefix used for text output.

	/** Clear all events in the raw event. WARNING! This method will delete all events in the
	 * event list. This could cause seg faults if the events are used elsewhere.
	 */	
	void ClearRawEvent();

	/** Clear all events in the spill event list. WARNING! This method will delete all events in the
	 * event list. This could cause seg faults if the events are used elsewhere.
	 */	
	void ClearEventList();
	
	/** Delete an event off the front of the event list. WARNING! This method will delete all events
	 * in the event list. This could cause seg faults if the events are used elsewhere.
	 */	
	void DeleteCurrentEvent();

	/** Process all events in the event list. This method will do nothing
	 *  unless it is overloaded by a derived class.
	 */
	virtual void ProcessRawEvent();
	
	/** Scan the time sorted event list and package the events into a raw
	 * event with a size governed by the event width.
	 */
	void ScanList();
	
	/// Scan the event list and sort it by timestamp.
	void SortList();
	
	/** Called form ReadSpill. Scan the current spill and construct a list of
	 * events which fired by obtaining the module, channel, trace, etc. of the
	 * timestamped event. This method will construct the event list for
	 * later processing.
	 */	
	int ReadBuffer(unsigned int *buf, unsigned long &bufLen);
	
  public:
  	/// Default constructor.
	Unpacker();
	
	/// Destructor.
	virtual ~Unpacker();

	/** Initialize the Unpacker object. Does nothing useful if not overloaded
	 * by a derived class.
	 */
	virtual bool Initialize(std::string prefix_="");
	
	/** Initialize the root output. Does nothing useful if not overloaded
	 * by a derived class.
	 */
	virtual bool InitRootOutput(std::string fname_, bool overwrite_=true){ return false; }

	/// Return true if Unpacker was properly initialized.
	bool IsInit(){ return init; }

	/// Perform tasks when waiting for a spill.
	virtual void IdleTask() {};

	/// Toggle debug mode on / off.
	bool SetDebugMode(bool state_=true){ return (debug_mode = state_); }
	
	/// Toggle shared memory mode on/off.
	bool SetSharedMemMode(bool state_=true){ return (shm_mode = state_); }

	/// Scan has stopped data acquisition.
	virtual void StopAcquisition(){  }
	
	/// Scan has started data acquisition.
	virtual void StartAcquisition(){  }
	
	/// Set the kill flag to true. This should be used to exit gracefully.
	void KillAll(){ kill_all = true; }

	/// Set the width of events in pixie16 clock ticks.
	unsigned int SetEventWidth(unsigned int width_){ return (event_width = width_); }
	
	void SetMsgPrefix(std::string prefix_){ message_head = prefix_; }
	
	/** ReadSpill is responsible for constructing a list of pixie16 events from
	 * a raw data spill. This method performs sanity checks on the spill and
	 * calls ReadBuffer in order to construct the event list.
	 */	
	bool ReadSpill(unsigned int *data, unsigned int nWords, bool is_verbose=true);
	
	/// Return the syntax string for this program.
	virtual void SyntaxStr(const char *name_, std::string prefix_=""){ std::cout << prefix_ << "SYNTAX: " << std::string(name_) << " <options> <input>\n"; }

	/// Print a command line help dialogue for recognized command line arguments.
	virtual void ArgHelp(std::string prefix_=""){}
	
	/// Print an in-terminal help dialogue for recognized commands.
	virtual void CmdHelp(std::string prefix_=""){}
	
	/// Scan input arguments and set class variables.
	virtual bool SetArgs(std::deque<std::string> &args_, std::string &filename_){ return true; }

	/// Print a status message.
	virtual void PrintStatus(std::string prefix_=""){}

	/** Search for an input command and perform the desired action. Return
	  * true if the command is valid and false otherwise.
	  */
	virtual bool CommandControl(std::string cmd_, const std::vector<std::string> &args_){ return false; }

	/// Empty the raw event and the event list.
	void Close();
};

extern Unpacker *GetCore(); /// External function which returns a pointer to a class derived from Unpacker.

#endif
