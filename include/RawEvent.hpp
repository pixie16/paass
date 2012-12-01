/** \file RawEvent.hpp
 * \brief Header file for the rawevent
 *
 * The variables and structures contained in a raw event
 *
 * \author S. Liddick
 * \date 02 July 2007
 */

#ifndef __RAWEVENT_HPP_
#define __RAWEVENT_HPP_ 1

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>

#include "Correlator.hpp"
#include "pixie16app_defs.h"
#include "Globals.hpp"
#include "Trace.hpp"
#include "DetectorSummary.hpp"
#include "ChanEvent.hpp"
#include "ChanIdentifier.hpp"

// see DetectorSummary.hpp
class DetectorSummary;

/** \brief The all important raw event
 *
 * The rawevent serves as the basis for the experimental analysis.  The rawevent 
 * includes a vector of individual channels that have been deemed to be close to
 * each other in time.  This determination is performed in ScanList() from
 * PixieStd.cpp.  The rawevent also includes a map of detector summaries which
 * contains a detector summary for each detector type that is used in the analysis
 *
 *  The rawevent is intended to be versatile enough to remain unaltered unless
 * LARGE changes are made to the pixie16 code.  Be careful when altering the
 * rawevent.
 */
class RawEvent
{
private:
    std::map<std::string, DetectorSummary> sumMap; /**< An STL map containing DetectorSummary classes
					    associated with detector types */
    // This only controls whether we output warnings, so it's free to change
    mutable std::set<std::string> nullSummaries;   /**< Summaries which were requested but don't exist */
    std::vector<ChanEvent*> eventList;             /**< Pointers to all the channels that are close
					             enough in time to be considered a single event */
    Correlator correlator;                         /**< class to correlate decay data with implantation data */
public:   
    RawEvent();
    void Clear(void);
    size_t Size(void) const;
    void Init(const std::set<std::string> &usedTypes);
    void AddChan(ChanEvent* event);       
    void Zero(const std::set<std::string> &);

    Correlator &GetCorrelator()
    {return correlator;} /**< get the correlator */
    DetectorSummary *GetSummary(const std::string& a, bool construct = true);
    const DetectorSummary *GetSummary(const std::string &a) const;
    const std::vector<ChanEvent *> &GetEventList(void) const
    {return eventList;} /**< Get the list of events */
};

#endif // __RAWEVENT_HPP_
