///@file UtkUnpacker.cpp
///@brief A child of the Unpacker class that is used to replace some of the
/// functionality of the PixieStd.cpp from pixie_scan
///@author S. V. Paulauskas
///@date June 17, 2016
#include <iostream>
#include <set>

#include <unistd.h>
#include <sys/times.h>

#include "DammPlotIds.hpp"
#include "Places.hpp"
#include "TreeCorrelator.hpp"
#include "UtkScanInterface.hpp"
#include "UtkUnpacker.hpp"

using namespace std;
using namespace dammIds::raw;

///The only thing that we do here is call the destructor of the
/// DetectorDriver. This will ensure that the memory is freed for all of the
/// initialized detector and experiment processors and that information about
/// the amount of time spent in each processor is output to the screen at the
/// end of execution.
UtkUnpacker::~UtkUnpacker() {
    delete DetectorDriver::get();
}

/// This method initializes the DetectorLibrary and DetectorDriver classes so
/// that we can begin processing the events. We take special action on the
/// first event so that we can handle somethings poperly. Then we processes
/// all channels in the event that we have not been told to ignore. The
/// rejection regions that are defined in the XML file are used here to
/// ignore chunks of data. We also make some calls to various other private
/// methods to plot useful spectra and output processing information to the
/// screen.
void UtkUnpacker::ProcessRawEvent() {
    static RawEvent rawev;
    DetectorDriver *driver = DetectorDriver::get();
    DetectorLibrary *detectorLibrary = DetectorLibrary::get();
    set<string> usedDetectors;
    Messenger m;
    stringstream ss;

    static clock_t systemStartTime;
    static struct tms systemTimes;
    static double lastTimeOfPreviousEvent;
    static unsigned int eventCounter = 0;
    static bool nEvent = false;

    if (eventCounter == 0)
        InitializeDriver(driver, detectorLibrary, rawev, systemStartTime);
    else if (eventCounter % 5000 == 0 || eventCounter == 1)
        PrintProcessingTimeInformation(systemStartTime, times(&systemTimes), GetEventStartTime(), eventCounter);

    if (Globals::get()->HasRejectionRegion()) {
        double eventTime = (GetEventStartTime() - GetFirstTime()) * Globals::get()->GetClockInSeconds();
        vector <pair<unsigned int, unsigned int>> rejectRegions = Globals::get()->GetRejectionRegions();

        for (vector<pair<unsigned int, unsigned int> >::iterator region = rejectRegions.begin();
             region != rejectRegions.end(); ++region)
            if (eventTime > region->first && eventTime < region->second)
                return;
    }

    driver->plot(D_EVENT_GAP, (GetRealStopTime() - lastTimeOfPreviousEvent) * Globals::get()->GetClockInSeconds() * 1e9);
    driver->plot(D_BUFFER_END_TIME, GetRealStopTime() * Globals::get()->GetClockInSeconds() * 1e9);
    driver->plot(D_EVENT_LENGTH, (GetRealStopTime() - GetRealStartTime()) * Globals::get()->GetClockInSeconds() * 1e9);
    driver->plot(D_EVENT_MULTIPLICITY, rawEvent.size());

    //loop over the list of channels that fired in this event
    for (deque<XiaData *>::iterator it = rawEvent.begin(); it != rawEvent.end(); it++) {

        if (!(*it))
            continue;

        RawStats((*it), driver);

        if ((*it)->GetId() == std::numeric_limits<unsigned int>::max()) {
            ss << "pattern 0 ignore";
            m.warning(ss.str());
            ss.str("");
            continue;
        }

        ///@TODO this will fail if the user does not define enough modules in the map. Related to pixie16/paass:#103
        if (detectorLibrary->at((*it)->GetId()).GetType() == "ignore")
            continue;

        ///@TODO we need to ensure that all of the memory is getting freed
        /// appropriately at the end of processing an event. I'm not sure
        /// that it is right now.
        ChanEvent *event = new ChanEvent(*(*it));

        ///@TODO This will also fail if the user doesn't define enough modules in the map. Related to pixie16/paass:#103
        usedDetectors.insert((*detectorLibrary)[(*it)->GetId()].GetType());
        rawev.AddChan(event);

        if(event->GetChanID().GetType() == "next" || event->GetChanID().GetType() == "RD") nEvent=true;
       // if(event->GetChanID().GetType() == "vandle") nEvent=true;
        //   if(event->GetChanID().GetType() == "next") nEvent=true;
        ///@TODO Add back in the processing for the dtime.
    }//for(deque<PixieData*>::iterator

    
    try {
        
        if (nEvent) driver->ProcessEvent(rawev);
        
        rawev.Zero(usedDetectors);
        usedDetectors.clear();

        ///@TODO I think that this is done twice, it needs to be investigated.
        for (map<string, Place *>::iterator it = TreeCorrelator::get()->places_.begin();
             it != TreeCorrelator::get()->places_.end(); ++it)
            if ((*it).second->resetable())
                (*it).second->reset();
    } catch (exception &ex) {
        throw;
    }
    
    eventCounter++;
    lastTimeOfPreviousEvent = GetRealStopTime();

    nEvent = false;
}

