/** \file PixieStd.ccp
 *
 * \brief pixie_std provides the interface between the HRIBF scan 
 * and the C++ analysis

 * This provides the interface between the HRIBF scan and the C++ analysis
 * and as such is not a class in its own right.  In this file the data
 * received from scan is first reassembled into a pixie16 spill and then
 * channel objects are made.
 *
 * The main program.  Buffers are passed to hissub_() and channel information
 * is extracted in ReadBuffData(). All channels that fired are stored as a
 * vector of pointers which is sorted based on time and then events are built
 * with each event being sent to the detector driver for processing.
 *
 * SNL - 7-20-07

 * SNL 2-5-08 Added in diagnostic spectra including:
 *   runtime, channel time difference in an event, time difference between
 *   events, length of event, and length of buffer
 *
 *
 * SNL 5-14-08 At SP's request, error message and termination occur if a
 *   module number is encountered in the data stream that is not included in
 *   the map.txt file
 *
 * DTM 5-5-10 Significant changes throughout for conciseness, optimization,
 *   and better error checking of incoming buffers
 */

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <cstring>
#include <ctime>

#include <unistd.h>
#include <sys/times.h>

#include "DetectorDriver.h"
#include "RawEvent.h"
#include "damm_plotids.h"
#include "param.h"
#include "pixie16app_defs.h"

using namespace std;
using pixie::word_t;

/**
 * Contains the description of each channel in the analysis.  The
 * description is read in from map.txt and includes the detector type and 
 * subtype, the damm spectrum number, and physical location
 */
vector<Identifier> modChan;

/**
 * Contains event information, the information is filled in ScanList() and is 
 * referenced externally in DetectorDriver.cpp, particularly in ProcessEvent()
 */
RawEvent rawev;

/** Driver used to process the raw events */
DetectorDriver driver;

/** The max number of modules used in the map.txt file */
unsigned int numModules;

enum HistoPoints {BUFFER_START, BUFFER_END, EVENT_START = 10, EVENT_CONTINUE};

// Function forward declarations
int InitMap(void);
void ScanList(vector<ChanEvent*> &eventList);
void RemoveList(vector<ChanEvent*> &eventList);
void HistoStats(unsigned int, double, double, HistoPoints);

#ifdef newreadout
/**
 * \brief Extract channel information from the raw parameter array ibuf
 */
void hissub_sec(unsigned int *ibuf[],unsigned int *nhw);
bool MakeModuleData(const word_t *data, unsigned long nWords); 
#endif

int ReadBuffData(word_t *lbuf, unsigned long *BufLen,
		 vector<ChanEvent *> &eventList);
void Pixie16Error(int errornum);

const string scanMode = "scan";

/** \brief Comparison function for sorting channel events (by pointer)
 *
 * Function that will be used in the event list sorting routine to sort the
 * channel event objects chronologically (early events first)
 */
bool Compare(const ChanEvent *a, const ChanEvent *b)
{
    return (a->GetTime() < b->GetTime());
}

/** \fn extern "C" void hissub_(unsigned short *ibuf[],unsigned short *nhw) 
 * \brief interface between scan and C++
 *
 * In a typical experiment, Pixie16 reads data from all modules when one module
 * has hit the maximum number of events which is programmed during experimental
 * setup.  This spill of data is then broken into smaller chunks for
 * transmitting across the network.  The hissub_ function takes the chunks
 * and reconstructs the spill.
 *
 * Summarizing the terminology:
 *  - Spill  - a readout of all Pixie16 modules
 *  - Buffer - the data from a specific Pixie16 module
 *  - Chunk  - the packet transferred from Pixie16 to the acquisition
 *    
 * The hissub_ function is passed a pointer to an array with data (ibuf) and
 * the number of half words (nhw) contained in it.  This function is used with
 * the new Pixie16 readout (which is the default).  If the old Pixie16 readout
 * is used, the code should be recompiled without the newreadout flag in which
 * case this particular function is not used.  
*/

#ifdef newreadout

// THIS SHOULD NOT BE SET LARGER THAN 1,000,000
//  this defines the maximum amount of data that will be received in a spill
const unsigned int TOTALREAD = 1000000;

#ifndef REVD
const unsigned int maxWords = IO_BUFFER_LENGTH; // Revision A
#else
const unsigned int maxWords = EXTERNAL_FIFO_LENGTH; //Revision D
#endif

