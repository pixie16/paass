// LSM data acquisition for Rev. D crate -- PACOR-style
// last updated: March 2010, DTM

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include <cstring>

#include <errno.h>
#include <getopt.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>

// Needed for LIST_MODE_RUNx defs. Incorporate into PixieInterface
#include "pixie16app_defs.h"
#include "StatsHandler.hpp"

#include "Acq_Params.h"
#include "Buffer_Structure.h"
#include "acq_ctl.h"
#include "orph_udp.h"
#include "spkt_io_udp.h"

// Interface for the PIXIE-16
#include "Display.h"
#include "PixieInterface.h"
#include "Utility.h"

// only one kind of word we're interested in
typedef PixieInterface::word_t word_t;

// maximum size of the shared memory buffer
const int maxShmSizeL = 4050; // in pixie words
const int maxShmSize  = maxShmSizeL * sizeof(word_t); // in bytes

const size_t maxEventSize = (0x1FFE0000 >> 17);
const word_t clockVsn = 1000;
typedef word_t eventdata_t[maxEventSize];

using namespace std;
using namespace Display;

static volatile bool isInterrupted = false;

// function prototypes
static void interrupt(int sig);
bool SynchMods(PixieInterface &pif);
int SendData(word_t *data, size_t nWords);
void BailOut(bool sendAlarm, string arg); // bail out with EXIT_FAILURE sending an alarm if wanted

