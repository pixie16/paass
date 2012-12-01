#ifndef PLACES_H
#define PLACES_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <deque>
#include <utility>
#include <stdexcept>
#include "Globals.hpp"

using namespace std;

/** Simple structure holding basic parameters needed for correlation
 * of events in the same place. */
class CorrEventData {
    public:
        /** Time is always needed, by default status is true,  
         * Energy is 0 (i.e. N/A) and event type is an empty string.*/
        CorrEventData(double t, bool s = true, double E = 0, string type = "") {
            time = t;
            status = s;
            energy = E;
        }

        /** Time and energy type of constructor */
        CorrEventData(double t, double E, bool s = true, string type = "") {
            time = t;
            energy = E;
            status = s;
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
            if (verbose::CORRELATOR_INIT)
                cout << "~Place" << endl;
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

        /** Activates Place and reports to the parent if place was not active,
         * always saves event data to the fifo.*/
        virtual void activate(CorrEventData& info) {
            if (!status_) {
                status_ = true;
                add_info_(info);
                report_(info);
            } else {
                add_info_(info);
            }
        }

        /** Simplified activation for counter-like detectors, without
         * need of creating CorrEventData object outside.*/
        virtual void activate(double time) {
            CorrEventData info(time);
            activate(info);
        }

        /** Changes status to false. Only time and status change is
         * recorded in fifo. */
        virtual void deactivate(double time) {
            if (status_) {
                status_ = false;
                CorrEventData info(time, status_);
                add_info_(info);
                report_(info);
            } else {
                CorrEventData info(time, status_);
                add_info_(info);
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

        /** Easy access to the second to last element of fifo. If fifo
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

        /** Vector keeping a list of children on which status of the
         * Place depends.
         * Place* is a pointer to the downstream place, bool describes relation
         * (true for coincidence-like, false for anti-coincidence).
         */
        vector< pair<Place*, bool> > children_;

        /** Vector keeping a list of parents to whom the change of status
         * should be reported.
         */
        vector<Place*> parents_;


};

/** "Lazy" Place does not store multiple activation or deactivation events.
 * Abstract class.*/
class PlaceLazy : public Place {
    public:
        PlaceLazy(bool resetable = true, unsigned max_size = 2) :
            Place(resetable, max_size) {}

        /** Activates Place and saves event data to deque only if place
         * was not active before.*/
        virtual void activate(CorrEventData& info) {
            if (!status_) {
                status_ = true;
                add_info_(info);
                report_(info);
            }
        }

        /** Changes status to false, time and status change is
         * recorded in fifo only if Place was active before. */
        virtual void deactivate(double time) {
            if (status_) {
                status_ = false;
                CorrEventData info(time, status_);
                add_info_(info);
                report_(info);
            }
        }
};

/** The basic detector which does not depend on children status. Will use
 *  info_ map to store additional information.*/
class PlaceDetector : public Place {
    public:
        PlaceDetector(bool resetable = true, unsigned max_size = 2) :
            Place(resetable, max_size) {} 
    protected:
        virtual void check_(CorrEventData& info);
};

/** Each activation must be within the set thresholds. */
class PlaceThreshold : public Place {
    public:
        /** Low and high threshold limit set in constructor. If both set to 0, no threshold is applied. */
        PlaceThreshold (double low_limit, double high_limit, bool resetable = true, unsigned max_size = 2)
            : Place(resetable, max_size) { 
                low_limit_ = low_limit;
                high_limit_ = high_limit;
        } 

        /** Activate place only if energy is within set limits or both limits are 0 (no threshold). */
        void activate(CorrEventData& info) {
            if (low_limit_ == 0 && high_limit_ == 0) 
                    Place::activate(info);
            else if (info.energy > low_limit_ && info.energy < high_limit_) 
                    Place::activate(info);
        }

        double getLowLimit() {
            return low_limit_;
        }

        double getHighLimit() {
            return high_limit_;
        }

    protected:
        virtual void check_(CorrEventData& info);
        /** Threshold low and high limits.*/
        double low_limit_;
        double high_limit_;
};

class PlaceThresholdOR : public PlaceThreshold {
    public:
        PlaceThresholdOR (double low_limit, double high_limit, bool resetable = true, unsigned max_size = 2)
            : PlaceThreshold(low_limit, high_limit, resetable, max_size) { 
        } 

    protected:
        virtual void check_(CorrEventData& info);
};

/** Counts number of activations coming from directly or from children.*/
class PlaceCounter : public Place {
    public:

        /** Low and high threshold limit. If both set to 0, no threshold is applied. */
        PlaceCounter(bool resetable = true, unsigned max_size = 2)
            : Place(resetable, max_size) { 
                counter_ = 0;
        } 

        void activate(CorrEventData& info) {
            ++counter_;
            Place::activate(info);
        }

        void deactivate(double time) {
            --counter_;
            Place::deactivate(time);
        }

        void reset() {
            Place::reset();
            counter_ = 0;
        }

        virtual int getCounter() const {
            return counter_;
        }

    protected:
        int counter_;
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
        PlaceOR(bool resetable = true, unsigned max_size = 2) :
            Place(resetable, max_size) {} 
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
        PlaceAND(bool resetable = true, unsigned max_size = 2) :
            Place(resetable, max_size) {} 
    protected:
        virtual void check_(CorrEventData& info);
};

#endif