extern "C" void hissub_(unsigned short *sbuf[],unsigned short *nhw)
{
    const unsigned int maxChunks = 200;

    static word_t totData[TOTALREAD];
    // keep track of the number of bad spills
    static unsigned int spillInvalidCount = 0, spillValidCount = 0;
    static bool firstTime = true;
    // might take a few entries into this function to get all the buffers in a spill
    static unsigned int bufInSpill = 0;    
    static unsigned int dataWords = 0;
    
    /*Assign ibuf variable to local variable for use in function */
    word_t *buf=(word_t*)sbuf;
    
    /* Initialize variables */
    unsigned long totWords=0;
    word_t nWords=buf[0] / 4;
    word_t totBuf=buf[1];
    word_t bufNum=buf[2];
    static unsigned int lastBuf = U_DELIMITER;

    // Check to make sure the number of buffers is not excessively large 
    if (totBuf > maxChunks) {
	cout << "LARGE TOTNUM = " << bufNum << endl;
	return;
    }

    /* Find a starting point in a file immediately following the 5-word buffer
         which indicates the end of a spill
     */
    if(bufNum != 0 && firstTime) {
	do {
	    if (buf[totWords] == U_DELIMITER) {
		cout << "  -1 DELIMITER, " 
		     << buf[totWords] << buf[totWords + 1] << endl;
		return;
	    }
	    nWords = buf[totWords] / 4;
	    totBuf = buf[totWords+1];
	    bufNum = buf[totWords+2];
	    totWords += nWords+1;
	    cout << "SKIP " << bufNum << " of " << totBuf << endl;
	} while(nWords != 5);
    }
    firstTime = false;
    
    do {
	do {
	    /*Determine the number of words, total number of buffers, and
	      current buffer number at this point in the chunk.  
	      Note: the total number of buffers is repeated for each 
	      buffer in the chunk */
	    if (buf[totWords] == U_DELIMITER) return;

	    nWords = buf[totWords] / 4;
 	    bufNum = buf[totWords+2]; 
	    // read total number of buffers later after we check if the last spill was good
	    if (lastBuf != U_DELIMITER && bufNum != lastBuf + 1) {
		cout << "Buffer skipped, Last: " << lastBuf << " of " << totBuf 
		     << " (" << bufInSpill << ") read -- Now: " << bufNum << endl;
		// if we are only missing the vsn 9999 terminator, reconstruct it
		if (lastBuf + 2 == totBuf && bufInSpill == totBuf - 1) {
		    cout << "  Reconstructing final buffer " << lastBuf + 1 << "." << endl;
		    totData[dataWords++] = 2;
		    totData[dataWords++] = 9999;
		    
		    MakeModuleData(totData, dataWords);
		    spillValidCount++;
		    bufInSpill = 0; dataWords = 0; lastBuf = -1;
		} else if (bufNum == 0) {
		    cout << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
			 << "  INCOMPLETE BUFFER " << spillInvalidCount++ 
			 << "\n  " << spillValidCount << " valid spills so far."
			 << " Starting fresh spill." << endl;
		    // throw away previous collected data and start fresh
		    bufInSpill = 0; dataWords = 0; lastBuf = -1;
		}
	    } // check that the chunks are in order
	    // update the total chunks only after the sanity checks above
	    totBuf = buf[totWords+1];
	    if (totBuf > maxChunks) {
		cout << "EEEEE LOST DATA: Total buffers = " << totBuf 
		     <<  ", word count = " << nWords << endl;
		return;
	    }
	    if (bufNum > totBuf - 1) {
		cout << "EEEEEEE LOST DATA: Buffer number " << bufNum
		     << " of total buffers " << totBuf << endl;
		return;
	    }
	    lastBuf = bufNum;

	    /* Increment the number of buffers in a spill*/
	    bufInSpill++;
	    if(nWords == 0) {
		cout << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE NWORDS 0" << endl;
		return;
	    }
	    
	    /* Extract this buffer information into the TotData array*/
	    memcpy(&totData[dataWords], &buf[totWords+3], (nWords - 3) * sizeof(int));
	    dataWords += nWords - 3;
	    
	    // Increment location in file 
	    // one extra word to pass over "-1" delimiter signalling end of buffer
	    totWords += nWords+1;
	    if (bufNum == totBuf - 1 && nWords != 5) {
		cout << "Strange final buffer " << bufNum << " of " << totBuf
		     << " with " << nWords << " words" << endl;
	    }
	    if (nWords == 5 && bufNum != totBuf - 1) {
		cout << "Five word buffer " << bufNum << " of " << totBuf
		     << " WORDS: " 
		     << hex << buf[3] << " " << buf[4] << dec << endl;
	    }
	} while(nWords != 5 || bufNum != totBuf - 1);
	/* reached the end of a spill when nwords = 5 and last chunk in spill */

	/* make sure we retrieved all the chunks of the spill */
	if (bufInSpill != totBuf) {
	    cout << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE  INCOMPLETE BUFFER "
		 << spillInvalidCount++ 
		 << "\n I/B [  " << bufInSpill << " of " << totBuf << " : pos " << totWords 
		 << "    " << spillValidCount << " total spills"
		 << "\n| " << hex << buf[0] << " " << buf[1] << "  " 
		 << buf[2] << " " << buf[3]
		 << "\n| " << dec << buf[totWords] << " " << buf[totWords+1] << "  "
		 << buf[totWords+2] << " " << buf[totWords+3] << endl;
	} else {
	    spillValidCount++;
	    MakeModuleData(totData, dataWords);	    
	} // else the number of buffers is complete
	dataWords = 0; bufInSpill = 0; lastBuf = -1; // reset the number of buffers recorded
    } while (totWords < nhw[0] / 4);
}

