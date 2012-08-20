#include "NewCorrelator.h"
#include <iostream>

using namespace std;

void Place::addChild (Place* child, bool coin) {
    bool different = true;

    vector<Child>::iterator it;
    for (it = children_.begin(); it != children_.end(); ++it) {
        if (it->place == child) {
            different = false;
            break;
        }
    }

    if (different) {
        children_.push_back(Child(child, coin));
        child->addParent(this);
    } else {
        cout << "#Warning: attempting to add second copy of\
 child in function Place::addChild() " << endl;
    }

}

/** Example of place that do not depend on children status, probably good
 * choice for last in the tree (e.g. physical single detector) */
void PlaceBasic::check() {
}

/** Example of OR place, place 'Gamma' is active if ANY of children was hit.*/
void PlaceOR::check() {
    if (children_.size() > 0) {
        // Take first child to get initial state
        // Browse through other children
        // if sum is true break (OR will be true anyway)
        bool sum = children_[0].getCoin(); 
        for (unsigned i = 1; i < children_.size(); ++i) {
            sum = sum || children_[i].getCoin(); 
            if (sum)
                break;
        }
    
        // If status is going to be changed, report it to parents
        if (sum != status_) {
            status_ = sum;
            report_();
        }
    }
}

/** Example of AND place, place 'GammaBeta' is active
 * if ALL of children were hit.*/
void PlaceAND::check() {
    if (children_.size() > 0) {
        // Take first child to get initial state
        // Browse through other children
        // if sum is false break (AND will be false anyway)
        bool sum = children_[0].getCoin(); 
        for (unsigned i = 1; i < children_.size(); ++i) {
            sum = sum && children_[i].getCoin(); 
            if (!sum)
                break;
        }
        
        // If status is going to be changed, report it to parents
        if (sum != status_) {
            status_ = sum;
            report_();
        }
    }
}
