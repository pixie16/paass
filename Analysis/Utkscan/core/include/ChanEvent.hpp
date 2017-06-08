/** \file ChanEvent.hpp
 * \brief A Class to define what a channel event is
 */
#ifndef __CHANEVENT_HPP
#define __CHANEVENT_HPP

#include <vector>

#include "ProcessedXiaData.hpp"
#include "DetectorLibrary.hpp"
#include "Identifier.hpp"
#include "Globals.hpp"
#include "Trace.hpp"
#include "XiaData.hpp"

/*! \brief A channel event
 *
 * All data is grouped together into channels.  For each pixie16 channel that
 * fires the energy, time (both trigger time and event time), and trace (if
 * applicable) are obtained.  Additional information includes the channels
 * identifier, calibrated energies, trace analysis information.
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
    ChanEvent(XiaData &evt) : ProcessedXiaData(evt) {};

    ///Default Destructor
    ~ChanEvent() {}

    //! \return The identifier in the map for the channel event
    const Identifier &GetChanID() const;

/** \return the channel id defined as pixie module # * 16 + channel number */
    int GetID() const;
};

/** Sort by increasing corrected time
 * \param [in] a : the left hand side for comparison
 * \param [in] b : the right hand side for comparison
 * \return True if LHS is less the RHS */
bool CompareCorrectedTime(const ChanEvent *a, const ChanEvent *b);

/** Sort by increasing raw time
 * \param [in] a : the left hand side for comparison
 * \param [in] b : the right hand side for comparison
 * \return True if LHS is less the RHS*/
bool CompareTime(const ChanEvent *a, const ChanEvent *b);

#endif