/** \brief inserts a delimiter in between individual module data and at end of 
 * buffer. Data is then passed to hissub_sec() for processing.
 */
bool MakeModuleData(const word_t *data, unsigned long nWords)
{
    const unsigned int maxVsn = 14; // no more than 14 pixie modules per crate

    unsigned int inWords = 0, outWords = 0;
    
    static word_t modData[TOTALREAD];
    // create a constant pointer to this data block for passing to hissub_sec
    static word_t* dataPtr = modData; 

    do {
	word_t lenRec = data[inWords];	
        word_t vsn    = data[inWords+1];
	/* Check sanity of record length and vsn*/
	if(lenRec > maxWords || (vsn > maxVsn && vsn != 9999)) { 
	    cout << "SANITY CHECK FAILED: lenRec = " << lenRec
		 << ", vsn = " << vsn << ", inWords = " << inWords
		 << " of " << nWords << ", outWords = " << outWords << endl;
	    // exit(EXIT_FAILURE);
	    return false;  
	}
	
	/*Extract the data from TotData and place into ModData*/	      
	memcpy(&modData[outWords], &data[inWords], lenRec * sizeof(word_t)); 
	inWords  += lenRec;
	outWords += lenRec;
	
	modData[outWords++]=U_DELIMITER;
    } while (inWords < nWords);
	    
    modData[outWords++]=U_DELIMITER;
    modData[outWords++]=U_DELIMITER;
	    
    if(nWords > TOTALREAD || inWords > TOTALREAD || outWords > TOTALREAD ) {
	cout << "Values of nn - " << nWords << " nk - "<< inWords  
	     << " mm - " << outWords << " TOTALREAD - " << TOTALREAD << endl;
	Pixie16Error(2); 
	return false;
    }

    //! shouldn't this be 4 * outWords
    unsigned int nhw = 8 * outWords; // calculate the number of half short ints

    hissub_sec(&dataPtr, &nhw);

    return true;
}
#endif


/**
 * If the new Pixie16 readout is used (default), this routine processes the
 * reconstructed buffer.  Specifically, it retrieves channel information
 * and places the channel information into a list of channels that triggered in
 * this spill.  The list of channels is sorted according to the event time
 * assigned to each channel by Pixie16 and the sorted list is passed to
 * ScanList() for raw event creation. 
 *
 * If the old pixie readout is used then this function is
 * redefined as hissub_.
 */
