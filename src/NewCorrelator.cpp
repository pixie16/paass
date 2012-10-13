#include <iostream>
#include <sstream>
#include <map>
#include "NewCorrelator.hpp"

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

void Detector::check_() {
}

/** Example of place that do not depend on children status, probably good
 * choice for last in the tree (e.g. physical single detector) */
void PlaceBasic::check_() {
}

/** Example of OR place, place 'Gamma' is active if ANY of children was hit.*/
void PlaceOR::check_() {
    if (children_.size() > 0) {
        // Take first child to get initial state
        // Browse through other children
        // if sum is true break (OR will be true anyway)
        // In order to check childer status we must take into account 
        // relation (true for coincidence, false for anticoincidence)
        // Thus we check (children_[i].first->status() == children_[i].second)
        // where first is pointer to child Place, second is bool (relation)
        bool result = (children_[0].first->status() == children_[0].second);
        for (unsigned i = 1; i < children_.size(); ++i) {
            result = result || (children_[i].first->status() == children_[0].second);
            if (result)
                break;
        }
        // If status is changed, report it to parents
        if (result != status_) {
            status_ = result;
            report_();
        }
    } else {
        stringstream ss;
        ss << "Place " << this << " has no children, however function check() was called.";
        throw GeneralException(ss.str());
    }
}

/** Example of AND place, place is active
 * if ALL of children were hit.*/
void PlaceAND::check_() {
    if (children_.size() > 0) {
        // Take first child to get initial state
        // Browse through other children
        // if result is false break (AND will be false anyway)
        bool result = (children_[0].first->status() == children_[0].second);
        for (unsigned i = 1; i < children_.size(); ++i) {
            result = result && (children_[i].first->status() == children_[0].second);
            if (!result)
                break;
        }
        
        // If status is going to be changed, report it to parents
        if (result != status_) {
            status_ = result;
            report_();
        }
    }
}
