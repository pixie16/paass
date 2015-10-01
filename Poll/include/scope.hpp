#ifndef OSCILLOSCOPE_HPP
#define OSCILLOSCOPE_HPP

#include "Unpacker.hpp"

class ChannelEvent;

class Oscilloscope : public Unpacker{
  private:
	int mod; /// The module of the signal of interest.
	int chan; /// The channel of the signal of interest.
  
	unsigned int num_traces; /// The number of displayed traces.
  
	/// Plot the current event.
	void Plot(ChannelEvent *event_);
  
	/// Process all events in the event list.
	void ProcessRawEvent();
	
  public:
	Oscilloscope();
	
	Oscilloscope(int mod_, int chan_);
	
	bool Initialize(std::string prefix_="");

	int GetMod(){ return mod; }
	
	int GetChan(){ return chan; }
	
	void SetMod(int mod_){ mod = mod_; }
	
	void SetChan(int chan_){ chan = chan_; }

	/// Return the syntax string for this program.
	void SyntaxStr(const char *name_, std::string prefix_=""){ std::cout << prefix_ << "SYNTAX: " << std::string(name_) << " <options> <input>\n"; }
	
	/// Print a help dialogue. 
	void Help(std::string prefix_="");
	
	/// Scan input arguments and set class variables.
	bool SetArgs(std::deque<std::string> &args_, std::string &filename_);
	
	void PrintStatus(std::string prefix_=""){ std::cout << prefix_ << "Found and displayed " << num_traces << " traces.\n"; }
};

/// Return a pointer to a new Oscilloscope object.
Unpacker *GetCore(){ return (Unpacker*)(new Oscilloscope()); }

#endif
