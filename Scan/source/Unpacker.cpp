/** \file Unpacker.cpp
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
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string.h>
#include <time.h>
#include <cmath>
#include <algorithm>
#include <limits>

#include "Unpacker.hpp"
#include "XiaData.hpp"

void clearDeque(std::deque<XiaData*> &list){
	while(!list.empty()){
		delete list.front();
		list.pop_front();
	}
}

/** Scan the event list and sort it by timestamp.
  * \return Nothing.
  */
void Unpacker::TimeSort(){
	for(std::vector<std::deque<XiaData*> >::iterator iter = eventList.begin(); iter != eventList.end(); iter++){
		sort(iter->begin(), iter->end(), &XiaData::compareTime);
	}
}

/** Scan the time sorted event list and package the events into a raw
  * event with a size governed by the event width.
  * \return True if the event list is not empty and false otherwise.
  */
bool Unpacker::BuildRawEvent(){
	if(!rawEvent.empty())
		ClearRawEvent();

	if(numRawEvt == 0){// This is the first rawEvent. Do some special processing.
		// Find the first XiaData event. The eventList is time sorted by module.
		// The first component of each deque will be the earliest time from that module.
		// The first event time will be the minimum of these first components.
		if(!GetFirstTime(firstTime))
			return false;
		std::cout << "BuildRawEvent: First event time is " << firstTime << " clock ticks.\n";
		eventStartTime = firstTime;
	}
	else{ 
		// Move the event window forward to the next valid channel fire.
		if(!GetFirstTime(eventStartTime))
			return false;
	}

	realStartTime = eventStartTime+eventWidth;
	realStopTime = eventStartTime;
	
	unsigned int mod, chan;
	std::string type, subtype, tag;
	XiaData *current_event = NULL;

	// Loop over all time-sorted modules.
	for(std::vector<std::deque<XiaData*> >::iterator iter = eventList.begin(); iter != eventList.end(); iter++){
		if(iter->empty())
			continue;
			
		// Loop over the list of channels that fired in this buffer
		while(!iter->empty()){
			current_event = iter->front();
			mod = current_event->modNum;
			chan = current_event->chanNum;
	
			if(mod > MAX_PIXIE_MOD || chan > MAX_PIXIE_CHAN){ // Skip this channel
				std::cout << "BuildRawEvent: Encountered non-physical Pixie ID (mod = " << mod << ", chan = " << chan << ")\n";
				delete current_event;
				iter->pop_front();
				continue;
			}

			double currtime = current_event->time;

			// Check for backwards time-skip. This is un-handled currently and needs fixed CRT!!!
			if(currtime < eventStartTime)
				std::cout << "BuildRawEvent: Detected backwards time-skip from start=" << eventStartTime << " to " << current_event->time << "???\n";

			// If the time difference between the current and previous event is 
			// larger than the event width, finalize the current event, otherwise
			// treat this as part of the current event
			if((currtime - eventStartTime) > eventWidth){ // 62 pixie ticks represents ~0.5 us
				break;
			}

			// Check for the minimum time in this raw event.
			if(currtime < realStartTime)
				realStartTime = currtime;
			
			// Check for the maximum time in this raw event.
			if(currtime > realStopTime)
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
  * \return True if the XiaData's module number is valid and false otherwise.
  */
bool Unpacker::AddEvent(XiaData *event_){
	if(event_->modNum > MAX_PIXIE_MOD){ return false; }
	
	// Check for the need to add a new deque to the event list.
	if(event_->modNum+1 > (unsigned int)eventList.size()){
		for(unsigned int i = 0; i < (event_->modNum+1 - (unsigned int)eventList.size()); i++){
			eventList.push_back(std::deque<XiaData*>());
		}
	}
	
	eventList.at(event_->modNum).push_back(event_);
	
	return true;
}

/** Clear all events in the spill event list. WARNING! This method will delete all events in the
  * event list. This could cause seg faults if the events are used elsewhere.
  * \return Nothing.
  */	
void Unpacker::ClearEventList(){
	for(std::vector<std::deque<XiaData*> >::iterator iter = eventList.begin(); iter != eventList.end(); iter++){
		clearDeque((*iter));
	}
}

/** Clear all events in the raw event list. WARNING! This method will delete all events in the
  * event list. This could cause seg faults if the events are used elsewhere.
  * \return Nothing.
  */
void Unpacker::ClearRawEvent(){
	clearDeque(rawEvent);
}

/** Get the minimum channel time from the event list.
  * \param[out] time The minimum time from the event list in system clock ticks.
  * \return True if the event list is not empty and false otherwise.
  */
bool Unpacker::GetFirstTime(double &time){
	if(IsEmpty())
		return false;

	time = std::numeric_limits<double>::max();
	for(std::vector<std::deque<XiaData*> >::iterator iter = eventList.begin(); iter != eventList.end(); iter++){
		if(iter->empty())
			continue;
		if(iter->front()->time < time)
			time = iter->front()->time;
	}
	
	return true;
}

/** Check whether or not the eventList is empty.
  * \return True if the eventList is empty, and false otherwise.
  */
bool Unpacker::IsEmpty(){
	for(std::vector<std::deque<XiaData*> >::iterator iter = eventList.begin(); iter != eventList.end(); iter++){
		if(!iter->empty())
			return false;
	}
	return true;
}

/** Process all events in the event list.
  * \param[in]  addr_ Pointer to a ScanInterface object. Unused by default.
  * \return Nothing.
  */
void Unpacker::ProcessRawEvent(ScanInterface *addr_/*=NULL*/){
	ClearRawEvent();
}

/** Called form ReadSpill. Scan the current spill and construct a list of
  * events which fired by obtaining the module, channel, trace, etc. of the
  * timestamped event. This method will construct the event list for
  * later processing.
  * \param[in]  buf    Pointer to an array of unsigned ints containing raw buffer data.
  * \param[out] bufLen The number of words in the buffer.
  * \return The number of XiaDatas read from the buffer.
  */
int Unpacker::ReadBuffer(unsigned int *buf, unsigned long &bufLen){						
	// multiplier for high bits of 48-bit time
	static const double HIGH_MULT = pow(2., 32.); 

	unsigned int modNum;
	unsigned long numEvents = 0;
	unsigned int *bufStart = buf;

	// Determine the number of words in the buffer
	bufLen = *buf++;

	// Read the module number
	modNum = *buf++;

	XiaData *lastVirtualChannel = NULL;

	if(bufLen > 0){ // Check if the buffer has data
		if(bufLen == 2){ // this is an empty channel
			return 0;
		}
		while( buf < bufStart + bufLen ){
			XiaData *currentEvt = new XiaData();

			// decoding event data... see pixie16app.c
			// buf points to the start of channel data
			unsigned int chanNum      = (buf[0] & 0x0000000F);
			unsigned int slotNum      = (buf[0] & 0x000000F0) >> 4;
			unsigned int crateNum     = (buf[0] & 0x00000F00) >> 8;
			unsigned int headerLength = (buf[0] & 0x0001F000) >> 12;
			unsigned int eventLength  = (buf[0] & 0x1FFE0000) >> 17;

			currentEvt->virtualChannel = ((buf[0] & 0x20000000) != 0);
			currentEvt->saturatedBit   = ((buf[0] & 0x40000000) != 0);
			currentEvt->pileupBit      = ((buf[0] & 0x80000000) != 0);

			// Rev. D header lengths not clearly defined in pixie16app_defs
			//! magic numbers here for now
			if(headerLength == 1){
				// this is a manual statistics block inserted by the poll program
				/*stats.DoStatisticsBlock(&buf[1], modNum);
				buf += eventLength;
				numEvents = -10;*/
				continue;
			}
			if(headerLength != 4 && headerLength != 8 && headerLength != 12 && headerLength != 16){
				std::cout << "ReadBuffer: Unexpected header length: " << headerLength << std::endl;
				std::cout << "ReadBuffer:   Buffer " << modNum << " of length " << bufLen << std::endl;
				std::cout << "ReadBuffer:   CHAN:SLOT:CRATE " << chanNum << ":" << slotNum << ":" << crateNum << std::endl;
				// advance to next event and continue
				// buf += EventLength;
				// continue;

				// skip the rest of this buffer
				return numEvents;
			}

			unsigned int lowTime     = buf[1];
			unsigned int highTime    = buf[2] & 0x0000FFFF;
			unsigned int cfdTime     = (buf[2] & 0xFFFF0000) >> 16;
			unsigned int energy      = buf[3] & 0x0000FFFF;
			unsigned int traceLength = (buf[3] & 0xFFFF0000) >> 16;

			if(headerLength == 8 || headerLength == 16){
				// Skip the onboard partial sums for now 
				// trailing, leading, gap, baseline
			}

			if(headerLength >= 12){
				int offset = headerLength - 8;
				for (int i=0; i < currentEvt->numQdcs; i++){
					currentEvt->qdcValue[i] = buf[offset + i];
				}
			}	 

			// One last check
			if( traceLength / 2 + headerLength != eventLength ){
				std::cout << "ReadBuffer: Bad event length (" << eventLength << ") does not correspond with length of header (";
				std::cout << headerLength << ") and length of trace (" << traceLength << ")" << std::endl;
				buf += eventLength;
				continue;
			}

			// Handle multiple crates
			modNum += 100 * crateNum;

			currentEvt->chanNum = chanNum;
			currentEvt->modNum = modNum;
			/*if(currentEvt->virtualChannel){
				DetectorLibrary* modChan = DetectorLibrary::get();

				currentEvt->modNum += modChan->GetPhysicalModules();
				if(modChan->at(modNum, chanNum).HasTag("construct_trace")){
					lastVirtualChannel = currentEvt;
				}
			}*/

			channel_counts[modNum][chanNum]++;

			currentEvt->energy = energy;
			if(currentEvt->saturatedBit){ currentEvt->energy = 16383; }
					
			currentEvt->trigTime = lowTime;
			currentEvt->cfdTime	= cfdTime;
			currentEvt->eventTimeHi = highTime;
			currentEvt->eventTimeLo = lowTime;
			currentEvt->time = highTime * HIGH_MULT + lowTime;

			buf += headerLength;
			// Check if trace data follows the channel header
			if( traceLength > 0 ){
				// sbuf points to the beginning of trace data
				unsigned short *sbuf = (unsigned short *)buf;

				currentEvt->reserve(traceLength);

				/*if(currentEvt->saturatedBit)
					currentEvt->trace.SetValue("saturation", 1);*/

				if( lastVirtualChannel != NULL && lastVirtualChannel->adcTrace.empty() ){		
					lastVirtualChannel->assign(traceLength, 0);
				}
				// Read the trace data (2-bytes per sample, i.e. 2 samples per word)
				for(unsigned int k = 0; k < traceLength; k ++){		
					currentEvt->push_back(sbuf[k]);

					if(lastVirtualChannel != NULL){
						lastVirtualChannel->adcTrace[k] += sbuf[k];
					}
				}
				buf += traceLength / 2;
			}
 
			AddEvent(currentEvt);
			
			numEvents++;
		}
	} 
	else{ // if buffer has data
		std::cout << "ReadBuffer: ERROR IN ReadBuffData, LIST UNKNOWN" << std::endl;
		return -100;
	}
	
	return numEvents;
}

Unpacker::Unpacker(){
	debug_mode = false;
	running = true;

	TOTALREAD = 1000000; // Maximum number of data words to read.
	maxWords = 131072; // Maximum number of data words for revision D.	
	numRawEvt = 0; // Count of raw events read from file.
	eventWidth = 62; // ~ 500 ns in 8 ns pixie clock ticks.
	
	firstTime = 0;
	eventStartTime = 0;
	realStartTime = 0;
	realStopTime = 0;
	
	interface = NULL;
	
	for(unsigned int i = 0; i <= MAX_PIXIE_MOD; i++){
		for(unsigned int j = 0; j <= MAX_PIXIE_CHAN; j++){
			channel_counts[i][j] = 0;
		}
	}
}

/// Destructor.
Unpacker::~Unpacker(){
	ClearRawEvent();
	ClearEventList();
}

/** ReadSpill is responsible for constructing a list of pixie16 events from
  * a raw data spill. This method performs sanity checks on the spill and
  * calls ReadBuffer in order to construct the event list.
  * \param[in]  data       Pointer to an array of unsigned ints containing the spill data.
  * \param[in]  nWords     The number of words in the array.
  * \param[in]  is_verbose Toggle the verbosity flag on/off.
  * \return True if the spill was read successfully and false otherwise.
  */	
bool Unpacker::ReadSpill(unsigned int *data, unsigned int nWords, bool is_verbose/*=true*/){
	const unsigned int maxVsn = 14; // No more than 14 pixie modules per crate
	unsigned int nWords_read = 0;
	
	//static clock_t clockBegin; // Initialization time
	//time_t tmsBegin;

	int retval = 0; // return value from various functions
	
	unsigned long bufLen;
	
	// Various event counters 
	unsigned long numEvents = 0;
	static int counter = 0; // the number of times this function is called
	static int evCount;	 // the number of times data is passed to ScanList
	static unsigned int lastVsn; // the last vsn read from the data
	time_t theTime = 0;

	// Initialize the scan program before the first event 
	if(counter==0){ lastVsn=-1; } // Set last vsn to -1 so we expect vsn 0 first 	
	counter++;
 
	unsigned int lenRec = 0xFFFFFFFF;
	unsigned int vsn = 0xFFFFFFFF;
	bool fullSpill=false; // True if spill had all vsn's

	// While the current location in the buffer has not gone beyond the end
	// of the buffer (ignoring the last three delimiters, continue reading
	while (nWords_read <= nWords){
		// Retrieve the record length and the vsn number
		lenRec = data[nWords_read]; // Number of words in this record
		vsn = data[nWords_read+1]; // Module number
	
		// Check sanity of record length and vsn
		if(lenRec > maxWords || (vsn > maxVsn && vsn != 9999 && vsn != 1000)){ 
			if(is_verbose){
				std::cout << "ReadSpill: SANITY CHECK FAILED: lenRec = " << lenRec << ", vsn = " << vsn << ", read " << nWords_read << " of " << nWords << std::endl;
			}
			return false;	
		}

		// If the record length is 6, this is an empty channel.
		// Skip this vsn and continue with the next
		//! Revision specific, so move to ReadBuffData
		if(lenRec==6){
			nWords_read += lenRec;
			lastVsn=vsn;
			continue;
		}

		// If both the current vsn inspected is within an acceptable 
		// range, begin reading the buffer.
		if(vsn < maxVsn){
			if(lastVsn != 0xFFFFFFFF && vsn != lastVsn+1){
				if(is_verbose){ 
					std::cout << "ReadSpill: MISSING BUFFER " << lastVsn+1 << ", lastVsn = " << lastVsn << ", vsn = " << vsn << ", lenrec = " << lenRec << std::endl;
				}
				ClearEventList();
				fullSpill=false; // WHY WAS THIS TRUE!?!? CRT
			}
			
			// Read the buffer.	After read, the vector eventList will 
			//contain pointers to all channels that fired in this buffer
			retval = ReadBuffer(&data[nWords_read], bufLen);

			// If the return value is less than the error code, 
			//reading the buffer failed for some reason.	
			//Print error message and reset variables if necessary
			if(retval <= -100){
				if(is_verbose){ std::cout << "ReadSpill: READOUT PROBLEM " << retval << " in event " << counter << std::endl; }
				if(retval == -100){
					if(is_verbose){ std::cout << "ReadSpill:  Remove list " << lastVsn << " " << vsn << std::endl; }
					ClearEventList();
				}
				return false;
			}
			else if(retval > 0){		
				// Increment the total number of events observed 
				numEvents += retval;
			}
			
			// Update the variables that are keeping track of what has been
			// analyzed and increment the location in the current buffer
			lastVsn = vsn;
			nWords_read += lenRec;
		} 
		else if(vsn == 1000){ // Buffer with vsn 1000 was inserted with the time for superheavy exp't
			memcpy(&theTime, &data[nWords_read+2], sizeof(time_t));
			if(is_verbose){
				/*struct tm * timeinfo;
				timeinfo = localtime (&theTime);
				std::cout << "ReadSpill: Read wall clock time of " << asctime(timeinfo);*/
			}
			nWords_read += lenRec;
			continue;
		}
		else if(vsn == 9999){
			// End spill vsn
			break;
		}
		else{
			// Bail out if we have lost our place,		
			// (bad vsn) and process events	 
			std::cout << "ReadSpill: UNEXPECTED VSN " << vsn << std::endl;
			break;
		}
	} // while still have words

	if(nWords > TOTALREAD || nWords_read > TOTALREAD){
		std::cout << "ReadSpill: Values of nn - " << nWords << " nk - "<< nWords_read << " TOTALREAD - " << TOTALREAD << std::endl;
		return false;
	}

	// If the vsn is 9999 this is the end of a spill, signal this buffer
	// for processing and determine if the buffer is split between spills.
	if(vsn == 9999 || vsn == 1000){
		fullSpill = true;
		nWords_read += 2; // Skip it
		lastVsn = 0xFFFFFFFF;
	}

	// Check the number of read words
	if(is_verbose && nWords_read != nWords){
		std::cout << "ReadSpill: Received spill of " << nWords << " words, but read " << nWords_read << " words\n";
	}

	// If there are events to process, continue 
	if(numEvents > 0){
		if(fullSpill){ // if full spill process events
			// Sort the vector of pointers eventlist according to time
			//double lastTimestamp = (*(eventList.rbegin()))->time;

			// Sort the event list in time
			TimeSort();

			// Once the vector of pointers eventlist is sorted based on time,
			// begin the event processing in ScanList().
			// ScanList will also clear the event list for us.
			while(BuildRawEvent()){
				// Process the event.
				ProcessRawEvent(interface);
			}
			
			ClearEventList();
			
			// Once the eventlist has been scanned, reset the number 
			// of events to zero and update the event counter
			numEvents=0;
			evCount++;
			
			// Every once in a while (when evcount is a multiple of 1000)
			// print the time elapsed doing the analysis
			if((evCount % 1000 == 0 || evCount == 1) && theTime != 0){
				std::cout << std::endl << "ReadSpill: Data read up to poll status time " << ctime(&theTime);
			}
		}
		else {
			if(is_verbose){ std::cout << "ReadSpill: Spill split between buffers" << std::endl; }
			ClearEventList(); // This tosses out all events read into the deque so far
			return false; 
		}		
	}
	else if(retval != -10){
		if(is_verbose){ std::cout << "ReadSpill: bad buffer, numEvents = " << numEvents << std::endl; }
		ClearEventList(); // This tosses out all events read into the deque so far
		return false;
	}
	
	return true;		
}

/** Write all recorded channel counts to a file.
  * \return Nothing.
  */
void Unpacker::Write(){
	std::ofstream count_output("counts.dat");
	if(count_output.good()){
		for(unsigned int i = 0; i <= MAX_PIXIE_MOD; i++){
			for(unsigned int j = 0; j <= MAX_PIXIE_CHAN; j++){
				count_output << i << "\t" << j << "\t" << channel_counts[i][j] << std::endl;
			}
		}
		count_output.close();
	}
}
