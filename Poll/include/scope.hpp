#ifndef OSCILLOSCOPE_HPP
#define OSCILLOSCOPE_HPP

#include <ctime>
#include <vector>

#include "Unpacker.hpp"

class ChannelEvent;
class TApplication;
class TCanvas;
class TGraph;

class Oscilloscope : public Unpacker{
  private:
	int mod_; ///< The module of the signal of interest.
	int chan_; ///< The channel of the signal of interest.
	
	bool need_graph_update; /// Set to true if the graph range needs updated.
	
	int delay_; /// The number of seconds to wait between drawing traces.
  
	std::vector<int> x_vals;
	
	bool resetGraph_;

	time_t last_trace; ///< The time of the last trace.
  
	unsigned int num_traces; ///< The total number of traces.
	
	unsigned int num_displayed; ///< The number of displayed traces.
	
	std::string saveFile_; ///< The name of the file to save a trace.

	TApplication* rootapp;

	TCanvas *canvas; ///< The main plotting canvas.
	
	TGraph *graph; ///< The TGraph for plotting traces.

	void ResetGraph(int size_);
	
	/// Plot the current event.
	void Plot(ChannelEvent *event_);
  
	/// Process all events in the event list.
	void ProcessRawEvent();
	
  public:
	Oscilloscope(int mod = 0, int chan = 0);
	
	~Oscilloscope();
	
	bool Initialize(std::string prefix_="");

	int GetMod(){ return mod_; }
	
	int GetChan(){ return chan_; }
	
	int GetDelay(){ return delay_; }
	
	void SetMod(int mod){ mod_ = mod; }
	
	void SetChan(int chan){ chan_ = chan; }

	/// Set the number of seconds to wait between drawing of traces.
	void SetDelay(int delay){ delay_ = (delay>1)?delay:1; }

	/// Return the syntax string for this program.
	void SyntaxStr(const char *name_, std::string prefix_=""){ std::cout << prefix_ << "SYNTAX: " << std::string(name_) << " <options> <input>\n"; }
	
	/// Print a command line help dialogue for recognized command line arguments.
	void ArgHelp(std::string prefix_="");
	
	/// Print an in-terminal help dialogue for recognized commands.
	void CmdHelp(std::string prefix_="");
	
	/// Scan input arguments and set class variables.
	bool SetArgs(std::deque<std::string> &args_, std::string &filename_);

	/// Print a status message.	
	void PrintStatus(std::string prefix_=""){ std::cout << prefix_ << "Found " << num_traces << " traces and displayed " << num_displayed << ".\n"; }

	/** Search for an input command and perform the desired action.
	  * 
	  * \return True if the command is valid and false otherwise.
	  */
	bool CommandControl(std::string cmd_, const std::vector<std::string> &args_);

	/// Perform tasks when waiting for a spill.
	virtual void IdleTask();
};

/// Return a pointer to a new Oscilloscope object.
Unpacker *GetCore(){ return (Unpacker*)(new Oscilloscope()); }

#endif
