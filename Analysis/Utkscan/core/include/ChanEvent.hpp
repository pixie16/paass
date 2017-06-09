/** \file ChanEvent.hpp
 * \brief A Class to define what a channel event is
 * @authors D. Miller, K. A. Miernik, S. V. Paulauskas
 */
#ifndef __CHANEVENT_HPP__
#define __CHANEVENT_HPP__

#include <vector>

#include "ChannelConfiguration.hpp"
#include "ProcessedXiaData.hpp"
#include "Trace.hpp"
#include "DetectorLibrary.hpp"


/*! \brief A channel event
 *
 * All data is grouped together into channels.  For each pixie16 channel that
 * fires the energy, time (both trigger time and event time), and trace (if
 * applicable) are obtained.  Additional information includes the channels
 * channelConfiguration, calibrated energies, trace analysis information.
 * Note that this currently stores raw values internally through pixie word types
 *   but returns data values through native C types. This is potentially non-portable.
 */
class ChanEvent : public ProcessedXiaData {
public:
    /** Default constructor that zeroes all values */
    ChanEvent() {}

    ///Constructor taking the base class as an argument so that we can set
    /// the trace information properly
    ///@param[in] evt : The event that we are going to assign here.
    ChanEvent(XiaData &evt) : ProcessedXiaData(evt) {}

    ///Default Destructor
    ~ChanEvent() {}

    //! \return The channelConfiguration in the map for the channel event
    const ChannelConfiguration &GetChanID() const {
        return DetectorLibrary::get()->at(GetModuleNumber(), GetChannelNumber());
    }

    /** \return the channel id defined as pixie module # * 16 + channel number */
    unsigned int GetID() const { return DetectorLibrary::get()->GetIndex(GetModuleNumber(), GetChannelNumber()); }

    ///Equality operator, we only check to see if the module number, channel number, and times are equal.
    ///@param [in] rhs : the configuration to compare to
    ///@return true if the module number, channel number, and time are identical
    bool operator==(const ChanEvent &rhs) const {
        return GetModuleNumber() == rhs.GetModuleNumber() && GetChannelNumber() == rhs.GetChannelNumber() &&
                GetTime() == rhs.GetTime();
    }

    ///Not - Equality operator for ChanEvent
    ///@param [in] x : the ChanEvent to compare to
    ///@return true if the type, subtype, or location do not match
    bool operator!=(const ChanEvent &rhs) const { return !operator==(rhs); }

    ///Less-then operator needed for STL containers
    ///@param [in] rhs : the ChannelConfiguration to compare
    ///@return true if the type, subtype, or location are less than those in rhs
    bool operator<(const ChanEvent &rhs) const {
        if(GetWalkCorrectedTime() == 0)
            return GetTime() < rhs.GetTime();
        return GetWalkCorrectedTime() < rhs.GetWalkCorrectedTime();
    }

    ///Greater-than operator needed for STL containers
    ///@param[in] rhs : The right hand side that we are comparing with.
    ///@return The negative of the less than operator.
    bool operator>(const ChanEvent &rhs) const { return !operator<(rhs); }
};
#endif
