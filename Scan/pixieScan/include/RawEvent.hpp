/** \file RawEvent.hpp
 * \brief Header file for the rawevent
 *
 * The variables and structures contained in a raw event
 * \author S. Liddick
 * \date 02 July 2007
 * Modified:
 *    KM = 27 - 11 - 2012
 *    some classes moved to their own .hpp and .cpp files
 */
#ifndef __RAWEVENT_HPP_
#define __RAWEVENT_HPP_

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "pixie16app_defs.h"

#include "Globals.hpp"
#include "Trace.hpp"
#include "DetectorSummary.hpp"
#include "ChanEvent.hpp"
#include "Identifier.hpp"

// see DetectorSummary.hpp
class DetectorSummary;
//see ChanEvent.hpp
class ChanEvent;

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
class RawEvent {
public:
    /** Default Constructor */
    RawEvent(){};

    /** Default Destructor */
    ~RawEvent(){};

    /** Clear the list of individual channel events (Memory is managed elsewhere) */
    void Clear(void) {eventList.clear();};

    /** \return the number of channels in the current event */
    size_t Size(void) const {return(eventList.size());};

    /** \brief Raw event initialization and set the rawevent detector summary
    * map with the passed argument.
    * \param [in] usedTypes : the list of types used in the analysis
    */
    void Init(const std::set<std::string> &usedTypes);

    /** Add a channel event to the raw event
    * \param [in] event : the event to add to the raw event */
    void AddChan(ChanEvent* event) {eventList.push_back(event);};

    /** \brief Raw event zeroing
    *
    * For any detector type that was used in the event, zero the appropriate
    * detector summary in the map, and clear the event list
    * \param [in] usedev : the detector summary to zero */
    void Zero(const std::set<std::string> &usedev);

    /** \brief Get a pointer to a specific detector summary
    *
    * Retrieve from the detector summary map a pointer to the specific detector
    * summary that is associated with the passed string.
    * \param [in] a : the summary that you would like
    * \param [in] construct : flag indicating if we need to construct the summary
    * \return a pointer to the summary */
    DetectorSummary *GetSummary(const std::string& a, bool construct = true);

    /** \return a pointer to the requested summary
    * \param [in] a : the name of the summary that you would like */
    const DetectorSummary *GetSummary(const std::string &a) const;

    /** \return the list of events */
    const std::vector<ChanEvent *> &GetEventList(void) const {return eventList;}
private:
    std::map<std::string, DetectorSummary> sumMap; /**< An STL map containing DetectorSummary classes
					    associated with detector types */
    mutable std::set<std::string> nullSummaries;   /**< Summaries which were requested but don't exist */
    std::vector<ChanEvent*> eventList; /**< Pointers to all the channels that are close
                                            enough in time to be considered a single event */
};
#endif // __RAWEVENT_HPP_
