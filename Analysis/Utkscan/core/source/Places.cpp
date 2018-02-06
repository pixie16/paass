/** \file Places.cpp
* \brief Defines the various places for the TreeCorrelator
* \author K. A. Miernik
* \date October 22, 2012
*/
#include <iostream>
#include <sstream>
#include <map>

#include "TreeCorrelator.hpp"
#include "Exceptions.hpp"

using namespace std;

bool Place::checkParents(Place* child) {
    bool isAllDifferent = true;
    vector<Place*>::iterator it;
    for (it = parents_.begin(); it != parents_.end(); ++it) {
        isAllDifferent = ((*it) != child) && (*it)->checkParents(child);
        if (!isAllDifferent)
            break;
    }
    return isAllDifferent;
}

bool Place::checkChildren(Place* child) {
    bool isAllDifferent = true;
    vector< pair<Place*, bool> >::iterator it;
    for (it = children_.begin(); it != children_.end(); ++it) {
        isAllDifferent = it->first != child;
        if (!isAllDifferent)
            break;
    }
    return isAllDifferent;
}

void Place::addChild (Place* child, bool relation) {
    if (checkChildren(child) && checkParents(child)) {
        children_.push_back(pair<Place*, bool>(child, relation));
        child->addParent_(this);
    } else {
        stringstream ss;
        ss << "Place " << this << " attempted unsuccesfully to add child "
           << child;
        throw GeneralException(ss.str());
    }
}

void PlaceOR::check_(EventData& info) {
    if (children_.size() > 0) {
        bool result = (children_[0].first->status() == children_[0].second);
        for (unsigned i = 1; i < children_.size(); ++i) {
            result = result || (children_[i].first->status() == children_[0].second);
            if (result)
                break;
        }

        if (result)
            this->activate(info);
        else
            this->deactivate(info.time);
        report_(info);
    } else {
        stringstream ss;
        ss << "Place " << this << " has no children, however function check() was called.";
        throw GeneralException(ss.str());
    }
}

void PlaceThresholdOR::check_(EventData& info) {
    if (children_.size() > 0) {
        bool result = (children_[0].first->status() == children_[0].second);
        for (unsigned i = 1; i < children_.size(); ++i) {
            result = result || (children_[i].first->status() == children_[0].second);
            if (result)
                break;
        }

        if (result)
            this->activate(info);
        else
            this->deactivate(info.time);
        report_(info);
    } else {
        stringstream ss;
        ss << "Place " << this << " has no children, however function check() was called.";
        throw GeneralException(ss.str());
    }
}

void PlaceCounter::check_(EventData& info) {
    if (info.status) {
        this->activate(info);
    }
}

void PlaceAND::check_(EventData& info) {
    if (children_.size() > 0) {
        bool result = (children_[0].first->status() == children_[0].second);
        for (unsigned i = 1; i < children_.size(); ++i) {
            result = result && (children_[i].first->status() == children_[0].second);
            if (!result)
                break;
        }
	
        if (result)
            this->activate(info);
        else
            this->deactivate(info.time);
        report_(info);
    }
}