int main(int argc, char **argv)
{
  // boolean configuration variables
  bool bootFast = false;
  bool insertWallClock = true;
  bool isQuiet = false;
  bool sendAlarm = false;
  bool showModuleRates = false;
  bool zeroClocks = false;

  // read the FIFO when it is this full
  unsigned int threshPercent = 50;
  int statsInterval = -1; //< in seconds
  int histoInterval = -1; //< in seconds
  string alarmArgument;

  // compiled-in configuration
  const int listMode = LIST_MODE_RUN0; // full header w/ traces
  // values associated with the minimum timing between pixie calls (in us)
  const unsigned int endRunPause = 100;
  const unsigned int pollPause   = 1;
  const unsigned int readPause   = 10;
  const unsigned int waitPause   = 10;
  const unsigned int pollTries   = 100;
  const unsigned int waitTries   = 100;

  enum E_LONG_OPTIONS {E_NO_WALL_CLOCK};
  int longIndex, longSwitch, opt;
  const option longOptions[] = {
    {"alarm"        , optional_argument, NULL        , 'a' },
    {"no-wall-clock", no_argument      , &longSwitch , E_NO_WALL_CLOCK },
    {"quiet"        , no_argument      , NULL        , 'q' },
    {0, 0, 0, 0}
  };

  while ( (opt = getopt_long(argc,argv,"-a::fh:qrs:t:z?",longOptions, &longIndex)) != -1) {
    switch(opt) {
    case 0:
      switch(longSwitch) {
      case E_NO_WALL_CLOCK:
	insertWallClock = false; break;
      }
      break;
    case 'a':
      if (optarg) {
	alarmArgument = optarg;
      }      
      sendAlarm = true;
      break;
    case 'f':
      bootFast = true; break;
    case 'h':
      histoInterval = atoi(optarg); break;
    case 'q':
      isQuiet = true; break;
    case 'r':
      showModuleRates = true; break;
    case 's':
      statsInterval = atoi(optarg); break;
    case 't':
      threshPercent = atoi(optarg); break;
    case 'z':
      zeroClocks = true; break;
    case 1:
      cout << "Unexpected argument " << argv[optind -1] << endl;
    case '?':
    default:
      ; // Only to make emacs tab properly, grr.
      cout << "Usage: " << argv[0] << " [options]" << endl;
      cout << "  -a,--alarm=[e-mail] Call the alarm script with a given e-mail (or no argument)" << endl; 
      cout << "  -f                  Fast boot (false by default)" << endl;
      cout << "  -h <num>            Dump histogram data every num seconds" << endl;
      cout << "  -q, --quiet         Run quietly (false by default)" << endl;
      cout << "  --no-wall-clock     Do not insert the wall clock in the data stream" << endl;
      cout << "  -r                  Display module rates in quiet mode (false by defualt)" << endl;
      cout << "  -s <num>            Output statistics data every num seconds" << endl;
      cout << "  -t <num>            Sets FIFO read threshold to num% full ("
	   << threshPercent << "% by default)" << endl;
      cout << "  -z                  Zero clocks on each START_ACQ (false by default)" << endl;
      if (opt=='?')
	return EXIT_SUCCESS;
      return EXIT_FAILURE;
    }
  }

  const word_t threshWords = EXTERNAL_FIFO_LENGTH * threshPercent / 100;

  string pacHost;
  if (getenv("PACHOST") == NULL) {
      pacHost="127.0.0.1";
  } else {
      pacHost=getenv("PACHOST");
  }

  spkt_connect(pacHost.c_str(), PROTO_DATA);
  cout << "Connected to PAC-machine " << pacHost << endl;

  // open a socket for command communication
  LeaderPrint("Opening socket");
  if ( StatusPrint(spkt_open("", PROTO_FECNTRL) == -1) ) {
    // failed to open socket
    return EXIT_FAILURE;
  }

  // initialize the pixie interface and boot
  PixieInterface pif("pixie.cfg");
  pif.GetSlots();
  pif.Init();
  if (bootFast)
    pif.Boot(PixieInterface::DownloadParameters |
	     PixieInterface::SetDAC |
	     PixieInterface::ProgramFPGA);
  else
    pif.Boot(PixieInterface::BootAll);

  signal(SIGINT, interrupt);

  // check the scheduler
  LeaderPrint("Checking scheduler");
  int startScheduler = sched_getscheduler(0);
  if (startScheduler == SCHED_BATCH)
      cout << InfoStr("BATCH") << endl;
  else if (startScheduler == SCHED_OTHER)
      cout << InfoStr("STANDARD") << endl;
  else
      cout << WarningStr("UNEXPECTED") << endl;
  
  
  /*
  sched_params schedParams;
  schedParams.sched_priority = 0;
  if (sched_setscheduler(0, SCHED_BATCH, &schedParams) < 0) {
      perror("sched_setscheduler");
      return EXIT_FAILURE;
  } else cout << "Now using batch scheduler." << endl;
  */

  if (!SynchMods(pif))
    return EXIT_FAILURE;

  // allocate memory buffers for FIFO
  size_t nCards = pif.GetNumberCards();
  
  // two extra words to store size of data block and module number
  cout << "Allocating memory to store FIFO data ("
       << sizeof(word_t) * (EXTERNAL_FIFO_LENGTH + 2) * nCards / 1024 
       << " KiB)" << endl;
  word_t *fifoData = new word_t[(EXTERNAL_FIFO_LENGTH + 2) * nCards];
  // allocate data for partial events
  cout << "Allocating memory for partial events ("
       << sizeof(eventdata_t) * nCards / 1024
       << " KiB)" << endl;
  eventdata_t partialEventData[nCards];
  vector<word_t> partialEventWords(nCards);
  vector<word_t> waitWords(nCards);
  StatsHandler statsHandler(nCards);

  UDP_Packet command;
  command.DataSize = 100;

  bool isStopped = true;
  bool justEnded = false;
  // Enter data acquisition loop
  cout.setf(ios_base::showbase);

  size_t dataWords = 0;

  int commandCount = 0;
  double startTime;
  double spillTime, lastSpillTime, durSpill;
  double parseTime, waitTime, readTime, sendTime, pollTime;
  double lastStatsTime, statsTime = 0;
  double lastHistoTime, histoTime = 0;

  typedef pair<unsigned int, unsigned int> chanid_t;
  map<chanid_t, PixieInterface::Histogram> histoMap;

  if (histoInterval != -1.) {
      cout << "Allocating memory to store HISTOGRAM data ("
	   << sizeof(PixieInterface::Histogram) * nCards * pif.GetNumberChannels() / 1024
	   << " KiB)" << endl;
      
      for (unsigned int mod=0; mod < nCards; mod++) {
	  for (unsigned int ch=0; ch < pif.GetNumberChannels(); ch++) {
	      chanid_t id(mod, ch);
	      
	      histoMap[id] = PixieInterface::Histogram();
	  }
      }
  }

  PixieInterface::Histogram deltaHisto;

  bool runDone[nCards];
  bool isExiting = false;

  int waitCounter = 0, nonWaitCounter = 0;
  int partialBufferCounter = 0;

  time_t pollClock; 

  if (sendAlarm) {
    LeaderPrint("Sending alarms to");
    if ( alarmArgument.empty() ) {
      cout << InfoStr("DEFAULT") << endl;
    } else {
      cout << WarningStr(alarmArgument) << endl;
    }
  }

  // enter the data acquisition loop
  socket_poll(0); // clear the file descriptor set we poll

  while (!isExiting) {
    // see if we have any commands from pacman
    time_t curTime, prevTime;
    time(&curTime);

    // only update the display if the time has changed
    if ( isStopped && prevTime != curTime) {
      string timeString(ctime(&curTime));
      // strip the trailing newline
      timeString.erase(timeString.length()-1, 1);

      LeaderPrint("Waiting for START command");
      cout << InfoStr(timeString.c_str()) << '\r' << flush;
    }
    prevTime = curTime;

    if (isInterrupted) {
      if (isStopped) {
	cout << endl << CriticalStr("INTERRUPTED") << endl;
	isExiting = true;	
      } else {
	cout << CriticalStr("INTERRUPTED: Ending run") << endl;
	pif.EndRun();
	justEnded = true;
	isStopped = true;
	usleep(endRunPause);
      }
    }

    if (socket_poll(1)) {
      spkt_recv(&command);
	
      cout << endl << "receive command[" << commandCount++ << "] = ";
	   
      switch (command.Data[0]) {
      case INIT_ACQ:
	cout << "INIT_ACQ" << endl;
	// artificial, but i work with what pacman gives me
	command.Data[0] = ACQ_OK;
	if (isStopped)
	  isExiting = true;
	spkt_send(&command);
      case STOP_ACQ:
	cout << "STOP_ACQ" << endl;
	if (isStopped) {
	  command.Data[0] = ACQ_STP_HALT;
	} else {
	  command.Data[0] = ACQ_OK;
	  pif.EndRun();
	  justEnded = true; // to take one more run through to read remainder of FIFO
	  isStopped = true;
	  usleep(endRunPause);
	}
	spkt_send(&command);
	break;
      case STATUS_ACQ:
	cout << "STATUS_ACQ" << endl;
	command.Data[0] = ACQ_OK; 
	command.Data[1] = isStopped ? ACQ_STOP : ACQ_RUN;
	spkt_send(&command);
	break;
      case START_ACQ:
	cout << "START_ACQ" << endl;
	cout << "Processing start acquisition, isStopped = " << isStopped << endl; //debugging statement
	if (isStopped) {
	  command.Data[0] = ACQ_OK;
	  // reset variables
	  for (size_t mod=0; mod < nCards; mod++) {
	    runDone[mod] = false;
	  }
	  if (zeroClocks)
	    SynchMods(pif);
	  startTime = usGetTime(0);
	  lastHistoTime = lastStatsTime = lastSpillTime = usGetTime(startTime);
	  if ( pif.StartListModeRun(listMode, NEW_RUN) ) {
	    isStopped = false;
	    waitCounter = 0;
	    nonWaitCounter = 0;
	  }
	} else {
	  command.Data[0] = ACQ_STR_RUN;
	}
	spkt_send(&command);
	break;
      case ZERO_CLK:
	cout << "ZERO_CLK" << endl;
	command.Data[0] = ACQ_OK;
	spkt_send(&command);
	spkt_close();
	
	delete[] fifoData;
	return EXIT_SUCCESS;
      case PAC_FILE:
      case HOST:
	// not implemented
      default:
	cout << "WTF" << endl;
	// unrecognized command
	break;
      } // end switch
    } // if socket polled

    // wait for more commands
    if (isStopped && !justEnded)
      continue;

    vector<word_t> nWords(nCards);
    vector<word_t>::iterator maxWords;

    parseTime = waitTime = readTime = 0.;

    // check if it's time to dump statistics
    if ( statsInterval != -1 && 
	 usGetTime(startTime) > lastStatsTime + statsInterval * 1e6 ) {
      usGetDTime(); // start timer
      for (size_t mod = 0; mod < nCards; mod++) {
	pif.GetStatistics(mod);
	PixieInterface::stats_t &stats = pif.GetStatisticsData();
	fifoData[dataWords++] = PixieInterface::STAT_SIZE + 3;
	fifoData[dataWords++] = mod;
	fifoData[dataWords++] = 
	  ( (PixieInterface::STAT_SIZE + 1) << 17 ) & // event size
	  ( 1 << 12 ) & // header length
	  ( pif.GetSlotNumber(mod) << 4); // slot number
	memcpy(&fifoData[dataWords], &stats, sizeof(stats));
	dataWords += PixieInterface::STAT_SIZE;

	if (isQuiet)
	  cout << endl;

	cout << "STATS " << mod << " : ICR ";
	for (size_t ch = 0; ch < pif.GetNumberChannels(); ch++) {
	  cout.precision(2);
	  cout << " " << pif.GetInputCountRate(mod, ch);
	}
	cout << endl << flush;
      
      }
      SendData(fifoData, dataWords);
      dataWords = 0;
      statsTime += usGetDTime();
      lastStatsTime = usGetTime(startTime);
    }

    // check whether it is time to dump histograms
    if ( histoInterval != -1 &&
	 usGetTime(startTime) > lastHistoTime + histoInterval * 1e6 ) {
	usGetDTime(); // start timer
	ofstream out("histo.dat", ios::trunc);
	ofstream deltaout("deltahisto.dat", ios::trunc);
	
	for (size_t mod=0; mod < nCards; mod++) {
	    for (size_t ch=0; ch < pif.GetNumberChannels(); ch++) {
		chanid_t id(mod, ch);
		PixieInterface::Histogram &histo = histoMap[id];

		// copy the old histogram data to the delta histogram temporarily
		deltaHisto = histo;
		// performance improvement possible using Pixie16EMbufferIO directly to fetch all channels
		histo.Read(pif, mod, ch);
		histo.Write(out);
		// calculate the change using the temporarily stored previous histogram
		deltaHisto = histo - deltaHisto;
		deltaHisto.Write(deltaout);
	    }
	}
	out.close();
	deltaout.close();

	histoTime += usGetDTime();
	lastHistoTime = usGetTime(startTime);
    }

    // check whether we have any data
    usGetDTime(); // start timer
    for (unsigned int timeout = 0; timeout < (justEnded ? 1 : pollTries);
	 timeout++) {
      // see if the modules have any data for us

      for (size_t mod = 0; mod < nCards; mod++) {
	if (!runDone[mod]) {
	  nWords[mod] = pif.CheckFIFOWords(mod);
	} else {
	  nWords[mod] = 0;
	}
      }
      maxWords = max_element(nWords.begin(), nWords.end());
      if (*maxWords > threshWords)
	break;
      usleep(pollPause);
    }
    time(&pollClock);
    pollTime = usGetDTime();

    int maxmod = maxWords - nWords.begin();
    bool readData = ( *maxWords > threshWords || justEnded );

    if (readData) {
      // if not timed out, we have data to read	
      // read the data, starting with the module with the most words      
      int mod = maxmod;      
      mod = maxmod = 0; //! tmp, read out in a fixed order

      usleep(readPause);
      do {
	bool fullFIFO = (nWords[mod] == EXTERNAL_FIFO_LENGTH);

	if (nWords[mod] > 0) {
	  usGetDTime(); // start read timer
	  
	  word_t &bufferLength = fifoData[dataWords];

	  // fifoData[dataWords++] = nWords[mod] + 2
	  fifoData[dataWords++] = nWords[mod] + partialEventWords[mod] + 2;
	  fifoData[dataWords++] = mod;
	  word_t beginData = dataWords;

	  //? only add to fifo stream if we have enough words to complete event?
	  if (partialEventWords[mod] > 0) {
	      memcpy(&fifoData[dataWords], partialEventData[mod],
		     sizeof(word_t) * partialEventWords[mod]);
	      dataWords += partialEventWords[mod];
	      partialEventWords[mod] = 0;
	  }

	  if (!pif.ReadFIFOWords(&fifoData[dataWords], nWords[mod], mod)) {
	    cout << "Error reading FIFO, bailing out!" << endl;

	    spkt_close();
	    delete[] fifoData;

	    // something is wrong
	    BailOut(sendAlarm, alarmArgument);
	  } else {
	      word_t parseWords = beginData;
	      word_t eventSize;

	      waitWords[mod] = 0; // no longer waiting (hopefully)

	    readTime += usGetDTime(); // and starts parse timer
	    // unfortuantely, we have to parse the data to make sure 
	    //   we grabbed complete events
	    do {
	      word_t slotRead = ((fifoData[parseWords] & 0xF0) >> 4);
	      word_t chanRead = (fifoData[parseWords] & 0xF);
	      word_t slotExpected = pif.GetSlotNumber(mod);
	      bool virtualChannel = ((fifoData[parseWords] & 0x20000000) != 0);
	      
	      eventSize = ((fifoData[parseWords] & 0x1FFE0000) >> 17);
	      if (!virtualChannel)
		  statsHandler.AddEvent(mod, chanRead, sizeof(word_t) * eventSize);

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
		     << "\n  parse started at position " << beginData
		     << " reading " << nWords[mod] << " words." << endl;
		//! how to proceed from here
		delete[] fifoData;

		BailOut(sendAlarm, alarmArgument);
	      }
	      parseWords += eventSize;	      
	    } while ( parseWords < dataWords + nWords[mod]);	   
	    parseTime += usGetDTime();

	    if (parseWords > dataWords + nWords[mod]) {
	      waitCounter++;
	      // if we have ended the run, we should have all the words
	      if (justEnded) {
		cout << ErrorStr("Words missing at end of run.") << endl;

		delete[] fifoData;
		BailOut(sendAlarm, alarmArgument);
	      } else {
		// we have a deficit of words, now we must wait for the remainder
		if ( fullFIFO ) {
		  // the FIFO was full so the rest of the partial event is likely lost
		  parseWords -= eventSize;
		  // update the buffer length
		  nWords[mod]  = parseWords;
		  bufferLength = nWords[mod] + 2;
		} else {
		  waitWords[mod] = parseWords - (dataWords + nWords[mod]);
		  unsigned int timeout = 0;
		
		  usGetDTime(); // start wait timer
		  
		  if (!isQuiet) 
		    cout << "Waiting for " << waitWords[mod] << " words in module " << mod << flush;
		  
		  usleep(waitPause);
		  word_t testWords;

		  while (timeout++ < waitTries) {
		      testWords = pif.CheckFIFOWords(mod);
		      if ( testWords >= max(waitWords[mod], 2U) )
			  break;
		      usleep(pollPause);
		  } 
		  waitTime += usGetDTime();
		  
		  if (timeout >= waitTries) {
		    if (!isQuiet)
		      cout << " --- TIMED OUT," << endl
			   << InfoStr("    moving partial event to next buffer") << endl;

		    partialBufferCounter++;
		    partialEventWords[mod] = eventSize - waitWords[mod];
		    memcpy(partialEventData[mod], 
			   &fifoData[dataWords + nWords[mod] - partialEventWords[mod]],
			   sizeof(word_t) * partialEventWords[mod]);
		    nWords[mod] = parseWords - beginData - eventSize;

		    // update the size of the buffer;
		    bufferLength = nWords[mod] + 2;
		  } else {
		    if (!isQuiet)
		      cout << endl;
		    usleep(readPause);
		    int testWords = pif.CheckFIFOWords(mod);
		    if ( !pif.ReadFIFOWords(&fifoData[dataWords + nWords[mod]],
					    waitWords[mod], mod) ) {
		      cout << "Error reading FIFO, bailing out!" << endl;
		      spkt_close();

		      delete[] fifoData;

		      BailOut(sendAlarm, alarmArgument);
		      // something is wrong 
		    } else {
		      nWords[mod] += waitWords[mod];
		      // no longer waiting for words
		      waitWords[mod] = 0;
		      // and update the length of the buffer
		      bufferLength = nWords[mod] + 2;
		    } // check success of read
		  } // if we DID NOT time out waiting for words
		} // if we DID NOT have a full FIFO
	      } // if we ARE NOT on the final read at the end of a run
	    } // if there are words remaining  
	    else {
	      nonWaitCounter++;
	    }
	  } // check success of read
	} else { // if we had any words
	  // write an empty buffer if there is no data
	  fifoData[dataWords++] = 2;
	  fifoData[dataWords++] = mod;	    
	}
	if (nWords[mod] > EXTERNAL_FIFO_LENGTH * 9 / 10) {
	  cout << "Abnormally full FIFO with " << nWords[mod] 
	       << " words in module " << mod << endl;

	  if (fullFIFO) {
	      pif.EndRun();
	      justEnded = true;
	      isStopped = true;
	  }
	}
	if (!isQuiet) {
	  if (fullFIFO)
	    cout << "Read " << WarningStr("full FIFO") << " in";
	  else 
	    cout << "Read " << nWords[mod] << " words from";
	  cout << " module " << mod 
	       << " to buffer position " << dataWords;
	  if (partialEventWords[mod] > 0) 
	      cout << ", " << partialEventWords[mod] << " words reserved.";
	  cout << endl;
	}
	dataWords += nWords[mod];
	
	// read the remainder of the modules in a modulo ring
	mod = (mod + 1) % nCards;
      } while ( mod != maxmod );
    } // if we have data to read 
    
    // update whether the run has ended with the data read out
    for (size_t mod = 0; mod < nCards; mod++) {
      if (!justEnded && !pif.CheckRunStatus(mod)) {
	runDone[mod] = true;
	cout << "Run ended in module " << mod << endl;
      }
      if (justEnded && pif.CheckRunStatus(mod)) {
	cout << "Run not properly finished in module " << mod << endl;
      }
    }

    // if we don't have enough words, poll socket and modules once more
    if (!readData)
	continue;

    if (insertWallClock) {
	// add the "wall time" in artificially
	size_t timeWordsNeeded = sizeof(time_t) / sizeof(word_t);
	if ( (sizeof(time_t) % sizeof(word_t)) != 0 )
	    timeWordsNeeded++;
	fifoData[dataWords++] = 2 + timeWordsNeeded;
	fifoData[dataWords++] = clockVsn;
	memcpy(&fifoData[dataWords], &pollClock, sizeof(time_t));
	if (!isQuiet)
	    cout << "Read " << timeWordsNeeded << " words for time to buffer position " << dataWords << endl;
	dataWords += timeWordsNeeded;
    }

    spillTime = usGetTime(startTime);
    durSpill = spillTime - lastSpillTime;
    lastSpillTime = spillTime;

    usGetDTime(); // start send timer
    int nBufs = SendData(fifoData, dataWords);
    sendTime = usGetDTime();

    statsHandler.AddTime(durSpill * 1e-6);

    if (!isQuiet) {
      cout << nBufs << " BUFFERS with " << dataWords << " WORDS, " << endl;
      cout.setf(ios::scientific, ios::floatfield);
      cout.precision(1);
      cout << "    SPILL " << durSpill << " us "
	   << " POLL  " << pollTime << " us "
	   << " PARSE " << parseTime << " us" << endl
	   << "    WAIT  " << waitTime << " us "
	   << " READ  " << readTime << " us "
	   << " SEND  " << sendTime << " us" << endl;	 
      // add some blank spaces so STATS or HISTO line up
      cout << "   ";
      if (statsInterval != -1) {
	  cout << " STATS " << statsTime << " us ";
      }
      if (histoInterval != -1) {
	  cout << " HISTO " << histoTime << " us ";
      }
      if (statsInterval != -1 || histoInterval != -1) {
	  cout << endl;
      }

      cout << endl;
    } else {
      cout.setf(ios::scientific, ios::floatfield);
      cout.precision(1);

      if (!showModuleRates) {
	  cout << nBufs << " bufs : " 
	       << "SEND " << sendTime << " / SPILL " << durSpill << "     \r";
      } else {      
	  for (size_t i=0; i < nCards; i++) {
	      cout << "M" << i << ", "
		   << statsHandler.GetEventRate(i) / 1000. << " kHz";
	      cout << " (" << statsHandler.GetDataRate(i) / 1000000. << " MB/s)";
	  }  
	  cout << "    \r";
      }
    }
    // reset the number of words of fifo data
    dataWords = 0;
    histoTime = statsTime = 0;
    justEnded = false;
  }

  spkt_close();
  // deallocate memory
  delete[] fifoData;

  if (waitCounter + nonWaitCounter != 0) {
    cout << "Waiting for " << waitCounter * 100 / (waitCounter + nonWaitCounter)
	 << "% of the spills." << endl;
    cout << "  " << partialBufferCounter << " partial buffers" << endl;
  }

  return EXIT_SUCCESS;
}

