/** \file Unpacker.cpp
 * \brief A class to handle the unpacking of UTK/ORNL style pixie16 data spills.
 *
 * This class is intended to be used as a replacement of PixieStd.cpp from
 * pixie_scan. The majority of function names and arguments are
 * preserved as much as possible while allowing for more standardized unpacking
 * of pixie16 data.
 *
 * \author C. R. Thornsberry and S. V. Paulauskas
 * \date February 12, 2016
 */
#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>

#include <cstring>

#include "Unpacker.hpp"
#include "XiaData.hpp"
#include "XiaListModeDataDecoder.hpp"
#include "XmlInterface.hpp"

using namespace std;

void clearDeque(deque<XiaData *> &list) {
    while (!list.empty()) {
        delete list.front();
        list.pop_front();
    }
}

///Scan the event list and sort it by timestamp.
/// @return Nothing.
void Unpacker::TimeSort() {
    for (vector<deque<XiaData *> >::iterator iter = eventList.begin(); iter != eventList.end(); iter++)
        sort(iter->begin(), iter->end(), &XiaData::CompareTime);
}

/** Scan the time sorted event list and package the events into a raw
  * event with a size governed by the event width.
  * \return True if the event list is not empty and false otherwise.
  */
bool Unpacker::BuildRawEvent() {
    if (!rawEvent.empty())
        ClearRawEvent();

    if (numRawEvt == 0) {// This is the first rawEvent. Do some special processing.
        // Find the first XiaData event. The eventList is time sorted by module.
        // The first component of each deque will be the earliest time from that module.
        // The first event time will be the minimum of these first components.
        if (!GetFirstTime(firstTime))
            return false;
        std::cout << "BuildRawEvent: First event time is " << firstTime << " clock ticks.\n";
        eventStartTime = firstTime;
    } else {
        // Move the event window forward to the next valid channel fire.
        if (!GetFirstTime(eventStartTime))
            return false;
    }

    realStartTime = eventStartTime + eventWidth_;
    realStopTime = eventStartTime;

    unsigned int mod, chan;
    string type, subtype, tag;
    XiaData *current_event = NULL;

    // Loop over all  time-sorted modules.
    for (vector<deque<XiaData *> >::iterator iter = eventList.begin(); iter != eventList.end(); iter++) {
        if (iter->empty())
            continue;

        // Loop over the list of channels that fired in this buffer
        while (!iter->empty()) {
            current_event = iter->front();
            mod = current_event->GetModuleNumber();
            chan = current_event->GetChannelNumber();

            if (mod > MAX_PIXIE_MOD ||
                chan > MAX_PIXIE_CHAN) { // Skip this channel
                cout << "BuildRawEvent: Encountered non-physical Pixie ID (mod = "
                     << mod << ", chan = " << chan << ")\n";
                delete current_event;
                iter->pop_front();
                continue;
            }

            double currtime = current_event->GetTime();

            // Check for backwards time-skip. This is un-handled currently and needs fixed CRT!!!
            if (currtime < eventStartTime)
                cout << "BuildRawEvent: Detected backwards time-skip from start=" << eventStartTime << " to "
                     << current_event->GetTime() << "???\n";

            // If the time difference between the current and previous event is
            // larger than the event width, finalize the current event, otherwise
            // treat this as part of the current event
            if ((currtime - eventStartTime) > eventWidth_)
                break;

            // Check for the minimum time in this raw event.
            if (currtime < realStartTime)
                realStartTime = currtime;

            // Check for the maximum time in this raw event.
            if (currtime > realStopTime)
                realStopTime = currtime;

            // Update raw stats output with the new event before adding it to the raw event.
            RawStats(current_event);

            // Push this channel event into the rawEvent.
            rawEvent.push_back(current_event);

            // Remove this event from the event list but do not delete it yet.
            // Deleting of the channel events will be handled by clearing the rawEvent.
            iter->pop_front();
        }
    }

    numRawEvt++;

    return true;
}

/** Push an event into the event list.
  * \param[in]  event_ The XiaData to push onto the back of the event list.
  * \return True if the XiaData's module number is valid and false otherwise. */
