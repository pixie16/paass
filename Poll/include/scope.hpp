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
	int mod; /// The module of the signal of interest.
	int chan; /// The channel of the signal of interest.
	
	int delay; /// The number of seconds to wait between drawing traces.
  
	time_t last_trace; /// The time of the last trace.
  
	unsigned int num_traces; /// The total number of traces.
	
	unsigned int num_displayed; /// The number of displayed traces.
	
	std::vector<int> x_vals; /// The x-axis values of the trace.

	TApplication* rootapp;

	TCanvas *canvas; /// The main plotting canvas.
	
	TGraph *graph; /// The TGraph for plotting traces.

	/// Plot the current event.
	void Plot(ChannelEvent *event_);
  
	/// Process all events in the event list.
	void ProcessRawEvent();
	
  public:
	Oscilloscope();
	
	Oscilloscope(int mod_, int chan_);
	
	~Oscilloscope();
	
	bool Initialize(std::string prefix_="");

	int GetMod(){ return mod; }
	
	int GetChan(){ return chan; }
	
	int GetDelay(){ return delay; }
	
	void SetMod(int mod_){ mod = mod_; }
	
	void SetChan(int chan_){ chan = chan_; }

	/// Set the number of seconds to wait between drawing of traces.
	void SetDelay(int delay_){ delay = (delay_>=1)?delay_:1; }

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

	/** Search for an input command and perform the desired action. Return
	  * true if the command is valid and false otherwise.
	  */
	bool CommandControl(std::string cmd_, const std::vector<std::string> &args_);
};

/// Return a pointer to a new Oscilloscope object.
Unpacker *GetCore(){ return (Unpacker*)(new Oscilloscope()); }

#endif
