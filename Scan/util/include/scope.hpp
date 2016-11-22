#ifndef OSCILLOSCOPE_HPP
#define OSCILLOSCOPE_HPP

#include <ctime>
#include <vector>
#include <deque>
#include <cmath>
#include <string>

#include "Unpacker.hpp"

#include "RootScanner.hpp"

class ChannelEvent;
class TGraph;
class TH2F;
class TF1;
class TLine;
class TProfile;

///////////////////////////////////////////////////////////////////////////////
// class scopeUnpacker
///////////////////////////////////////////////////////////////////////////////

class scopeUnpacker : public Unpacker {
  public:
  	/// Default constructor.
	scopeUnpacker(const unsigned int &mod=0, const unsigned int &chan=0);
	
	/// Destructor.
	~scopeUnpacker(){  }

	int SetMod(const unsigned int &mod){ return(mod >= 0 ? (mod_ = mod) : (mod_ = 0)); }
	
	int SetChan(const unsigned int &chan){ return(chan >= 0 ? (chan_ = chan) : (chan_ = 0)); }
	
	void SetThreshLow(const int &threshLow){ threshLow_ = threshLow; }
	
	void SetThreshHigh(const int &threshHigh){ threshHigh_ = threshHigh; }

	unsigned int GetMod(){ return mod_; }
	
	unsigned int GetChan(){ return chan_; }
	
	int GetThreshLow(){ return threshLow_; }
	
	int GetThreshHigh(){ return threshHigh_; }

  private:
	unsigned int mod_; ///< The module of the signal of interest.
	unsigned int chan_; ///< The channel of the signal of interest.
	int threshLow_;
	int threshHigh_;

	/** Process all events in the event list.
	  * \param[in]  addr_ Pointer to a ScanInterface object.
	  * \return Nothing.
	  */
	virtual void ProcessRawEvent(ScanInterface *addr_=NULL);
	
	/** Add an event to generic statistics output.
	  * \param[in]  event_ Pointer to the current XIA event.
	  * \param[in]  addr_  Pointer to a ScanInterface object.
	  * \return Nothing.
	  */
	virtual void RawStats(XiaData *event_, ScanInterface *addr_=NULL){  }
};

///////////////////////////////////////////////////////////////////////////////
// class scopeScanner
///////////////////////////////////////////////////////////////////////////////

class scopeScanner : public RootScanner {
  public:
	/// Default constructor.
	scopeScanner(int mod = 0, int chan = 0);
	
	/// Destructor.
	~scopeScanner();
	
	/// Set the time delay between traces (in seconds).
	int GetDelay(){ return delay_; }
 	
 	/// Return the maximum number of events to store.
 	size_t GetNumEvents(){ return numEvents; }

	/// Set the number of seconds to wait between drawing of traces.
	void SetDelay(int delay){ delay_ = (delay>1)?delay:1; }

	/// Set the maximum number of events to store.
	void SetNumEvents(size_t num_){ numEvents = num_; }

	/// Stop the run.
	void StopACQ(){ running = false; }
	
	/// Star the run.
	void StartACQ(){ running = true; }

	/** ExtraCommands is used to send command strings to classes derived
	  * from ScanInterface. If ScanInterface receives an unrecognized
	  * command from the user, it will pass it on to the derived class.
	  * \param[in]  cmd_ The command to interpret.
	  * \param[out] arg_ Vector or arguments to the user command.
	  * \return True if the command was recognized and false otherwise.
	  */
	virtual bool ExtraCommands(const std::string &cmd_, std::vector<std::string> &args_);
	
	/** ExtraArguments is used to send command line arguments to classes derived
	  * from ScanInterface. This method should loop over the optionExt elements
	  * in the vector userOpts and check for those options which have been flagged
	  * as active by ::Setup(). This should be overloaded in the derived class.
	  * \return Nothing.
	  */
    virtual void ExtraArguments();
	
