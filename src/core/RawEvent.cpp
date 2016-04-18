/** \file RawEvent.cpp
 *  \brief defines functions associated with a rawevent
 */
#include <sstream>

#include "RawEvent.hpp"
#include "Messenger.hpp"

using namespace std;

void RawEvent::Init(const std::set<std::string> &usedTypes)
{
    /*! initialize the map of used detectors. This will associate the name of a
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

void RawEvent::Zero(const std::set<std::string> &usedev) {
    for (map<string, DetectorSummary>::iterator it = sumMap.begin();
	 it != sumMap.end(); it++) {
        (*it).second.Zero();
    }

    eventList.clear();
}

DetectorSummary *RawEvent::GetSummary(const std::string& s, bool construct) {
    map<string, DetectorSummary>::iterator it = sumMap.find(s);
    static set<string> nullSummaries;

    Messenger m;
    stringstream ss;
    if (it == sumMap.end()) {
        if (construct) {
            // construct the summary
            ss << "Constructing detector summary for type " << s;
            m.detail(ss.str());
            sumMap.insert( make_pair(s, DetectorSummary(s, eventList) ) );
            it = sumMap.find(s);
        } else {
            if (nullSummaries.count(s) == 0) {
                ss << "Returning NULL detector summary for type " << s;
                m.detail(ss.str());
                nullSummaries.insert(s);
            }
            return NULL;
        }
    }
    return &(it->second);
}

const DetectorSummary *RawEvent::GetSummary(const std::string &s) const {
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
