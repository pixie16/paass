///@file UtkUnpacker.cpp
///@brief A child of the Unpacker class that is used to replace some of the
/// functionality of the PixieStd.cpp from pixie_scan
///@author S. V. Paulauskas
///@date June 17, 2016
#ifndef __UTKUNPACKER_HPP__
#define __UTKUNPACKER_HPP__

#include <ctime>
#include <set>
#include <string>

#include "DetectorDriver.hpp"
#include "DetectorLibrary.hpp"
#include "RawEvent.hpp"
#include "Unpacker.hpp"

///A class that is derived from Unpacker that defines what we are going to do
/// with all of the events that are built by the Unpacker class. We only
/// define a single class (ProcessRawEvent) and overload the RawStats class
/// to take a pointer to a DetectorDriver instance. The rest of the virtual
/// methods in the parent are used as default.
class UtkUnpacker : public Unpacker {
public:
    /// Default constructor that does nothing in particular
    UtkUnpacker() : Unpacker() {}

    /// Default destructor that deconstructs the DetectorDriver singleton
    ~UtkUnpacker();

private:
    ///@brief Process all events in the event list.
    ///@param[in]  addr_ Pointer to a ScanInterface object.
    void ProcessRawEvent();

    ///@brief Initializes the DetectorLibrary and DetectorDriver
    ///@param[in] driver A pointer to the DetectorDriver that we're using.
    ///@param[in] detlib A pointer to the DetectorLibrary that we're using.
    ///@param[in] start The time that we called this method.
    void InitializeDriver(DetectorDriver *driver, DetectorLibrary *detlib, RawEvent &rawev, clock_t &start);

    ///@brief Prints information about time spent processing to the screen
    ///@param[in] start The time that we started processing data (buffer 0)
    ///@param[in] now What time it is now
    ///@param[in] eventTime The time of the current event
    ///@param[in] eventCounter What event we are currently processing.
    void PrintProcessingTimeInformation(const clock_t &start, const clock_t &now, const double &eventTime,
                                        const unsigned int &eventCounter);

    ///@brief Add an event to generic statistics output.
    ///@param[in] event_ Pointer to the current XIA event.
    ///@param[in] driver Pointer to the DetectorDriver class that we're using.
    ///@param[in] addr_  Pointer to a ScanInterface object.
    virtual void RawStats(XiaData *event_, DetectorDriver *driver);
    std::set<std::string> usedDetectors;
};

#endif //__UTKUNPACKER_HPP__