	/** CmdHelp is used to allow a derived class to print a help statement about
	  * its own commands. This method is called whenever the user enters 'help'
	  * or 'h' into the interactive terminal (if available).
	  * \param[in]  prefix_ String to append at the start of any output. Not used by default.
	  * \return Nothing.
	  */
	virtual void CmdHelp(const std::string &prefix_="");
	
	/** ArgHelp is used to allow a derived class to add a command line option
	  * to the main list of options. This method is called at the end of
	  * from the ::Setup method.
	  * Does nothing useful by default.
	  * \return Nothing.
	  */
	virtual void ArgHelp();
	
	/** SyntaxStr is used to print a linux style usage message to the screen.
	  * \param[in]  name_ The name of the program.
	  * \return Nothing.
	  */
	virtual void SyntaxStr(char *name_);

	/** Initialize the map file, the config file, the processor handler, 
	  * and add all of the required processors.
	  * \param[in]  prefix_ String to append to the beginning of system output.
	  * \return True upon successfully initializing and false otherwise.
	  */
	virtual bool Initialize(std::string prefix_="");
	
	/** Peform any last minute initialization before processing data. 
	  * /return Nothing.
	  */
	virtual void FinalInitialization(){  }
	
	/** Initialize the root output. 
	  * \param[in]  fname_     Filename of the output root file. 
	  * \param[in]  overwrite_ Set to true if the user wishes to overwrite the output file. 
	  * \return True upon successfully opening the output file and false otherwise. 
	  */
	virtual bool InitRootOutput(std::string fname_, bool overwrite_=true){ return false; }

	/** Receive various status notifications from the scan.
	  * \param[in] code_ The notification code passed from ScanInterface methods.
	  * \return Nothing.
	  */
	virtual void Notify(const std::string &code_="");

	/** Return a pointer to the Unpacker object to use for data unpacking.
	  * If no object has been initialized, create a new one.
	  * \return Pointer to an Unpacker object.
	  */
	virtual Unpacker *GetCore();

	/** Add a channel event to the deque of events to send to the processors.
	  * This method should only be called from skeletonUnpacker::ProcessRawEvent().
	  * \param[in]  event_ The raw XiaData to add to the channel event deque.
	  * \return True if events are ready to be processed and false otherwise.
	  */
	virtual bool AddEvent(XiaData *event_);
	
	/** Process all channel events read in from the rawEvent.
	  * This method should only be called from skeletonUnpacker::ProcessRawEvent().
	  * \return True if events were processed and false otherwise.
	  */
	virtual bool ProcessEvents();

	/** Clear the event deque.
	  * \return Nothing.
	  */
	void ClearEvents();

  private:
	unsigned int numAvgWaveforms_;
	unsigned int num_displayed; ///< The number of displayed traces.
	
	size_t numEvents; /// The number of waveforms to store.
	
	float cfdF_;
	int cfdD_;
	int cfdL_;
	int fitLow_;
	int fitHigh_;
	int delay_; /// The number of seconds to wait between drawing traces.
	
	bool need_graph_update; /// Set to true if the graph range needs updated.
	bool resetGraph_;
	bool acqRun_;
	bool singleCapture_;
	bool init;	
	bool running;
	bool performFit_;
	bool performCfd_;
  
	std::vector<int> x_vals;
	std::deque<ChannelEvent*> chanEvents_; ///<The buffer of waveforms to be plotted.

	time_t last_trace; ///< The time of the last trace.
	
	std::string saveFile_; ///< The name of the file to save a trace.

	TGraph *graph; ///< The TGraph for plotting traces.
	TLine *cfdLine;
	TF1 *cfdPol3;
	TF1 *cfdPol2;
	TH2F *hist; ///<The histogram containing the waveform frequencies.
	TProfile *prof; ///<The profile of the average histogram.

	TF1 *paulauskasFunc; ///< A TF1 of the Paulauskas Function (NIM A 737 (2014) 22)

	TF1 *SetupFunc();

	void ResetGraph(unsigned int size_);
	
	/// Plot the current event.
	void Plot();
};

#endif
