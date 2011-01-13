/** \file rawevent.cpp
 *  \brief defines functions associated with a rawevent  
 */

#include <iostream>

#include "RawEvent.h"

using namespace std;

const double ChanEvent::pixieEnergyContraction = 4.0;

/**
 * Identifier constructor
 *
 * The dammid and detector location variable are set to -1
 * and the detector type and sub type are both set to ""
 * when an identifier object is created.
 */
Identifier::Identifier(){
    Zero();
}

/**
 * Identifier zero
 *
 * The dammid and detector location variable are reset to -1
 * and the detector type and sub type are both reset to ""
 * when an identifier object is zeroed.
 */
void Identifier::Zero(){
    dammID   = -1;
    location = -1;
    type     = "";
    subtype  = "";
}

/**
 * Channel event constructor
 *
 * All numerical values are set to -1
 */
ChanEvent::ChanEvent() {
    ZeroNums();
}

/**
 * Channel event number zeroing
 *
 * This zeroes all the numerical values to -1, leaving internal objects
 * which should have constructors untouched
 */
void ChanEvent::ZeroNums() 
{
    energy      = -1;
    calEnergy   = -1;
    time        = -1;
    calTime     = -1;
    highResTime = -1;
    trigTime    = -1;
    eventTimeLo = -1;
    eventTimeHi = -1;
    runTime0    = -1;
    runTime1    = -1;
    runTime2    = -1;
    chanNum     = -1;
    modNum      = -1;

}

//* Find the identifier in the map for the channel event */
const Identifier& ChanEvent::GetChanID() const
{
    static Identifier nullIdentifier;
    
    extern vector<Identifier> modChan; // from PixieStd.cpp

    return ( (chanNum == -1) ? nullIdentifier : modChan.at(GetID()) );
}

//* Calculate a channel index */
int ChanEvent::GetID() const 
{
    return ((chanNum == -1) ? -1 : (16 * modNum + chanNum));
}

/**
 * Channel event zeroing

 * All numerical values are set to -1, and the trace,
 * and traceinfo vectors are cleared and the channel
 * identifier is zeroed using its identifier::zeroid method.
 */
void ChanEvent::ZeroVar() 
{
    ZeroNums();

    // clear objects
    trace.clear();
}

/**
 * Detector summary zeroing
 *
 * Clear the list of channel events associated with this summary
 */
void DetectorSummary::Zero() 
{
    eventList.clear();
    maxEvent = NULL;
}

/**
 * Detector summary constructor
 *
 * Clear the summary
 */
DetectorSummary::DetectorSummary()
{
    maxEvent = NULL;
}

DetectorSummary::DetectorSummary(const string &str, 
				 const vector<ChanEvent *> &fullList) : name(str)
{
    maxEvent = NULL;
    
    // go find all channel events with appropriate type and subtype
    size_t colonPos = str.find_first_of(":");
    type = str.substr(0, colonPos);
    if (colonPos == string::npos) {
	subtype = ""; // no associated subtype
    } else {
	subtype = str.substr(colonPos+1);
    }
    
    for (vector<ChanEvent *>::const_iterator it = fullList.begin();
	 it != fullList.end(); it++) {	
	const Identifier& id = (*it)->GetChanID();
	if ( id.GetType() != type )
	    continue;
	if ( subtype != "" && id.GetSubtype() != subtype )
	    continue;
	// put it in the summary
	AddEvent(*it);
    }
}

void DetectorSummary::AddEvent(ChanEvent *ev)
{
    eventList.push_back(ev);

    if (maxEvent == NULL || ev->GetCalEnergy() > maxEvent->GetCalEnergy()) {
	maxEvent = ev;
    }
}

/**
 * Override the < operator
 *
 * To use the detector summary in the STL map it is necessary to define the 
 * behavior of the "<" operator.  The "<" operator between a and b is defined
 * as whether the name for a is less than the name for b.
 */
bool operator<(const DetectorSummary &a, const DetectorSummary &b) {
    return a.GetName() < b.GetName();
}

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
void RawEvent::Init(const set<string> &usedTypes,
		    const set<string> &usedSubtypes)
{
    /* initialize the map of used detectors. This will associate the name of a
       detector type (such as dssd_front, ge ...) with a detector summary. 
       See ProcessEvent() in DetectorDriver.cpp for a description of the 
       variables in the summary
    */
    usedDetectors = usedTypes;

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

    if (it == sumMap.end()) {
	if (construct) {
	    // construct the summary
	    cout << "Constructing detector summary for type " << s << endl;
	    sumMap.insert( make_pair(s, DetectorSummary(s, eventList) ) );
	    it = sumMap.find(s);
	} else {
	    cout << "Returning NULL detector summary for type " << s << endl;
	    return NULL;
	}
    }
    return &(it->second);
}

const DetectorSummary *RawEvent::GetSummary(const string &s) const
{
    map<string, DetectorSummary>::const_iterator it = sumMap.find(s);
  
    if ( it == sumMap.end() ) {
	cout << "Returning NULL detector summary for type " << s << endl;
	return NULL;
    }
    return &(it->second);
}
