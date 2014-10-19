#include "DetectorSummary.hpp"

using namespace std;

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
    size_t colonPos1 = str.find_last_of(":");
    
    type = str.substr(0, colonPos);

    if (colonPos == string::npos) {
	subtype = ""; // no associated subtype
    } else {
	if(colonPos != colonPos1) {
	    subtype = str.substr(colonPos+1, colonPos1-colonPos-1);
	    tag = str.substr(colonPos1+1);
	} else {
	    subtype = str.substr(colonPos+1);
	    tag = "";
	}
    }    

    for (vector<ChanEvent *>::const_iterator it = fullList.begin();
	 it != fullList.end(); it++) {	
	const Identifier& id = (*it)->GetChanID();
	
	if ( id.GetType() != type )
	    continue;
	if ( subtype != "" && id.GetSubtype() != subtype )
	    continue;
	if (tag != "" && !id.HasTag(tag))
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

