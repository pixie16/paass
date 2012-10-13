#ifndef NEWCORRELATOR_H
#define NEWCORRELATOR_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <stdexcept>

using namespace std;

class GeneralException : public std::exception {
public:
    explicit GeneralException(const string& msg) 
        : exception(), message_(msg) {}

    virtual ~GeneralException() throw() {}

    virtual const char* what() const throw()
    {
        return message_.c_str();
    }

private:
    const string message_;
};


/** A pure abstract class to define a "place" for correlator.
 * A place has physical or abstract meaning, might be a detector, 
 * a condition or a group of detectors.
 */
class Place {
    public:
        /** C'tor. By default the Place is resetable. */
        Place(bool resetable = true) {
            resetable_ = resetable;
            status_ = false;
        }

        virtual ~Place() {
        }

        /** Defines 'child' of place. A child will report any
         * changes of its status to parent. Parent will change its
         * status depending on status of children but parent status will
         * not affect status of children.
         */
        virtual void addChild (Place* child, bool coin = true);

        /** Checks is child is already listed among children.
         * In case it is, False value is returned.*/
        virtual bool checkChildren (Place* child);

        /** Checks is child is its own parent or his parent parent (
         * uptree) etc. In such a case False is returned.*/
        virtual bool checkParents (Place* child);

        /** Returns requested info (double) on Place stored in 
         * map<string, double> by key passed as name.
         * If name is not present in map, -1 is returned.*/
        virtual double getInfo(string name) {
            if (info_.count(name) > 0)
                return info_[name];
            else
                return -1;
        }

        /** Unary operator, changes status to true.*/
        virtual void operator+() {
            if (!status_) {
                status_ = true;
                report_();
            }
        }

        /** Binary operator, changes status to true, and saves info.*/
        virtual void operator+(map<string, double>& info) {
            if (!status_) {
                status_ = true;
                info_ = info;
                report_();
            }
        }

        /** Function version of operator +*/
        virtual void activate() {
            +(*this);
        }

        /** Function version of binary operator +*/
        virtual void activate(map<string, double>& info) {
            (*this) + info;
        }

        /** Unary operator, changes status to false.*/
        virtual void operator-() {
            if (status_) {
                status_ = false;
                report_();
            }
        }

        /** Function version of operator -*/
        virtual void deactivate() {
            -(*this);
        }

        /** Toggles status*/
        virtual void toggle() {
            status_ = !status_;
            report_();
        }

        /** Toggle status with info send. Info is stored only if
         * status is changing from False to True (place is activated).*/
        virtual void toggle(map<string, double> info) {
            if (!status_) {
                info_ = info;
            }
            status_ = !status_;
            report_();
        }

        /** Logical AND operator for two Places. */
        virtual bool operator&& (const Place& right) const {
            return (*this)() && right();
        }

        /** Logical OR operator for two Places. */
        virtual bool operator|| (const Place& right) const {
            return (*this)() || right();
        }

        /** Returns status of the place.*/
        virtual bool operator() () const {
            return status_;
        }

        /** Returns status of the place. Function version of operator ()*/
        virtual bool status() const {
            return (*this)();
        }

        /** Returns true if place should automatically deactivate
         * after the end of event and false if place should stay 
         * in current state as long as deactivating
         * signal is explicitly send. */
        virtual bool resetable() const {
            return resetable_;
        }

    protected:
        /** Pure virutal function. The check function should decide how
         * to change status depending on the status of children. Should be 
         * implemented in a derived class.
         */
        virtual void check_() = 0;

        /** Adds parent to the list. User should be able to call addChild 
         * function only. The addParent function should be called by addChild.
         */
        virtual void addParent_ (Place* parent) {
            parents_.push_back(parent);
        }

        /** Reports change of status to parents. 
         * Calls check() function for all the parents.
         */
        virtual void report_() {
            vector<Place*>::iterator it;
            for (it = parents_.begin(); it != parents_.end(); ++it)
                (*it)->check_();
        }

        /** Status is true if given place is in active state (e.g. detector
         * recorded an event).*/
        bool status_;

        /** Indicates if place should be reseted after end of event (true)
         * or if should persist until status is changed explicitly (false).*/
        bool resetable_;

        /** Vector keeping a list of parents to whom the change of status
         * should be reported.
         */
        vector<Place*> parents_;

        /** Vector keeping a list of children on which status of the
         * Place depends.
         * Place* is a pointer to the downstream place, bool describes relation
         * (true for coincidence-like, false for anti-coincidence).
         */
        vector< pair<Place*, bool> > children_;

        /** Map string-> double keeping all information that might be 
         * useful for correlator e.g. energy, time, position etc.
         * This allow for flexible passing of arguments. 
         * Use getInfo(string) function to get access to data stored here.*/
        map<string, double> info_;

};

/** Most basic place which does not have any dependency on children.*/
class PlaceBasic : public Place {
    public:
        PlaceBasic(bool resetable = true) : Place(resetable) {}
    protected:
        virtual void check_();
};

/** The basic detector which does not depend on children status. Will use
 *  info_ map to store additional information.*/
class Detector : public Place {
    public:
        Detector(bool resetable = true) : Place(resetable) {} 
    protected:
        virtual void check_();
};

/** This Place is probably an abstract place, which status depends on
 * children in OR manner i.e if any of children is activated, the place 
 * is activated too. An example is a Beta place (if any of beta detectors was
 * activated, the place Beta is also active indicating that there was beta
 * registered in the system).*/
class PlaceOR : public Place {
    public:
        PlaceOR() : Place() {}
    protected:
        virtual void check_();
};

/** Similar to PlaceOR but uses AND relation. That makes it a suitable choice
 * for coincidences and anti-coincidences (use relation parameter when adding
 * child). An example is a GammaBeta Place (if both Gamma and Beta places
 * are active that means we had beta-gamma coincidence and the
 * GammaBeta should be active).*/
class PlaceAND : public Place {
    public:
        PlaceAND() : Place() {}
    protected:
        virtual void check_();
};
#endif