bool Unpacker::AddEvent(XiaData *event_) {
    if (event_->GetModuleNumber() > MAX_PIXIE_MOD)
        return false;

    // Check for the need to add a new deque to the event list.
    if (event_->GetModuleNumber() + 1 > (unsigned int) eventList.size())
        while (eventList.size() < event_->GetModuleNumber() + 1)
            eventList.push_back(std::deque<XiaData *>());

    eventList.at(event_->GetModuleNumber()).push_back(event_);

    return true;
}

/** Clear all events in the spill event list. WARNING! This method will delete all events in the
  * event list. This could cause seg faults if the events are used elsewhere.
  * \return Nothing. */
void Unpacker::ClearEventList() {
    for (std::vector<std::deque<XiaData *> >::iterator iter = eventList.begin(); iter != eventList.end(); iter++)
        clearDeque((*iter));
}

/** Clear all events in the raw event list. WARNING! This method will delete all events in the
  * event list. This could cause seg faults if the events are used elsewhere.
  * \return Nothing. */
void Unpacker::ClearRawEvent() {
    clearDeque(rawEvent);
}

/** Get the minimum channel time from the event list.
  * \param[out] time The minimum time from the event list in system clock ticks.
  * \return True if the event list is not empty and false otherwise. */
bool Unpacker::GetFirstTime(double &time) {
    if (IsEmpty())
        return false;

    time = std::numeric_limits<double>::max();
    for (std::vector<std::deque<XiaData *> >::iterator iter = eventList.begin(); iter != eventList.end(); iter++) {
        if (iter->empty())
            continue;
        if (iter->front()->GetTime() < time)
            time = iter->front()->GetTime();
    }

    return true;
}

/** Check whether or not the eventList is empty.
  * \return True if the eventList is empty, and false otherwise. */
bool Unpacker::IsEmpty() {
    for (std::vector<std::deque<XiaData *> >::iterator iter = eventList.begin(); iter != eventList.end(); iter++)
        if (!iter->empty())
            return false;
    return true;
}

///Process all events in the event list.
void Unpacker::ProcessRawEvent() {
    ClearRawEvent();
}

///Called form ReadSpill. Scan the current spill and construct a list of events which fired by obtaining the module,
/// channel, trace, etc. of the timestamped event. This method will construct the event list for later processing.
///@param[in] buf : Pointer to an array of unsigned ints containing raw buffer data.
///@return The number of XiaDatas read from the buffer.
int Unpacker::ReadBuffer(unsigned int *buf, const unsigned int &vsn) {
    static XiaListModeDataDecoder decoder;

    if (maskMap_.size() != 0) {
        auto found = maskMap_.find(vsn);
        if(found == maskMap_.end())
            throw invalid_argument("Unpacker::ReadBuffer - Unable to locate VSN = " + to_string(vsn)
                                   + " in the maskMap. Ensure that it's defined in your configuration file!");
        mask_.SetFirmware((*found).second.first);
        mask_.SetFrequency((*found).second.second);
    }

    std::vector<XiaData *> decodedList = decoder.DecodeBuffer(buf, mask_);
    for (vector<XiaData *>::iterator it = decodedList.begin(); it != decodedList.end(); it++)
        AddEvent(*it);
    return (int) decodedList.size();
}

Unpacker::Unpacker() : debug_mode(false), eventWidth_(62), running(true),
                       TOTALREAD(1000000), // Maximum number of data words to read.
                       maxWords(131072), // Maximum number of data words for revision D.
                       numRawEvt(0), // Count of raw events read from file.
                       firstTime(0), eventStartTime(0), realStartTime(0), realStopTime(0) {

    for (unsigned int i = 0; i <= MAX_PIXIE_MOD; i++)
        for (unsigned int j = 0; j <= MAX_PIXIE_CHAN; j++)
            channel_counts[i][j] = 0;
}

Unpacker::~Unpacker() {
    ClearRawEvent();
    ClearEventList();
}