#ifdef newreadout
void hissub_sec(word_t *ibuf[],unsigned int *nhw)
#else
extern "C" void hissub_(unsigned short *ibuf[],unsigned short *nhw)
#endif
{
    static float hz = sysconf(_SC_CLK_TCK); // get the number of clock ticks per second
    static clock_t clockBegin; // initialization time
    static struct tms tmsBegin;

    vector<ChanEvent*> eventList; // vector to hold the events

    // local version of ibuf pointer
    word_t *lbuf;

    int retval = 0; // return value from various functions
    
    unsigned long bufLen;
    
    /*
      Various event counters
    */
    unsigned long numEvents = 0;
    static int counter = 0; // the number of times this function is called
    static int evCount;     // the number of times data is passed to ScanList
    static unsigned int lastVsn; // the last vsn read from the data

    /*
      Assign the local variable lbuf to the variable ibuf which is passed into
      the routine.  The difference between the new and old pixie16 readouts is
      the type of the variable and source of the variable ibuf.

      In the new readout ibuf is from a C++ function and is of type unsigned int*
      In the old readout ibuf is from a Fortran function and is of type
      unsigned short*

      This results in two different assignment statements depending on 
      the readout.
    */
#ifdef newreadout
    lbuf=(word_t *)ibuf[0];
#else
    lbuf=(word_t *)ibuf; //old readout
#endif

    /* Initialize the scan program before the first event */
    if (counter==0) {
        /* Retrieve the current time for use later to determine the total
	 * running time of the analysis.
	 */
        clockBegin = times(&tmsBegin);
	cout << "First buffer at " << clockBegin << " sys time" << endl;
        /* After completion the descriptions of all channels are in the modChan
	 * vector, the DetectorDriver and rawevent have been initialized with the
	 * detectors that will be used in this analysis.
	 */
        InitMap();

	/* Make a last check to see that everything is in order for the driver 
	 * before processing data
	 */
	if ( !driver.SanityCheck() ) {
	    cout << "Detector driver did not pass sanity check!" << endl;
	    exit(EXIT_FAILURE);
	}

        lastVsn=-1; // set last vsn to -1 so we expect vsn 0 first 	

	cout << "Init done at " << times(&tmsBegin) << " sys time." << endl;
    }
    counter++;
 
    unsigned int nWords=0;  // buffer counter, reset only for new buffer
 
    // true if the buffer being analyzed is split across a spill from pixie
    bool multSpill;

    do {
	word_t vsn = U_DELIMITER;
	bool fullSpill=false; //true if spill had all vsn's
        multSpill=false;  //assume all buffers are not split between spills    

        /* while the current location in the buffer has not gone beyond the end
         * of the buffer (ignoring the last three delimiters, continue reading
	 */
        while (nWords < (nhw[0]/2 - 6)) {
            /*
              Retrieve the record length and the vsn number
            */
            word_t lenRec = lbuf[nWords];
            vsn = lbuf[nWords+1];
            
            /* If the record length is -1 (after end of spill), increment the
	       location in the buffer by two and start over with the while loop
	     */
            if (lenRec == U_DELIMITER) {
                nWords += 2;  // increment two whole words and try again
                continue;                         
            }
            
            /*
              If the record length is 6, this is an empty channel.
	      Skip this vsn and continue with the next
            */
	    //! Revision specific, so move to ReadBuffData
            if (lenRec==6) {
                nWords += lenRec+1; // one additional word for delimiter
                lastVsn=vsn;
                continue;
            }
            
            /* If both the current vsn inspected is within an acceptable 
	       range, begin reading the buffer.
            */
            if ( vsn < numModules ) {
	        if ( lastVsn != U_DELIMITER) {
		    // the modules should be read out cyclically
		    if ( ((lastVsn+1) % numModules) != vsn ) {
			cout << " MISSING BUFFER " << vsn
			     << " -- lastVsn = " << lastVsn << "  " 
			     << ", length = " << lenRec << endl;
                        RemoveList(eventList);
                        fullSpill=true;
                    }
                }
                /* Read the buffer.  After read, the vector eventList will 
		   contain pointers to all channels that fired in this buffer
                */

                retval= ReadBuffData(&lbuf[nWords],&bufLen,eventList);

                
                /* If the return value is less than the error code, 
		   reading the buffer failed for some reason.  
		   Print error message and reset variables if necessary
                */
                if ( retval <= readbuff::ERROR ) {
		    cout << " READOUT PROBLEM " << retval 
			 << " in event " << counter << endl;
                    if ( retval == readbuff::ERROR ) {
			cout << "  Remove list " << lastVsn << " " << vsn << endl;
                        RemoveList(eventList); 	                        
                    }
                    return;
                } else if ( retval == 0 ) {
		    // empty buffers are regular in Rev. D data
		    // cout << " EMPTY BUFFER" << endl;
		  nWords += lenRec + 1;
		  lastVsn = vsn;
		  continue;
                  //  return;
                } else if ( retval > 0 ) {		
		  /* increment the total number of events observed */
		  numEvents += retval;
                }
                /* Update the variables that are keeping track of what has been
		   analyzed and increment the location in the current buffer
                */
                lastVsn = vsn;
                nWords += lenRec+1; // one extra word for delimiter
            } else break; // bail out if we have lost our place (bad vsn) and process events            
        } // while still have words
	if (nWords > nhw[0] / 2 - 6) {
	    cout << "This actually happens!" << endl;	    
	}
        
        /* If the vsn is 9999 this is the end of a spill, signal this buffer
	   for processing and determine if the buffer is split between spills.
        */
        if ( vsn == 9999 ) {
            fullSpill = true;
            nWords += 3;//skip it
            if (lbuf[nWords+1] != U_DELIMITER) {
		cout << "this actually happens!" << endl;
                multSpill = true;
            }
            lastVsn=U_DELIMITER;
        }
        
        /* if there are events to process, continue */
        if( numEvents>0 ) {
	    if (fullSpill) { 	  // if full spill process events
		// sort the vector of pointers eventlist according to time
		
		sort(eventList.begin(),eventList.end(),Compare);
		
		/* once the vector of pointers eventlist is sorted based on time,
		   begin the event processing in ScanList()
		*/
		ScanList(eventList);
		if( retval<0 ) {
		    cout << "scan list error " << retval << endl;
		    return;
		}
		
		/* once the eventlist has been scanned, remove it from memory
		   and reset the number of events to zero and update the event
		   counter
		*/
		RemoveList(eventList);
		numEvents=0;
		evCount++;
		
		/*
		  every once in a while (when evcount is a multiple of 1000)
		  print the time elapsed doing the analysis
		*/
		if(evCount % 1000 == 0){
		    tms tmsNow;
		    clock_t clockNow = times(&tmsNow);

		    cout << " event = " << evCount << ", user time = " 
			 << (tmsNow.tms_utime - tmsBegin.tms_utime) / hz
			 << ", system time = " 
			 << (tmsNow.tms_stime - tmsBegin.tms_stime) / hz
			 << ", real time = "
			 << (clockNow - clockBegin) / hz << endl;
		}		
	    } // end fullSpill 
	    else {
		cout << "Spill split between buffers" << endl;
		return; //! this tosses out all events read into the vector so far
	    }	    
        }  // end numEvents > 0
        else {
	    cout << "bad buffer, numEvents = " << numEvents << endl;
            return;
        }
        
    } while (multSpill); // end while loop over multiple spills
    return;      
}