static void interrupt(int sig)
{
  isInterrupted = true;
}

bool SynchMods(PixieInterface &pif)
{
  static bool firstTime = true;
  static char synchString[] = "IN_SYNCH";
  static char waitString[] = "SYNCH_WAIT";

  bool hadError = false;
   
  LeaderPrint("Synchronizing");

  if (firstTime) {
    // only need to set this in the first module once
    if (!pif.WriteSglModPar(waitString, 1, 0))
      hadError = true;
    firstTime = false;
  }
  
  for (unsigned int mod = 0; mod < pif.GetNumberCards(); mod++)
    if (!pif.WriteSglModPar(synchString, 0, mod))
      hadError = true;

  if (!hadError)
    cout << OkayStr() << endl;
  else
    cout << ErrorStr() << endl;

  return !hadError;
}

int SendData(word_t *data, size_t nWords)
{
  static data_pack acqBuf;
  const unsigned int sendPause = 1;

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
      usleep(sendPause);
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
      usleep(sendPause);
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

void BailOut(bool sendAlarm, string arg)
{
  if (sendAlarm) {
    stringstream str;
    str << "./send_alarm";
    if ( !arg.empty() ) {
      str << " " << arg;
    }
    system(str.str().c_str());
  }
  exit(EXIT_FAILURE);
}
