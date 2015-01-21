// False Pixie-16 Rev. D data with controllable rate -- PACOR-style
// last updated: March 2011, DTM

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#include <cmath> // for pow
#include <cstring>

#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>

#include "Acq_Params.h"
#include "Buffer_Structure.h"
#include "acq_ctl.h"
#include "orph_udp.h"
#include "spkt_io_udp.h"

// Interface for the PIXIE-16
#include "Display.h"
#include "PixieInterface.h"
#include "Utility.h"

// some defs borrowed from the Pixie Interface
typedef uint32_t word_t;
typedef uint16_t traceword_t;

// maximum size of the shared memory buffer
const int maxShmSizeL = 4050; // in pixie words
const int maxShmSize  = maxShmSizeL * sizeof(word_t); // in bytes

char *VME = "192.168.13.248";
// char *VME = "192.168.100.5";
// const char *VME = "127.0.0.1";
// char *VME = "192.168.1.100";

using namespace std;
using namespace Display;

static volatile bool isInterrupted = false;

// function prototypes
static void interrupt(int sig);
int SendData(word_t *data, size_t nWords);

int main(int argc, char **argv)
{
  bool quiet = false;
  bool zeroClocks = false;
  const int listMode = LIST_MODE_RUN; // full header w/ traces
  // read the FIFO when it is this full
  const unsigned int maxEnergy = 10000;
  const traceword_t baseline = 500;

  unsigned int threshPercent = 50;
  double rate = 10000;
  unsigned int traceLength = 200;
  unsigned int traceDelay  = 50;
  unsigned int pulserPeaks = 10;

  // values associated with the minimum timing between pixie calls (in us)
  // keep these the same for consistency between the programs
  const unsigned int endRunPause = 100;
  const unsigned int pollPause   = 1;
  const unsigned int readPause   = 10;
  const unsigned int waitPause   = 10;
  const unsigned int pollTries   = 100;

  int opt;

  while ( (opt = getopt(argc,argv,"d:l:p:qr:t:z?")) != -1) {
    switch(opt) {
    case 'd':
      traceDelay = atoi(optarg); break;
    case 'l':
      traceLength = atoi(optarg); break;
    case 'p':
      pulserPeaks = atoi(optarg); break;
    case 'q':
      quiet = true; break;
    case 'r':
      rate = atof(optarg); break;
    case 't':
      threshPercent = atoi(optarg); break;
    case 'z':
      zeroClocks = true; break;
    case '?':
    default:
      cout << "Usage: " << argv[0] << " [options]" << endl;
      cout << "  -d <num> Trace delay (in clock samples, 50 default)" << endl;
      cout << "  -l <num> Trace length (in clock samples, 200 default)" << endl;
      cout << "  -p <num> Number of pulser peaks to generate" << endl;      
      cout << "  -q       Run quietly (false by default)" << endl;
      cout << "  -r <num> Rate of events (in hertz, 10 kHz default)" << endl;
      cout << "  -t <num> Sets FIFO read threshold to num% full ("
	   << threshPercent << "% by default)" << endl;
      cout << "  -z       Zero clocks on each START_ACQ (false by default)" << endl;
      return EXIT_FAILURE;
    }
  }

  const word_t threshWords = EXTERNAL_FIFO_LENGTH * threshPercent / 100;

  spkt_connect(VME, PROTO_DATA);
  cout << "Connected to PAC-machine " << VME << endl;

  // open a socket for command communication
  LeaderPrint("Opening socket");
  if ( StatusPrint(spkt_open("", PROTO_FECNTRL) == -1) ) {
    // failed to open socket
    return EXIT_FAILURE;
  }

  signal(SIGINT, interrupt);

  // allocate memory buffers for FIFO
  const size_t nCards = 1;

  // two extra words to store size of data block and module number
  cout << "Allocating memory to store FIFO data ("
       << sizeof(word_t) * (EXTERNAL_FIFO_LENGTH + 2) * nCards / 1024 
       << " KB)" << endl;
  word_t *fifoData = new word_t[(EXTERNAL_FIFO_LENGTH + 2) * nCards];
  word_t *fauxData = new word_t[EXTERNAL_FIFO_LENGTH * nCards];
  vector<word_t> nWords(nCards);
  vector<word_t>::iterator maxWords;

  UDP_Packet command;
  command.DataSize = 100;

  bool isStopped = true;
  bool justEnded = false;
  // Enter data acquisition loop
  cout.setf(ios_base::showbase);

  size_t dataWords = 0;

  int commandCount = 0;
  double startTime, aTime;
  double clockTime = usGetTime(0); // this is the base time for our faux data
  double spillTime, lastSpillTime, durSpill;
  double parseTime, waitTime, readTime, sendTime, pollTime;
  double lastData;
  int currentPeak = 0;
  // convert from hertz to number of microseconds in between data dumps
  double deltaTime = 1. / rate * 1e6;
  LeaderPrint("Delta time (in us)");
  cout << deltaTime << endl;
  // calculate how long it should take to fill a buffer
  const unsigned int headerLength = 4;
  const unsigned int eventLength  = headerLength + traceLength / 2;
  LeaderPrint("Calculated spill time (in us)");
  double calcSpillTime = threshWords / eventLength * deltaTime;
  cout << calcSpillTime << endl;

  bool runDone[nCards];
  bool isExiting = false;

  int waitCounter = 0, nonWaitCounter = 0;

  // enter the data acquisition loop
  socket_poll(0); // clear the file descriptor set we poll

  while (!isExiting) {
    // see if we have any commands from pacman
    time_t curTime, prevTime;
    time(&curTime);

    // only update the display if the time has changed
    if ( isStopped && prevTime != curTime) {
      char timeStr[30];
      
      ctime_r(&curTime, timeStr);
      timeStr[20] = '\0';

      LeaderPrint("Waiting for START command");
      cout << InfoStr(timeStr) << '\r' << flush;
    }
    prevTime = curTime;

    if (isInterrupted) {
      if (isStopped) {
	cout << endl << CriticalStr("INTERRUPTED") << endl;
	isExiting = true;	
      } else {
	cout << CriticalStr("INTERRUPTED: Ending run") << endl;
	justEnded = true;
	isStopped = true;
	usleep(endRunPause);
      }
    }

    if (socket_poll(1)) {
      spkt_recv(&command);
	
      cout << endl << "receive command[" << commandCount++ 
	   << "] = " << hex << (int)command.Data[0] << dec << endl;

      switch (command.Data[0]) {
      case INIT_ACQ:
	// artificial, but i work with what pacman gives me
	command.Data[0] = ACQ_OK;
	if (isStopped)
	  isExiting = true;
	spkt_send(&command);
      case STOP_ACQ:
	if (isStopped) {
	  command.Data[0] = ACQ_STP_HALT;
	} else {
	  command.Data[0] = ACQ_OK;
	  justEnded = true; // to take one more run through to read remainder of FIFO
	  isStopped = true;
	  usleep(endRunPause);
	}
	spkt_send(&command);
	break;
      case STATUS_ACQ:
	command.Data[0] = ACQ_OK; 
	command.Data[1] = isStopped ? ACQ_STOP : ACQ_RUN;
	spkt_send(&command);
	break;
      case START_ACQ:
	if (isStopped) {
	  command.Data[0] = ACQ_OK;
	  // reset variables
	  for (size_t mod=0; mod < nCards; mod++) {
	    runDone[mod] = false;
	  }
	  if (zeroClocks) {
	    clockTime = usGetTime(0);
	  }
	  lastSpillTime = startTime = usGetTime(0);
	  lastData = usGetTime(clockTime);

	  currentPeak = 0;
	  nWords.assign(nWords.size(), 0); // clear our faux fifos

	  isStopped = false;
	  waitCounter = 0;
	  nonWaitCounter = 0;
	} else {
	  command.Data[0] = ACQ_STR_RUN;
	}
	spkt_send(&command);
	break;
      case ZERO_CLK:
	command.Data[0] = ACQ_OK;
	spkt_send(&command);
	spkt_close();
	
	for (size_t mod=0; mod < nCards; mod++) {	      
	  delete[] fifoData;
	}
	return EXIT_SUCCESS;
      default:
	// unrecognized command
	break;
      } // end switch
    } // if socket polled

    // wait for more commands
    if (isStopped && !justEnded)
      continue;

    parseTime = waitTime = readTime = 0.;

    // check whether we have any data
    usGetDTime(); // start timer
    
    for (unsigned int timeout = 0; timeout < (justEnded ? 1 : pollTries);
	 timeout++) {
      // make some faux data
      const unsigned int channel = 0;
      const unsigned int crate   = 0;

      while (usGetTime(clockTime) > lastData + deltaTime) {
	static bool first = true;
	// aTime = usGetTime(clockTime);
	aTime = lastData + deltaTime;
	if (first) {
	  cout << "First time = " << aTime * 100. << endl;
	  first = false;
	}
	for (size_t i=0; i < nCards; i++) {	  
	  
	  word_t *dataptr = &fauxData[EXTERNAL_FIFO_LENGTH * i + nWords[i]];
	  unsigned int slot = 2 + i;
	  word_t energy = (currentPeak + 1 ) * maxEnergy / pulserPeaks;
	  
	  *dataptr++ = (channel) + (slot << 4) + (crate << 8) + (headerLength << 12) + (eventLength << 17) + (0 << 31);	  
	  word_t hiTime = word_t(aTime * 100. / pow(2., 32));
	  word_t loTime = word_t(aTime * 100 - hiTime * pow(2., 32));
	  *dataptr++ = loTime;
	  *dataptr++ = hiTime;
	  *dataptr++ = energy + (traceLength << 16);	  
	  traceword_t *traceptr = (traceword_t *)dataptr;
	  for (size_t j=0; j < traceLength; j++) {
	    if (j < traceDelay) {
	      *traceptr++ = baseline;
	    } else {
	      // dividing energy by 4 puts the trace roughly in the same dynamic range as pixie
	      *traceptr++ = baseline + energy / 4; 	     
	    }
	  }
	  nWords[i] += eventLength;
	}
	currentPeak = (currentPeak + 1) % pulserPeaks;
	lastData = aTime;
      }

      maxWords = max_element(nWords.begin(), nWords.end());
      if (*maxWords > threshWords)
	break;
      usleep(pollPause);
    }
    pollTime = usGetDTime();

    int maxmod = maxWords - nWords.begin();
    
    if ( *maxWords > threshWords || justEnded ) {
      // if not timed out, we have data to read	
      // read the data, starting with the module with the most words      
      int mod = maxmod;      
      mod = maxmod = 0; //! tmp, read out in a fixed order

      usleep(readPause);
      do {
	bool fullFIFO = (nWords[mod] == EXTERNAL_FIFO_LENGTH);

	if (nWords[mod] > 0) {
	  usGetDTime(); // start read timer
	  
	  fifoData[dataWords++] = nWords[mod] + 2;
	  fifoData[dataWords++] = mod;
	
	  // faux FIFO read
	  memcpy(&fifoData[dataWords], &fauxData[EXTERNAL_FIFO_LENGTH * mod], nWords[mod] * sizeof(word_t));

	  word_t parseWords = dataWords;
	  word_t eventSize;
	
	  readTime += usGetDTime(); // and starts parse timer
	  // unfortuantely, we have to parse the data to make sure 
	  //   we grabbed complete events
	  do {
	    word_t slotRead = ((fifoData[parseWords] & 0xF0) >> 4);
	    // PixieInterface::word_t chanRead = (fifoData[parseWords] & 0xF);
	    word_t slotExpected = mod + 2; // faux slot expected

	    eventSize = (fifoData[parseWords] & 0x7FFE0000) >> 17;
	    if (eventSize == 0 || slotRead != slotExpected ) {
	      if ( slotRead != slotExpected )
		cout << "Slot read (" << slotRead << ") not the same as"
		     << " module expected (" << slotExpected << ")" << endl;
	      if (eventSize == 0)
		cout << "ZERO EVENT SIZE" << endl;
	      cout << "First header words: " << hex << fifoData[parseWords] 
		   << " " << fifoData[parseWords + 1] 
		   << " " << fifoData[parseWords + 2]
		   << " at position " << dec << parseWords 
		   << "\n  parse started at position " << dataWords 
		   << " reading " << nWords[mod] << " words." << endl;
	      //! how to proceed from here
	      return EXIT_FAILURE;
	    }
	    parseWords += eventSize;	      
	  } while ( parseWords < dataWords + nWords[mod]);	   
	  parseTime += usGetDTime();

	  if (parseWords > dataWords + nWords[mod]) {
	    // this can never happen in the faux readout
	    // might want to add a faux hiccup here to test the waiting scheme
	    if (true) {
	      cout << ErrorStr("Words missing in faux readout.") << endl;
	      return EXIT_FAILURE;
	    } else {
	      /* ignore this for now since it never will happen
	      // we have a deficit of words, now we must wait for the remainder
	      if ( fullFIFO ) {
		// the FIFO was full so the rest of the partial event is likely lost
		parseWords -= eventSize;
		// update the buffer length
		nWords[mod] = parseWords;
		fifoData[dataWords - 2] = nWords[mod] + 2;
	      } else {
		word_t readWords = parseWords - (dataWords + nWords[mod]);
		unsigned int timeout = 0;
		
		usGetDTime(); // start wait timer
		  
		if (!quiet) 
		  cout << "Waiting for " << readWords << " words in module " << mod << flush;
		
		usleep(waitPause);
		do {
		  if ( pif.CheckFIFOWords(mod) >= max(readWords, 2U) )
		    break;
		  usleep(pollPause);
		} while (timeout++ < pollTries);
		waitTime += usGetDTime();
		
		if (timeout == pollTries) {
		  if (!quiet)
		    cout << " --- TIMED OUT" << endl;
		} else {
		  if (!quiet)
		    cout << endl;
		    usleep(readPause);
		    int testWords = pif.CheckFIFOWords(mod);
		    if ( !pif.ReadFIFOWords(&fifoData[dataWords + nWords[mod]],
					    readWords, mod) ) {
		      cout << "Error reading FIFO, bailing out!" << endl;
		      spkt_close();
		      delete[] fifoData;
		      return EXIT_FAILURE;
		      // something is wrong 
		    } else {
		      if (readWords == 1 && !quiet) {
			// currently Pixie behaviour is a bit bizarre when
			//   reading one word from the FIFO (see Interface).
			//   hence we try to keep track of it
			cout << "One word read of " << testWords << " : " 
			     << hex
			     <<  fifoData[dataWords + nWords[mod]] 
			     << dec << endl;
		      }
		      nWords[mod] += readWords;
		      // and update the length of the buffer
		      fifoData[dataWords - 2] = nWords[mod] + 2;
		    }
		  } // if we DID NOT time out waiting for words
		} // if we DID NOT have a full FIFO
	      */
	    } // if we ARE NOT on the final read at the end of a run
	      
	  } // if there are words remaining  
	  else {
	    nonWaitCounter++;
	  }
	} else { // if we had any words
	  // write an empty buffer if there is no data
	  fifoData[dataWords++] = 2;
	  fifoData[dataWords++] = mod;	    
	}
	if (nWords[mod] > EXTERNAL_FIFO_LENGTH * 9 / 10) {
	  cout << "Abnormally full FIFO with " << nWords[mod] 
	       << " words in module " << mod 
	       << ". Ending run" << endl;
	  justEnded = true;
	  isStopped = true;
	}
	if (!quiet) {
	  if (fullFIFO)
	    cout << "Read " << WarningStr("full FIFO") << " in";
	  else 
	    cout << "Read " << nWords[mod] << " words from";
	  cout << " module " << mod 
	       << " to buffer position " << dataWords << endl;
	}
	dataWords += nWords[mod];

	// read the remainder of the modules in a modulo ring
	mod = (mod + 1) % nCards;
      } while ( mod != maxmod );
    } // if we have data to read 
    
    // update whether the run has ended with the data read out
    for (size_t mod = 0; mod < nCards; mod++) {
      if (justEnded) {
	runDone[mod] = true;
      }
    }

    // if we don't have enough words, poll socket and modules once more
    if (*maxWords <= threshWords && !justEnded)
      continue;
    
    // Clear our faux FIFO
    for (int mod=0; mod < nCards; mod++) {
      nWords[mod] = 0;
    }
    spillTime = usGetTime(startTime);
    durSpill = spillTime - lastSpillTime;
    lastSpillTime = spillTime;

    usGetDTime(); // start send timer
    int nBufs = SendData(fifoData, dataWords);
    sendTime = usGetDTime();

    if (!quiet) {
      cout << nBufs << " BUFFERS with " << dataWords << " WORDS, " << endl;
      cout.setf(ios::scientific, ios::floatfield);
      cout.precision(1);
      cout << "    SPILL " << durSpill << " us,"
	   << " POLL  " << pollTime << " us,"
	   << " PARSE " << parseTime << " us," << endl
	   << "    WAIT  " << waitTime << " us,"
	   << " READ  " << readTime << " us,"
	   << " SEND  " << sendTime << " us" << endl;
      /* no stats 
      if (statsInterval != -1) {
	cout << " STATS  " << statsTime << " us" << endl;
      }
      */
      cout << endl;
    } else {
      cout.setf(ios::scientific, ios::floatfield);
      cout.precision(1);
      cout << nBufs << " bufs : " 
	   << "SEND " << sendTime << " / SPILL " << durSpill << "     \r";
    }
    // reset the number of words of fifo data
    dataWords = 0;
    // statsTime = 0;
    justEnded = false;
  }

  spkt_close();
  // deallocate memory
  delete[] fifoData;

  if (waitCounter + nonWaitCounter != 0) {
    cout << "Waiting for " << waitCounter * 100 / (waitCounter + nonWaitCounter)
	 << "% of the spills." << endl;
  }

  return EXIT_SUCCESS;
}

