#ifndef TREECORRELATOR_H
#define TREECORRELATOR_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <deque>
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

/** Simple structure holding basic parameters needed for correlation
 * of events in the same place. */
class CorrEventData {
    public:
        /** Time is always needed, by default status is true and
         * Energy is 0 (i.e. N/A).*/
        CorrEventData(double t, bool s = true, double E = 0) {
            time = t;
            status = s;
            energy = E;
        }
        bool status;
        double time;
        double energy;
};

/** A pure abstract class to define a "place" for correlator.
 * A place has physical or abstract meaning, might be a detector, 
 * a condition or a group of detectors.
 */
class Place {
    public:
        /** C'tor. By default the Place is resetable, and internal
         * fifo remebers only current and previous event.*/
        Place(bool resetable = true, unsigned max_size = 2) {
            resetable_ = resetable;
            max_size_ = max_size;
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

        /** Activates Place if place was not active, saves event data to
         * deque.*/
        virtual void activate(CorrEventData& info) {
            if (!status_) {
                status_ = true;
                add_info_(info);
                report_(info);
            }
        }

        /** Simplified activation for counter-like detectors, without
         * need of creating CorrEventData object.*/
        virtual void activate(double time) {
            if (!status_) {
                status_ = true;
                CorrEventData info(time, status_);
                add_info_(info);
                report_(info);
            }
        }

        /** Changes status to false. Only time and status change is
         * recorded in fifo. */
        virtual void deactivate(double time) {
            if (status_) {
                status_ = false;
                CorrEventData info(time, status_);
                add_info_(info);
                report_(info);
            }
        }

        /** Changes status to false without storing correlation
         * data or reporting to parents. Use only when ending current
         * event. For deactivation occuring due to physical conditions of 
         * the system use deactivate() method.*/
        virtual void reset() {
            status_ = false;
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

        /** Easy access to stored data in fifo, notice at() function used
         * (raises exception).*/
        virtual CorrEventData& operator [] (unsigned index) {
            return info_.at(index);
        }

        virtual CorrEventData operator [] (unsigned index) const {
            return info_.at(index);
        }

        /** Easy access to last (current) element of fifo. If fifo
         * is empty time=-1 event is returned*/
        virtual CorrEventData last() {
            if (info_.size() > 0)
                return info_.back();
            else {
                CorrEventData empty(-1);
                return empty;
            }
        }

        /** Easy access to second to last element of fifo. If fifo
         * has only one event, time=-1 event is returned. */
        virtual CorrEventData secondlast() {
            if (info_.size() > 1) {
                unsigned sz = info_.size();
                return info_.at(sz - 2);
            } else {
                CorrEventData empty(-1);
                return empty;
            }
        }

        /** Returns true if place should automatically deactivate
         * after the end of event and false if place should stay 
         * in current state as long as deactivating
         * signal is explicitly send. */
        virtual bool resetable() const {
            return resetable_;
        }

        /** Pythonic style private field. Use it if you must,
         * but perhaps you should not. Stores information on past 
         * events in a given Place.*/
        deque<CorrEventData> info_;

    protected:
        /** Pure virutal function. The check function should decide how
         * to change status depending on the status of children. Should be 
         * implemented in a derived class.
         */
        virtual void check_(CorrEventData& info) = 0;

        /** Fifo (info_) depth */
        unsigned max_size_;

        /** Adds parent to the list. User should be able to call addChild 
         * function only. The addParent function should be called by addChild.
         */
        virtual void addParent_ (Place* parent) {
            parents_.push_back(parent);
        }

        /** Reports change of status to parents. 
         * Calls check() function for all the parents.
         */
        virtual void report_(CorrEventData& info) {
            vector<Place*>::iterator it;
            for (it = parents_.begin(); it != parents_.end(); ++it)
                (*it)->check_(info);
        }

        virtual void add_info_(const CorrEventData& info) {
            info_.push_back(info);
            while (info_.size() > max_size_)
                info_.pop_front();
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

};

/** T stands for Tree not root style useless decorator for classes. Singleton.*/
class TreeCorrelator {
    public:
        /** Returns only instance (reference) of TCorrelator class.*/
        static TreeCorrelator& get() {
            // Safe for destruction, placed in static memory
            static TreeCorrelator instance;
            return instance;
        }

        /** This map holds all Places. */
        map<string, Place*> places;
    private:
        /** Make constructor, copy-constructor and operator =
         * private to complete singleton implementation.*/
        TreeCorrelator() {}
        /* Do not implement*/
        TreeCorrelator(TreeCorrelator const&);
        void operator=(TreeCorrelator const&);
};

/** The basic detector which does not depend on children status. Will use
 *  info_ map to store additional information.*/
class PlaceDetector : public Place {
    public:
        PlaceDetector(bool resetable = true) : Place(resetable) {} 
    protected:
        virtual void check_(CorrEventData& info);
};

/** This Place is an abstract place, which status depends on
 * children in OR manner i.e if any of children is activated, the place 
 * is activated too.
 * Example: Beta place - if any of physical beta detectors was
 * activated, the place Beta is also active indicating that there was beta
 * registered in the system).*/
class PlaceOR : public Place {
    public:
        PlaceOR() : Place() {}
    protected:
        virtual void check_(CorrEventData& info);
};

/** Similar to PlaceOR but uses AND relation. That makes it a suitable choice
 * for coincidences and anti-coincidences (use relation parameter when adding
 * a child).
 * Example : GammaBeta place storing beta-gamma coincidence,
 * active when both Beta and Gamma children are active.
 * */
class PlaceAND : public Place {
    public:
        PlaceAND() : Place() {}
    protected:
        virtual void check_(CorrEventData& info);
};

#endif
