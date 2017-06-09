///@file DetectorSummary.hpp
///@brief A class that generates summaries of detector types/subtypes/tags that are requested by the user.
///@author D. Miller, K. Miernik, S. V. Paulauskas
///@date May 28, 2017
#include <stdexcept>

#include "DetectorSummary.hpp"
#include "StringManipulationFunctions.hpp"

using namespace std;

void DetectorSummary::Zero() {
    eventList_.clear();
    maxEvent_ = NULL;
}

DetectorSummary::DetectorSummary() {
    maxEvent_ = NULL;
}

DetectorSummary::DetectorSummary(const std::string &str, const std::vector<ChanEvent *> &fullList) : name_(str) {
    if (str == "")
        throw invalid_argument("DetectorSummary::DetectorSummary : Received a request using an empty summary name.");

    maxEvent_ = NULL;

    vector<string> tokens = StringManipulation::TokenizeString(str, ":");

    switch (tokens.size()) {
        case 3:
            tag_ = tokens.at(2);
        case 2:
            subtype_ = tokens.at(1);
        case 1:
            type_ = tokens.at(0);
            break;
        default:
            throw invalid_argument("DetectorSummary::DetectorSummary - Too many tokens in the string: " + str);
    }

    for (vector<ChanEvent *>::const_iterator it = fullList.begin(); it != fullList.end(); it++) {
        const ChannelConfiguration &id = (*it)->GetChanID();
        if ( id.GetType() != type_ )
            continue;
        if ( subtype_ != "" && id.GetSubtype() != subtype_ )
            continue;
        if (tag_ != "" && !id.HasTag(tag_))
            continue;
        AddEvent(*it);
    }
}

void DetectorSummary::AddEvent(ChanEvent *ev) {
    eventList_.push_back(ev);

    if (maxEvent_ == NULL || ev->GetCalibratedEnergy() > maxEvent_->GetCalibratedEnergy())
        maxEvent_ = ev;
}

