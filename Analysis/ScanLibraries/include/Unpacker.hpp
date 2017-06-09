/** \file Unpacker.hpp
 * \brief A class to handle the unpacking of UTK/ORNL style pixie16 data spills.
 *
 * This class is intended to be used as a replacement of pixiestd.cpp from Stan
 * Paulauskas's pixie_scan. The majority of function names and arguments are
 * preserved as much as possible while allowing for more standardized unpacking
 * of pixie16 data.
 * CRT
 *
 * \author C. R. Thornsberry, S. V. Paulauskas
 * \date Feb. 12th, 2016
 */
#ifndef UNPACKER_HPP
#define UNPACKER_HPP

#include <deque>
#include <map>
#include <string>
#include <vector>

#include "XiaListModeDataMask.hpp"

#ifndef MAX_PIXIE_MOD
#define MAX_PIXIE_MOD 12
#endif
#ifndef MAX_PIXIE_CHAN
#define MAX_PIXIE_CHAN 15
#endif

class XiaData;

class ScanMain;

class Unpacker {
public:
    /// Default constructor.
    Unpacker();

    /// Destructor.
    virtual ~Unpacker();

    ///@return the maximum module read from the input file. The calculation on this cannot be right.
    unsigned int GetMaxModuleInFile() { return maxModuleNumberInFile_; }

    /// Return the number of raw events read from the file.
    unsigned int GetNumRawEvents() { return numRawEvt; }

    /// Return the width of the raw event window in pixie16 clock ticks.
    double GetEventWidth() { return eventWidth_; }

    /// Return the time of the first fired channel event.
    double GetFirstTime() { return firstTime; }

    /// Get the start time of the current raw event.
    double GetEventStartTime() { return eventStartTime; }

    /// Get the stop time of the current raw event.
    double GetEventStopTime() { return eventStartTime + eventWidth_; }

    /// Get the time of the first xia event in the raw event.
    double GetRealStartTime() { return realStartTime; }

    /// Get the time of the last xia event in the raw event.
    double GetRealStopTime() { return realStopTime; }

    /// Return true if the scan is running and false otherwise.
    bool IsRunning() { return running; }

    /// Toggle debug mode on / off.
    bool SetDebugMode(bool state_ = true) { return (debug_mode = state_); }

    /// Set the width of events in pixie16 clock ticks.
    void SetEventWidth(double width) { eventWidth_ = width; }

    void InitializeDataMask(const std::string &firmware, const unsigned int &frequency = 0);

    /** ReadSpill is responsible for constructing a list of pixie16 events from
      * a raw data spill. This method performs sanity checks on the spill and
      * calls ReadBuffer in order to construct the event list.
      * \param[in]  data       Pointer to an array of unsigned ints containing the spill data.
      * \param[in]  nWords     The number of words in the array.
      * \param[in]  is_verbose Toggle the verbosity flag on/off.
      * \return True if the spill was read successfully and false otherwise.
      */
    bool ReadSpill(unsigned int *data, unsigned int nWords, bool is_verbose = true);

    /** Write all recorded channel counts to a file.
      * \return Nothing.
      */
    void Write();

    /** Stop the scan. Unused by default.
      * \return Nothing.
      */
    void Stop() { running = false; }

    /** Run the scan. Unused by default.
      * \return Nothing.
      */
    void Run() { running = true; }

protected:
    bool debug_mode; ///< True if debug mode is set.
    std::vector<std::deque<XiaData *>> eventList; ///< The list of all events in a spill.
    double eventWidth_; ///< The width of the raw event in pixie clock ticks
    XiaListModeDataMask mask_; ///< Object providing the masks necessary to decode the data.
    std::map<unsigned int, std::pair<std::string, unsigned int> > maskMap_;///< Maps firmware/frequency to module number
    unsigned int maxModuleNumberInFile_; ///< The maximum module number that we've encountered in the data file.
    std::deque<XiaData *> rawEvent; ///< The list of all events in the event window.
    bool running; ///< True if the scan is running.

    /** Process all events in the event list.
      * \param[in]  addr_ Pointer to a ScanInterface object. Unused by default.
      * \return Nothing.
      */
    virtual void ProcessRawEvent();

    /** Add an event to generic statistics output.
      * \param[in]  event_ Pointer to the current XIA event. Unused by default.
      * \param[in]  addr_  Pointer to a ScanInterface object. Unused by default.
      * \return Nothing.
      */
    virtual void RawStats(XiaData *event_) {}

    /** Called form ReadSpill. Scan the current spill and construct a list of
      * events which fired by obtaining the module, channel, trace, etc. of the
      * timestamped event. This method will construct the event list for
      * later processing.
      * \param[in]  buf    Pointer to an array of unsigned ints containing raw buffer data.
      * \param[out] bufLen The number of words in the buffer.
      * \return The number of XiaDatas read from the buffer.
      */
    int ReadBuffer(unsigned int *buf, const unsigned int &vsn);

private:
    unsigned int TOTALREAD; /// Maximum number of data words to read.
    unsigned int maxWords; /// Maximum number of data words for revision D.
    unsigned int numRawEvt; /// The total count of raw events read from file.

    unsigned int channel_counts[MAX_PIXIE_MOD + 1][MAX_PIXIE_CHAN + 1]; /// Counters for each channel in each module.

    double firstTime; /// The first recorded event time.
    double eventStartTime; /// The start time of the current raw event.
    double realStartTime; /// The time of the first xia event in the raw event.
    double realStopTime; /// The time of the last xia event in the raw event.

    /** Scan the event list and sort it by timestamp.
      * \return Nothing.
      */
    void TimeSort();

    /** Scan the time sorted event list and package the events into a raw
      * event with a size governed by the event width.
      * \return True if the event list is not empty and false otherwise.
      */
    bool BuildRawEvent();

    /** Push an event into the event list.
      * \param[in]  event_ The XiaData to push onto the back of the event list.
      * \return True if the XiaData's module number is valid and false otherwise.
      */
    bool AddEvent(XiaData *event_);

    /** Clear all events in the spill event list. WARNING! This method will delete all events in the
      * event list. This could cause seg faults if the events are used elsewhere.
      * \return Nothing.
      */
    void ClearEventList();

    /** Clear all events in the raw event list. WARNING! This method will delete all events in the
      * event list. This could cause seg faults if the events are used elsewhere.
      * \return Nothing.
      */
    void ClearRawEvent();

    /** Get the minimum channel time from the event list.
      * \param[out] time The minimum time from the event list in system clock ticks.
      * \return True if the event list is not empty and false otherwise.
      */
    bool GetFirstTime(double &time);

    /** Check whether or not the eventList is empty.
      * \return True if the eventList is empty, and false otherwise.
      */
    bool IsEmpty();
};

#endif