static void interrupt(int sig)
{
  isInterrupted = true;
}

int SendData(word_t *data, size_t nWords)
{
  static data_pack acqBuf;
  
  // now write to the shared memory
  unsigned int nBufs     = nWords / maxShmSizeL;
  unsigned int wordsLeft = nWords % maxShmSizeL;
  
  unsigned int totalBufs = nBufs + 1 + ((wordsLeft != 0) ? 1 : 0);
    
  word_t *pWrite = (word_t *)acqBuf.Data;
  // chop the data and send it through the network
  for (size_t buf=0; buf < nBufs; buf++) {
    // get a long sized pointer to the data block for writing
    // put a header on each shared memory buffer
    acqBuf.BufLen = (maxShmSizeL + 3) * sizeof(word_t);
    pWrite[0] = acqBuf.BufLen; // size
    pWrite[1] = totalBufs; // number of buffers we expect
    pWrite[2] = buf;
    memcpy(&pWrite[3], &data[buf * maxShmSizeL], maxShmSize);
      
    send_buf(&acqBuf);
  }
      
  // send the last fragment (if there is any)
  if (wordsLeft != 0) {
    acqBuf.BufLen = (wordsLeft + 3) * sizeof(word_t);
    pWrite[0] = acqBuf.BufLen;
    pWrite[1] = totalBufs;
    pWrite[2] = nBufs;
    memcpy(&pWrite[3], &data[nBufs * maxShmSizeL], 
	   wordsLeft * sizeof(word_t) );
    
    send_buf(&acqBuf);
  }
    
  // send a buffer to say that we are done
  acqBuf.BufLen = 5 * sizeof(word_t);
  pWrite[0] = acqBuf.BufLen;
  pWrite[1] = totalBufs;
  pWrite[2] = totalBufs - 1;
  // pacman looks for the following data
  pWrite[3] = 0x2; 
  pWrite[4] = 0x270f; // vsn 9999
  
  send_buf(&acqBuf);  

  return nBufs;
}