/** Remove events in list from memory when no longer needed */
void RemoveList(vector<ChanEvent*> &eventList)
{
    /*
      using the iterator and starting from the beginning and going to 
      the end of eventlist, delete the actual objects
    */
    for(vector<ChanEvent*>::iterator it = eventList.begin();
	it != eventList.end(); it++) {
        delete *it;
    }
    
    // once the actual objects are deleted, clear the vector eventList
    eventList.clear();   
}

/** \brief event by event analysis
 * 
 * ScanList() operates on the time sorted list of all channels that triggered in
 * a given spill.  Starting from the begining of the list and continuing to the
 * end, an individual channel event time is compared with the previous channel
 * event time to determine if they occur within a time period defined by the
 * diff_t variable (time is in units of 10 ns).  Depending on the answer,
 * different actions are performed:
 *   - yes - the two channels are grouped together as belonging to the same event
 *   and the current channel is added to the list of channels for the rawevent 
 *   - no - the previous rawevent is sent for processing and once finished, the
 *   rawevent is zeroed and the current channel placed inside it.
 */

void ScanList(vector<ChanEvent*> &eventList) 
{
    /** The time width of an event in units of pixie16 clock ticks */
    const int eventWidth = 200;

    unsigned long chanTime, eventTime;

    // local variable for the detectors used in a given event
    set<string> usedDetectors;
    
    vector<ChanEvent*>::iterator iEvent = eventList.begin();

    // local variables for the times of the current event, previous
    // event and time difference between the two
    double diffTime = 0;
    
    //set last_t to the time of the first event
    double lastTime = (*iEvent)->GetTime();
    double currTime = lastTime;
    unsigned int id = (*iEvent)->GetID();

    HistoStats(id, diffTime, lastTime, BUFFER_START);

    //loop over the list of channels that fired in this buffer
    for(; iEvent != eventList.end(); iEvent++) { 
	id        = (*iEvent)->GetID();
	if (id == U_DELIMITER) {
	  cout << "pattern 0 ignore" << endl;
	  continue;
	}
	if (id > numModules * NUMBER_OF_CHANNELS) {
	  cout << "Unexpected channel id " << id << endl;
	  Pixie16Error(1);
	}
	if (modChan[id].GetType() == "ignore") {
	  continue;
	}

	// this is a channel we're interested in
	chanTime  = (*iEvent)->GetTrigTime(); 
	eventTime = (*iEvent)->GetEventTimeLo();

       /* retrieve the current event time and determine the time difference 
	   between the current and previous events. 
        */
	currTime = (*iEvent)->GetTime();
        diffTime = currTime - lastTime;

        /* if the time difference between the current and previous event is 
	   larger than the event width, finalize the current event, otherwise
	   treat this as part of the current event
        */
	if ( diffTime > eventWidth ) {
            if(rawev.Size()>0) {
		/* detector driver accesses rawevent externally in order to
		   have access to proper detector_summaries
		*/
                driver.ProcessEvent(scanMode);
            }
 
            //after processing zero the rawevent variable
            rawev.Zero(usedDetectors);
            usedDetectors.clear();	    

	    HistoStats(id, diffTime, currTime, EVENT_START);
	} else HistoStats(id, diffTime, currTime, EVENT_CONTINUE);
	unsigned long dtimebin = 2000 + eventTime - chanTime;
	if (dtimebin < 0 || dtimebin > (unsigned)(SE)) {
	    cout << "strange dtime for id " << id << ":" << dtimebin << endl;
	}
	plot(dammIds::misc::offsets::D_TIME + id, dtimebin);

	usedDetectors.insert(modChan[id].GetType());
	rawev.AddChan(*iEvent);
	    
        lastTime = currTime; // update the time of the last event
    } //end loop over event list

    //process the last event in the buffer
    if( rawev.Size()>0 ) {
	string mode;
        HistoStats(id, diffTime, currTime, BUFFER_END);

	driver.ProcessEvent(scanMode);
	rawev.Zero(usedDetectors);
    }
}