void Unpacker::InitializeDataMask(const std::string &firmware, const unsigned int &frequency) {
    if (frequency == 0) {
        unsigned int modCounter = 0;
        pugi::xml_node node = XmlInterface::get(firmware)->GetDocument()->child("Configuration").child("Map");
        for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it, modCounter++) {
            if (it->attribute("number").empty())
                throw invalid_argument("Unpacker::InitializeDataMask - Unable to read the \"number\" attribute from "
                                               "the module in position #" + to_string(modCounter) + "(0 counting)");
            if (it->attribute("firmware").empty())
                throw invalid_argument("Unpacker::InitializeDataMask - Unable to read the \"firmware\" attribute from"
                                               " the /Configuration/Map/Module/" + to_string(modCounter));
            if (it->attribute("frequency").empty())
                throw invalid_argument("Unpacker::InitializeDataMask - Unable to read the \"frequency\" attribute from"
                                               " the /Configuration/Map/Module/" + to_string(modCounter));

            maskMap_.insert(make_pair(it->attribute("number").as_uint(),
                                      make_pair(it->attribute("firmware").as_string(),
                                                it->attribute("frequency").as_uint())));
        }
    } else {
        mask_.SetFrequency(frequency);
        mask_.SetFirmware(firmware);
    }
}

/** ReadSpill is responsible for constructing a list of pixie16 events from
  * a raw data spill. This method performs sanity checks on the spill and
  * calls ReadBuffer in order to construct the event list.
  * \param[in]  data       Pointer to an array of unsigned ints containing the spill data.
  * \param[in]  nWords     The number of words in the array.
  * \param[in]  is_verbose Toggle the verbosity flag on/off.
  * \return True if the spill was read successfully and false otherwise.
  */