/// This method plots information about the running time of the program, the
/// hit spectrum, and the scalars for each of the channels. The two runtime
/// spectra are critical when we are trying to debug potential data losses in
/// the system. These spectra print the total number of counts in a given
/// (milli)second of time.
void UtkUnpacker::RawStats(XiaData *event_, DetectorDriver *driver) {
    int id = event_->GetId();
    static const int specNoBins = SE;
    static double runTimeSecs = 0, remainNumSecs = 0;
    static double runTimeMsecs = 0, remainNumMsecs = 0;
    static int rowNumSecs = 0, rowNumMsecs = 0;

    runTimeSecs = (event_->GetTime() - GetFirstTime()) * Globals::get()->GetClockInSeconds();
    rowNumSecs = int(runTimeSecs / specNoBins);
    remainNumSecs = runTimeSecs - rowNumSecs * specNoBins;

    runTimeMsecs = runTimeSecs * 1000;
    rowNumMsecs = int(runTimeMsecs / specNoBins);
    remainNumMsecs = runTimeMsecs - rowNumMsecs * specNoBins;

    driver->plot(D_HIT_SPECTRUM, id);
    driver->plot(DD_RUNTIME_SEC, remainNumSecs, rowNumSecs);
    driver->plot(DD_RUNTIME_MSEC, remainNumMsecs, rowNumMsecs);
    driver->plot(D_SCALAR + id, runTimeSecs);
}

/// First we initialize the DetectorLibrary, which reads the Map
/// node in the XML configuration file. Then we initialize DetectorDriver and
/// check that everything went all right with DetectorDriver::SanityCheck().
/// If the sanity check fails then we will terminate the program, if things went
/// well then we will warn the user about it and continue. If there is an
/// error parsing the DetectorDriver node in the XML file it will show up as
/// a General Exception here. This can be extremely useless sometimes...
/// @TODO Expand the types of exceptions handled so that we can make the
/// diagnostic information more useful for the user.
void UtkUnpacker::InitializeDriver(DetectorDriver *driver, DetectorLibrary *detlib, RawEvent &rawev, clock_t &start) {
    struct tms systemTimes;
    Messenger m;
    stringstream ss;

    m.start("Initializing scan");
    start = times(&systemTimes);
    ss << "First buffer at " << start << " system time";
    m.detail(ss.str());
    ss.str("");

    if(GetMaxModuleInFile() != detlib->GetModules() - 1)
        throw invalid_argument("UtkUnpacker::InitializeDriver - You did not define the last module (" +
                                       to_string(GetMaxModuleInFile()) + ") in the configuration file. This is fatal.");

    //detlib->PrintUsedDetectors(rawev);
    driver->Init(rawev);

    try {
        driver->SanityCheck();
    } catch (GeneralException &e) {
        m.fail();
        std::cout << "Exception checking DetectorDriver sanity in "
                "UtkUnpacker::InitializeDriver" << std::endl;
        std::cout << "\t" << e.what() << std::endl;
        exit(EXIT_FAILURE);
    } catch (GeneralWarning &w) {
        std::cout << "Warning checking DetectorDriver sanity in "
                "UtkUnpacker::InitializeDriver" << std::endl;
        std::cout << "\t" << w.what() << std::endl;
    }

    ss << "Init at " << times(&systemTimes) << " system time.";
    m.detail(ss.str());
    m.done();
}

/// Spits out some useful information about the analysis time, what timestamp
/// that we are currently on and information about how long it took us to get
/// to this point. One should note that this does not contain all of the
/// information that was present in PixieStd.cpp::hissub_. Some of that
/// information is not available or just not that relevant to us.
void UtkUnpacker::PrintProcessingTimeInformation(const clock_t &start, const clock_t &now, const double &eventTime,
                                                 const unsigned int &eventCounter) {
    Messenger m;
    stringstream ss;
    static float hz = sysconf(_SC_CLK_TCK);

    ss << "Data read up to built event number " << eventCounter << " in "
       << (now - start) / hz << " seconds. Current timestamp is "
       << eventTime;
    m.run_message(ss.str());
}