/**
 * At various points in the processing of data in ScanList(), HistoStats() is
 * called to increment some low level pixie16 informational and diagnostic
 * spectra.  The list of spectra filled includes runtime in second and
 * milliseconds, the deadtime, time between events, and time width of an event.
 */
void HistoStats(unsigned int id, double diff, double clock, HistoPoints event)
{
    using namespace dammIds::misc;

    static const int specNoBins = SE;

    static double start, stop;
    static int count;
    static double firstTime = 0.;
    static double bufStart;

    double runTimeSecs   = (clock - firstTime) * pixie::clockInSeconds;
    int    rowNumSecs    = int(runTimeSecs / specNoBins);
    double remainNumSecs = runTimeSecs - rowNumSecs * specNoBins;

    double runTimeMsecs   = runTimeSecs * 1000;
    int    rowNumMsecs    = int(runTimeMsecs / specNoBins);
    double remainNumMsecs = runTimeMsecs - rowNumMsecs * specNoBins;

    static double bufEnd = 0, bufLength = 0;
    // static double deadTime = 0 // not used

    if (firstTime > clock) {
	cout << "Backwards clock jump detected: prior start " << firstTime
	     << ", now " << clock << endl;
	// detect a backwards clock jump which occurs when some of the
	//   last buffers of a previous run sneak into the beginning of the 
	//   next run, elapsed time of last buffers is usually small but 
	//   just in case make some room for it
	double elapsed = stop - firstTime;
	// make an artificial 10 second gap by 
	//   resetting the first time accordingly
	firstTime = clock - 10 / pixie::clockInSeconds - elapsed;
	cout << elapsed*pixie::clockInSeconds << " prior seconds elapsed "
	     << ", resetting first time to " << firstTime << endl;	
    }

    switch (event) {
	case BUFFER_START:
	    bufStart = clock;
	    if(firstTime == 0.) {
		firstTime = clock;
	    } else if (bufLength != 0.){
		//plot time between buffers as a function of time - dead time spectrum	    
		// deadTime += (clock - bufEnd)*pixie::clockInSeconds;
		// plot(DD_DEAD_TIME_CUMUL,remainNumSecs,rownum,int(deadTime/runTimeSecs));	    	    
		plot(DD_BUFFER_START_TIME,remainNumSecs,rowNumSecs,
		     (clock-bufEnd)/bufLength*1000. );	    
	    }
	    break;
	case BUFFER_END:
	    plot(D_BUFFER_END_TIME, (stop - bufStart) * pixie::clockInSeconds * 1000);
	    bufEnd = clock;
	    bufLength = clock - bufStart;
	case EVENT_START:
	    plot(D_EVENT_LENGTH, stop - start); // plot the last event
	    plot(D_EVENT_GAP, diff);
	    plot(D_EVENT_MULTIPLICITY, count);
	    
	    start = stop = clock; // reset the counters      
	    count = 1;
	    break;
	case EVENT_CONTINUE:
	    count++;
	    if(diff > 0.) {
		plot(D_SUBEVENT_GAP,diff + 100);
	    }
	    stop = clock;
	    break;
	default:
	    cout << "Unexpected type " << event << " given to HistoStats" << endl;
    }

    //fill these spectra on all events, id plots and runtime.
    // Exclude event type 0/1 since it will also appear as an
    // event type 11
    if ( event != BUFFER_START && event != BUFFER_END ){      
	plot(DD_RUNTIME_SEC,remainNumSecs,rowNumSecs);
	plot(DD_RUNTIME_MSEC,remainNumMsecs,rowNumMsecs);      
	//fill scalar spectrum (per second) 
	plot(D_HIT_SPECTRUM,id);     // plot hit spectrum
	plot(offsets::D_SCALAR + id, runTimeSecs);
    }
}