bool Unpacker::ReadSpill(unsigned int *data, unsigned int nWords, bool is_verbose/*=true*/) {
    const unsigned int maxVsn = 14; // No more than 14 pixie modules per crate
    unsigned int nWords_read = 0;

    int retval = 0; // return value from various functions

    // Various event counters
    unsigned long numEvents = 0;
    static int counter = 0; // the number of times this function is called
    static int evCount;     // the number of times data is passed to ScanList
    unsigned int lastVsn = 0xFFFFFFFF; // the last vsn read from the data
    time_t theTime = 0;

    if (counter == 0)
        maxModuleNumberInFile_ = 0;

    counter++;

    unsigned int lenRec = 0xFFFFFFFF;
    unsigned int vsn = 0xFFFFFFFF;
    bool fullSpill = false; // True if spill had all vsn's

    // While the current location in the buffer has not gone beyond the end
    // of the buffer (ignoring the last three delimiters, continue reading
    while (nWords_read <= nWords) {
        while (data[nWords_read] == 0xFFFFFFFF) // Search for the next non-delimiter.
            nWords_read++;

        // Retrieve the record length and the vsn number
        lenRec = data[nWords_read]; // Number of words in this record
        vsn = data[nWords_read + 1]; // Module number

        if (vsn > maxModuleNumberInFile_ && vsn != 9999 && vsn != 1000)
            maxModuleNumberInFile_ = vsn;

        // Check sanity of record length and vsn
        if (lenRec > maxWords || (vsn > maxVsn && vsn != 9999 && vsn != 1000)) {
            if (is_verbose)
                cout << "ReadSpill: SANITY CHECK FAILED: lenRec = " << lenRec << ", vsn = " << vsn << ", read "
                     << nWords_read << " of " << nWords << endl;
            return false;
        }

        // If the record length is 6, this is an empty channel.
        // Skip this vsn and continue with the next
        ///@TODO Revision specific, so move to ReadBuffData
        if (lenRec == 6) {
            nWords_read += lenRec;
            lastVsn = vsn;
            continue;
        }

        // If both the current vsn inspected is within an acceptable
        // range, begin reading the buffer.
        if (vsn < maxVsn) {
            if (lastVsn != 0xFFFFFFFF && vsn != lastVsn + 1) {
                if (is_verbose)
                    cout << "ReadSpill: MISSING BUFFER " << lastVsn + 1 << ", lastVsn = " << lastVsn << ", vsn = "
                         << vsn << ", lenrec = " << lenRec << endl;
                ClearEventList();
                fullSpill = false; // WHY WAS THIS TRUE!?!? CRT
            }

            // Read the buffer.	After read, the vector eventList will
            //contain pointers to all channels that fired in this buffer
            retval = ReadBuffer(&data[nWords_read], vsn);

            // If the return value is less than the error code,
            //reading the buffer failed for some reason.
            //Print error message and reset variables if necessary
            if (retval <= -100) {
                if (is_verbose)
                    cout << "ReadSpill: READOUT PROBLEM " << retval << " in event " << counter << endl;
                if (retval == -100) {
                    if (is_verbose)
                        cout << "ReadSpill:  Remove list " << lastVsn << " " << vsn << endl;
                    ClearEventList();
                }
                return false;
            } else if (retval > 0) {
                // Increment the total number of events observed
                numEvents += retval;
            }

            // Update the variables that are keeping track of what has been
            // analyzed and increment the location in the current buffer
            lastVsn = vsn;
            nWords_read += lenRec;
        } else if (vsn == 1000) { // Buffer with vsn 1000 was inserted with the time for superheavy exp't
            memcpy(&theTime, &data[nWords_read + 2], sizeof(time_t));
            if (is_verbose) {
                /*struct tm * timeinfo;
                timeinfo = localtime (&theTime);
                cout << "ReadSpill: Read wall clock time of " << asctime(timeinfo);*/
            }
            nWords_read += lenRec;
            continue;
        } else if (vsn == 9999) {
            // End spill vsn
            break;
        } else {
            // Bail out if we have lost our place,
            // (bad vsn) and process events
            cout << "ReadSpill: UNEXPECTED VSN " << vsn << endl;
            break;
        }
    } // while still have words

    if (nWords > TOTALREAD || nWords_read > TOTALREAD) {
        cout << "ReadSpill: Values of nn - " << nWords << " nk - " << nWords_read << " TOTALREAD - " << TOTALREAD
             << endl;
        return false;
    }

    // If the vsn is 9999 this is the end of a spill, signal this buffer
    // for processing and determine if the buffer is split between spills.
    if (vsn == 9999 || vsn == 1000) {
        fullSpill = true;
        nWords_read += 2; // Skip it
        lastVsn = 0xFFFFFFFF;
    }

    // Check the number of read words
    if (is_verbose && nWords_read != nWords)
        cout << "ReadSpill: Received spill of " << nWords << " words, but read " << nWords_read << " words\n";

    // If there are events to process, continue
    if (numEvents > 0) {
        if (fullSpill) { // if full spill process events
            // Sort the vector of pointers eventlist according to time
            //double lastTimestamp = (*(eventList.rbegin()))->time;

            // Sort the event list in time
            TimeSort();

            // Once the vector of pointers eventlist is sorted based on time,
            // begin the event processing in ScanList().
            // ScanList will also clear the event list for us.
            while (BuildRawEvent())
                ProcessRawEvent();

            ClearEventList();

            // Once the eventlist has been scanned, reset the number
            // of events to zero and update the event counter
            numEvents = 0;
            evCount++;

            // Every once in a while (when evcount is a multiple of 1000)
            // print the time elapsed doing the analysis
            if ((evCount % 1000 == 0 || evCount == 1) && theTime != 0)
                cout << endl << "ReadSpill: Data read up to poll status time " << ctime(&theTime);
        } else {
            if (is_verbose)
                cout << "ReadSpill: Spill split between buffers" << endl;
            ClearEventList(); // This tosses out all events read into the deque so far
            return false;
        }
    } else if (retval != -10) {
        if (is_verbose)
            cout << "ReadSpill: bad buffer, numEvents = " << numEvents << endl;
        ClearEventList(); // This tosses out all events read into the deque so far
        return false;
    }

    return true;
}

/** Write all recorded channel counts to a file.
  * \return Nothing.
  */
void Unpacker::Write() {
    std::ofstream count_output("counts.dat");
    if (count_output.good()) {
        for (unsigned int i = 0; i <= MAX_PIXIE_MOD; i++)
            for (unsigned int j = 0; j <= MAX_PIXIE_CHAN; j++)
                count_output << i << "\t" << j << "\t" << channel_counts[i][j] << endl;
        count_output.close();
    }
}
