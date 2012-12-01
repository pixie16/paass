/** \file RawEvent.cpp
 *  \brief defines functions associated with a rawevent  
 */
#include "RawEvent.hpp"

using namespace std;

/**
 * rawevent constructor
 */
RawEvent::RawEvent()
{
    // zeroing handling in member c'tors
}

/** Return the number of channels in the current event */
size_t RawEvent::Size() const
{
    return eventList.size();
}

/** Clear the list of individual channel events (Memory is managed elsewhere) */
void RawEvent::Clear()
{
    eventList.clear();
}

/**
 * \brief Raw event initialization
 *
 * Set the rawevent detector summary map with the passed argument.
 */
void RawEvent::Init(const set<string> &usedTypes)
{
    /* initialize the map of used detectors. This will associate the name of a
       detector type (such as dssd_front, ge ...) with a detector summary. 
       See ProcessEvent() for a description of the 
       variables in the summary
    */
    DetectorSummary ds;
    ds.Zero();

    for (set<string>::const_iterator it = usedTypes.begin();
	 it != usedTypes.end(); it++) {
        ds.SetName(*it);
        sumMap.insert(make_pair(*it,ds));
    }
}

/** Add a channel event to the raw event */
void RawEvent::AddChan(ChanEvent *event)
{
    eventList.push_back(event);
}

/**
 * Raw event zeroing
 *
 * For any detector type that was used in the event, zero the appropriate
 * detector summary in the map, and clear the event list
 */
void RawEvent::Zero(const set<string> &usedev)
{
    for (map<string, DetectorSummary>::iterator it = sumMap.begin();
	 it != sumMap.end(); it++) {
	(*it).second.Zero();
    }

    eventList.clear();
}

/**
 * Get a pointer to a specific detector summary
 *
 * Retrieve from the detector summary map a pointer to the specific detector
 * summary that is associated with the passed string. 
 */
DetectorSummary *RawEvent::GetSummary(const string& s, bool construct)
{
    map<string, DetectorSummary>::iterator it = sumMap.find(s);
    static set<string> nullSummaries;

    if (it == sumMap.end()) {
        if (construct) {
            // construct the summary
            cout << "Constructing detector summary for type " << s << endl;
            sumMap.insert( make_pair(s, DetectorSummary(s, eventList) ) );
            it = sumMap.find(s);
        } else {
            if (nullSummaries.count(s) == 0) {
                cout << "Returning NULL detector summary for type " << s << endl;
                nullSummaries.insert(s);
            }
            return NULL;
        }
    }
    return &(it->second);
}

const DetectorSummary *RawEvent::GetSummary(const string &s) const
{
    map<string, DetectorSummary>::const_iterator it = sumMap.find(s);
    
    if ( it == sumMap.end() ) {
        if (nullSummaries.count(s) == 0) {
            cout << "Returning NULL const detector summary for type " << s << endl;
            nullSummaries.insert(s);
        }
        return NULL;
    }
    return &(it->second);
}