/**
 * \brief Initialize the analysis
 *
 * Read in the map.txt file to determine what detector types are going to be used
 * in this analysis.  For each detector type used, create an entry into the map
 * of detector summaries in the raw event.  After this is completed, proceed to 
 * initialize each detector driver to complete the initialization.
 */
int InitMap(void) 
{
    /*
     * Local variables to store the types of detectors that are known to the
     * program (inspect DetectorDriver.cpp for more details), the detectors that
     * are used in this particular analysis, and the detector sub types used in
     * this particular analysis. If a detector type read in from map.txt does not
     * match one of the known types, then the program will terminate.
     */
    const set<string> &knownDets = driver.GetKnownDetectors();
    set<string> usedTypes;
    set<string> usedSubtypes;

    /*
      Local variables for reading in the map.txt file.  For each channel, the
      read variables are the module and channel number (from which the channel's
      unique id will be computed), the detector type and subtype, the spectrum
      number where the raw channel energies will be plotted (currently
      placeholder numbers), the physical detector location (think strip number
      or detector number) and an integer called trace which is not currently used.
    */
    unsigned int modNum, chanNum, dammID, detLocation, trace;
    string detType, detSubtype;

    ifstream mapFile("map.txt");

    if (!mapFile) {
        cout << "Can not open file 'map.txt'" << endl;
        return 1;
    }
      
    // read the map file until the end is reached
    while (mapFile) {
	/*
	 * If the first input on a line is a number, read in channel info.
	 * Otherwise, treat it as a comment.
	 */
	if ( isdigit(mapFile.peek()) ) {
	    mapFile >> modNum >> chanNum >> dammID >> detType
		    >> detSubtype >> detLocation >> trace;
	    
	    // Using the current module number, set the number of modules
	    numModules = max(numModules, modNum + 1);
	    // Resize the modchan vector so it's guaranteed to have enough chans
	    if ( modChan.size() < numModules * NUMBER_OF_CHANNELS ) {
		modChan.resize(numModules * NUMBER_OF_CHANNELS);
	    }  
	    Identifier& id = modChan.at(NUMBER_OF_CHANNELS * modNum + chanNum);
	    /*
	     * If you have not specified that this channel
	     * be ignored, process the information
	     */
	    if (detType != "ignore") {
		/*
		 * Search the list of known detectors; if the detector type 
		 * is not matched, print out an error message and terminate
		 */
		if(knownDets.find(detType) == knownDets.end()){
		    cout << endl;
		    cout << "The detector called '" << detType <<"'"<< endl
			 << "read in from the file 'map.txt'" << endl
			 << "is unknown to this program!.  This is a" << endl
			 << "fatal error.  Program execution halted!" << endl
			 << "If you believe this detector should exist," << endl
			 << "please edit the 'getKnownDetectors'" << endl
			 << "function inside the 'DetectorDriver.cpp' file" << endl
			 << endl;
		    cout << "The currently known detectors include:" << endl;
		    copy(knownDets.begin(), knownDets.end(), 
			 ostream_iterator<string>(cout, " "));
		    exit(EXIT_FAILURE);
		}
		
		/* if the type already has been set, something is wrong with
		 * in the map file.
		 */
		if (id.GetType() != "") {
		    cout << "Identifier for " << detType << "in module " 
			 << modNum << ": channel " << chanNum 
			 << " is initialized more than once in the map file."
			 << endl;
		    exit(EXIT_FAILURE);
		}
		/*
		 * The detector type from map.txt matched a detector listed in
		 * the set of known detectors. Load in the information into a
		 * class called an identifier which contains the detector type
		 * and subtype, detector location and dammid.
		 */
		id.SetDammID(dammID);
		id.SetType(detType);
		id.SetSubtype(detSubtype);
		id.SetLocation(detLocation);
		
		usedTypes.insert(detType);
		usedSubtypes.insert(detSubtype);
	    } // end != ignore condition                
	    else {
		/*
		 * This channel has been set to be ignored in the analysis.
		 * Put a dummy identifier into the identifier vector modchan
		 */
		id.SetType("ignore");
		id.SetSubtype("ignore");                   
	    }
	} // end if to see if the line begins with a digit
	else {
	    // This is a comment line in the map file. Skip the line.	   
	    mapFile.ignore(1000,'\n');
	}	
    } // end while (!mapfile) loop - end reading map.txt file
    
    mapFile.close();    
    
    // Print the number of detectors and detector subtypes used in the analysis
    cout << usedTypes.size() <<" detector types are used in this analysis " 
	 << "and are named:" << endl << "  "; 
    copy(usedTypes.begin(), usedTypes.end(), ostream_iterator<string>(cout, " "));
    cout << endl;
    
    cout << usedSubtypes.size() <<" detector subtypes are used in this "
	 << "analysis and are named:" << endl << "  ";
    copy(usedSubtypes.begin(), usedSubtypes.end(), ostream_iterator<string>(cout," "));
    cout << endl;
    
    rawev.Init(usedTypes, usedSubtypes);
    driver.Init();
    
    return(0);
}

