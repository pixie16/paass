#include <iostream>
#include <set>

#include <ctime>

#include <unistd.h>
#include <sys/times.h>

#include <XiaData.hpp>


#include "DetectorDriver.hpp"
#include "Places.hpp"
#include "TreeCorrelator.hpp"
#include "UtkScanInterface.hpp"
#include "UtkUnpacker.hpp"

using namespace std;

/// Contains event information, the information is filled in ScanList() and is
/// referenced in DetectorDriver.cpp, particularly in ProcessEvent().
RawEvent rawev;

UtkUnpacker::~UtkUnpacker() {
    //Call destructor for Detector Driver so that we close out Processors
    // properly
    DetectorDriver::get()->~DetectorDriver();
}

/** Process all events in the event list.
  * \param[in]  addr_ Pointer to a location in memory. 
  * \return Nothing. */
void UtkUnpacker::ProcessRawEvent(ScanInterface *addr_/*=NULL*/){
    if(!addr_)
        return;
    
    DetectorDriver* driver = DetectorDriver::get();
    DetectorLibrary* modChan = DetectorLibrary::get();
    XiaData *current_event = NULL;

    static float hz = sysconf(_SC_CLK_TCK); // get the number of clock ticks per second
    static clock_t clockBegin; // initialization time
    static struct tms tmsBegin;
    static unsigned int counter = 0;

    // local variable for the detectors used in a given event
    set<string> usedDetectors;
    Messenger m;
    stringstream ss;
    
    // Initialize the scan program before the first event
    if(counter == 0){
	// Retrieve the current time for use later to determine the total running time of the analysis.
	m.start("Initializing scan");
	clockBegin = times(&tmsBegin);
	ss << "First buffer at " << clockBegin << " sys time";
	m.detail(ss.str());
	ss.str("");
	
	// After completion the descriptions of all channels are in the modChan
	// vector, the DetectorDriver and rawevent have been initialized with the
	// detectors that will be used in this analysis.
	modChan->PrintUsedDetectors(rawev);
	driver->Init(rawev);
	
	/* Make a last check to see that everything is in order for the driver
	 * before processing data. SanityCheck function throws exception if
	 * something went wrong. */
	try{
	    driver->SanityCheck(); 
	} catch(GeneralException &e){
	    m.fail();
	    std::cout << "Exception caught while checking DetectorDriver"
                      << " sanity in UtkUnpacker::ProcessRawEvent" << std::endl;
	    std::cout << "\t" << e.what() << std::endl;
	    exit(EXIT_FAILURE);
	} catch(GeneralWarning &w){
	    std::cout << "Warning caught during checking DetectorDriver"
                      << " at UtkUnpacker::ProcessRawEvent" << std::endl;
	    std::cout << "\t" << w.what() << std::endl;
	}
	
	ss << "Init at " << times(&tmsBegin) << " sys time.";
	m.detail(ss.str());
	m.done();
    } //if(counter == 0)

    //BEGIN SCANLIST PART
    unsigned long chanTime, eventTime;
    
    /** Rejection regions should be defined here*/
    
    //The initial event
    deque<XiaData*>::iterator iEvent = rawEvent.begin();

    // local variables for the times of the current event, previous
    // event and time difference between the two
    double diffTime = 0;
    //set last_t to the time of the first event
    double lastTime = (*iEvent)->time;
    double currTime = lastTime;
    unsigned int id = (*iEvent)->getID();

    //Save time of the beginning of the file,
    //this is needed for the rejection regions
    static double firstTime = lastTime;
    bool isFirstEvt = true;
    bool isLastEvt = false;
    //HistoStats(id, diffTime, lastTime, BUFFER_START);

    // Now clear all places in correlator (if resetable type)
    for (map<string, Place*>::iterator it =
	     TreeCorrelator::get()->places_.begin();
	 it != TreeCorrelator::get()->places_.end(); ++it)
	if ((*it).second->resetable())
	    (*it).second->reset();

    //loop over the list of channels that fired in this buffer
    for(deque<XiaData*>::iterator it = rawEvent.begin();
     	it != rawEvent.end(); it++){

        ///Check if this is the last event in the deque.
        if(rawEvent.size() == 1)
            isLastEvt = true;

        current_event = *it;

        // Check that this channel event exists.
        if(!current_event)
            continue;

        ///Completely ignore any channel that is set to be ignored
        if (id == pixie::U_DELIMITER) {
            ss << "pattern 0 ignore";
            m.warning(ss.str());
            ss.str("");
            continue;
	    }
	
        if ((*modChan)[id].GetType() == "ignore")
            continue;

        // Do something with the current event.
        ChanEvent *event = new ChanEvent(current_event);

        //calculate some of the parameters of interest
        id = event->GetID();
        chanTime  = event->GetTime();
        eventTime = event->GetEventTimeLo();
        /* retrieve the current event time and determine the time difference
           between the current and previous events.
        */
        currTime = event->GetTime();
        diffTime = currTime - lastTime;

        //Add the ChanEvent pointer to the rawev and used detectors.
        usedDetectors.insert((*modChan)[id].GetType());
        rawev.AddChan(event);

        // 	if(isFirstEvt) {
        // 	    //Save time of the beginning of the file,
        // 	    //this is needed for the rejection regions
        // 	    firstTime = lastTime;
        // 	    //HistoStats(id, diffTime, lastTime, BUFFER_START);
        // 	    isFirstEvt = false;
        // 	}else if(isLastEvt) {
        // 	    string mode;
        // 	    //HistoStats(id, diffTime, currTime, BUFFER_END);


        // 	}else
        // 	    //HistoStats(id, diffTime, lastTime, EVENT_CONTINUE);

        // 	//REJECTION REGIONS WOULD GO HERE

        //     /* if the time difference between the current and previous event is
        //     larger than the event width, finalize the current event, otherwise
        //     treat this as part of the current event
        //     */
        //     if ( diffTime > Globals::get()->eventWidth() ) {
        //         if(rawev.Size() > 0) {
        //         /* detector driver accesses rawevent externally in order to
        //         have access to proper detector_summaries
        //         */
        // 		cout << rawev.GetEventList().size() << endl;
        //             driver->ProcessEvent(rawev);
        //         }

        //         //HistoStats(id, diffTime, currTime, EVENT_START);
        //     } //else HistoStats(id, diffTime, currTime, EVENT_CONTINUE);

        // 	//DTIME STUFF GOES HERE

        //     // update the time of the last event
        //     lastTime = currTime;
    }//for(deque<PixieData*>::iterator

    driver->ProcessEvent(rawev);
    rawev.Zero(usedDetectors);
    usedDetectors.clear();
    counter++;
}
