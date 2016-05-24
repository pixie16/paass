/** \file Unpacker.hpp
 * \brief A class to handle the unpacking of UTK/ORNL style pixie16 data spills.
 *
 * This class is intended to be used as a replacement of pixiestd.cpp from Stan
 * Paulauskas's pixie_scan. The majority of function names and arguments are
 * preserved as much as possible while allowing for more standardized unpacking
 * of pixie16 data.
 * CRT
 *
 * \author C. R. Thornsberry
 * \date Feb. 12th, 2016
 */
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

class XiaEvent;
class ScanMain;

class eventList{
  public:
	eventList();

	/// Return the maximum module read from the input file.
	size_t GetMaxModule(){ return list.size(); }

	/// Scan the event list and sort it by timestamp.
	void TimeSort();

	/** Scan the time sorted event list and package the events into a raw
	 * event with a size governed by the event width.
	 */
	bool BuildRawEvent(std::deque<XiaEvent*> &rawEvt, const double &eventWidth_);
	
	/// Push an event into the event list.
	bool AddEvent(XiaEvent *event_);
	
	/** Clear all events in the spill event list. WARNING! This method will delete all events in the
	 * event list. This could cause seg faults if the events are used elsewhere.
	 */	
	void Clear();

  private:
	std::vector<std::deque<XiaEvent*> > list; /// The list of events.
};

class Unpacker{
  protected:
	unsigned int TOTALREAD; /// Maximum number of data words to read.
	unsigned int maxWords; /// Maximum number of data words for revision D.
	
	unsigned int event_width; /// The width of the raw event in pixie clock ticks (8 ns).
	
	unsigned int channel_counts[MAX_PIXIE_MOD+1][MAX_PIXIE_CHAN+1]; /// Counters for each channel in each module.
	
	bool debug_mode; /// True if debug mode is set.
	bool init; /// True if the class has been properly initialized.

	eventList events; /// The list of all events in a spill.
	std::deque<XiaEvent*> rawEvent; /// The list of all events in the event window.

	std::string message_head; /// Prefix used for text output.

	/** Process all events in the event list. This method will do nothing
	 *  unless it is overloaded by a derived class.
	 */
	virtual void ProcessRawEvent();
	
	/** Called form ReadSpill. Scan the current spill and construct a list of
	 * events which fired by obtaining the module, channel, trace, etc. of the
	 * timestamped event. This method will construct the event list for
	 * later processing.
	 */	
	int ReadBuffer(unsigned int *buf, unsigned long &bufLen);
	
  	/// Default constructor.
	Unpacker();
	
  public:
	/// Destructor.
	virtual ~Unpacker();

	/** Initialize the Unpacker object. Does nothing useful if not overloaded
	 * by a derived class.
	 */
	virtual bool Initialize(std::string prefix_="");
	
	/** Peform any last minute initialization which derived classes need to run
	  * before processing data.
	  */
	virtual void FinalInitialization(){ }
	
	/** Initialize the root output. Does nothing useful if not overloaded
	 * by a derived class.
	 */
	virtual bool InitRootOutput(std::string fname_, bool overwrite_=true){ return false; }

	/// Return true if Unpacker was properly initialized.
	bool IsInit(){ return init; }

	/// Toggle debug mode on / off.
	bool SetDebugMode(bool state_=true){ return (debug_mode = state_); }
	
	/// Set the width of events in pixie16 clock ticks.
	unsigned int SetEventWidth(unsigned int width_){ return (event_width = width_); }
	
	void SetMsgPrefix(std::string prefix_){ message_head = prefix_; }
	
	/** ReadSpill is responsible for constructing a list of pixie16 events from
	 * a raw data spill. This method performs sanity checks on the spill and
	 * calls ReadBuffer in order to construct the event list.
	 */	
	bool ReadSpill(unsigned int *data, unsigned int nWords, bool is_verbose=true);
	
	/// Print a status message.
	virtual void PrintStatus(std::string prefix_=""){}

	/// Empty the raw event and the event list.
	void Close(bool write_count_file=false);
};

extern Unpacker *GetCore(); /// External function which returns a pointer to a class derived from Unpacker.

#endif