/** \brief pixie16 scan error handling.
 *
 * Print out an error message and terminate program depending on value
 * of errorNum. 
 */
void Pixie16Error(int errorNum)
{
  //print error message depending on errornum
  switch (errorNum) {
      case 1:
	  cout << endl;
	  cout << " **************  SCAN ERROR  ****************** " << endl;
	  cout << "There appears to be more modules in the data " << endl;
	  cout << "stream than are present in the map.txt file. " << endl;
	  cout << "Please verify that the map.txt file is correct " << endl;
	  cout << "This is a fatal error, program terminating" << endl;
	  exit(EXIT_FAILURE);
      case 2:
	  cout << endl;
	  cout << "***************  SCAN ERROR  ******************* "<<endl;
	  cout << "One of the variables named nn, nk, or mm" << endl;
	  cout << "have exceeded the value of TOTALREAD. The value of" << endl;
	  cout << "TOTALREAD MUST NEVER exceed 1000000 for correct " << endl;
	  cout << "opertation of code between 32-bit and 64-bit architecture " << endl;
	  cout << "Either these variables have not been zeroed correctly or" << endl;
	  cout << "the poll program controlling pixie16 is trying to send too " << endl;
	  cout << "much data at once" << endl;
	  cout << "This is a fatal error, program terminating " << endl;
	  exit(EXIT_FAILURE);
  }
}
